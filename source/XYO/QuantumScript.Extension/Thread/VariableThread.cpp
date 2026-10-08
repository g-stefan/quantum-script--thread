//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <XYO/QuantumScript.Extension/Thread/VariableThread.hpp>
#include <XYO/QuantumScript.Extension/Thread/Context.hpp>

namespace XYO::QuantumScript::Extension::Thread {

	XYO_DYNAMIC_TYPE_IMPLEMENT(VariableThread, "{EC4DDB48-CB72-4668-9F29-ACED0BD31287}");
	const char *VariableThread::strTypeThread = "Thread.Thread";

	VariableThread::VariableThread() {
		XYO_DYNAMIC_TYPE_PUSH(VariableThread);
		requestToTerminateThread.set(false);
		returnValue = nullptr;
		executive = nullptr;
		sourceCode = "";
		bytecode.length = 0;
		functionHint = ParserFunctionHint::All;
		startedOk = false;
		threadEnded = true;
		symbolList = nullptr;
		functionSymbolList = nullptr;
		returnedValue = Variable::newVariable();
	};

	String VariableThread::getVariableType() {
		return strTypeThread;
	};

	Variable *VariableThread::newVariable() {
		return (Variable *)TMemory<VariableThread>::newMemory();
	};

	Variable *VariableThread::instancePrototype() {
		// no prototype in a thread that did not require the extension
		ThreadContext *threadContext = Extension::Thread::getContext();
		if (!threadContext->prototypeThread) {
			return nullptr;
		};
		return threadContext->prototypeThread->prototype;
	};

	void VariableThread::activeDestructor() {
		// nobody can read the result anymore, ask the thread to end before waiting for it
		requestToTerminateThread.set(true);
		join();
		requestToTerminateThread.set(false);
		returnValue = nullptr;
		executive = nullptr;
		releaseStartData_();
		functionHint = ParserFunctionHint::All;
		startedOk = false;
		threadEnded = true;
		returnedValue = Variable::newVariable();
	};

	Variable *VariableThread::clone(SymbolList &inSymbolList) {
		VariableThread *out = (VariableThread *)newVariable();
		return out;
	};

	bool VariableThread::toBoolean() {
		return true;
	};

	String VariableThread::toString() {
		return strTypeThread;
	};

	// ---

	// The thread function in the executive of the thread
	static bool getThreadFunction(VariableThread *thread, TPointer<Variable> &threadFunction) {
		if (thread->bytecode.length == 0) {
			if (ExecutiveX::executeString((char *)thread->sourceCode.value())) {
				threadFunction = ExecutiveX::returnValue();
				if (!threadFunction) {
					return false;
				};
				return true;
			};
			return false;
		};

		Executive &executive = ExecutiveX::getExecutive();
		CodeUnit unit;
		CodeUnitMap units;
		String error;
		if (!Bytecode::load(thread->bytecode.buffer, thread->bytecode.length, unit, error, &units)) {
			return false;
		};
		CodeUnitMap::Node *scan;
		for (scan = units.begin(); scan; scan = scan->successor()) {
			executive.setIncludeUnit(scan->key, scan->value.value());
		};
		threadFunction = executive.newFunctionFromUnit(unit, thread->functionHint);
		return true;
	};

	// Setup of the thread executive, the caller waits in threadStarted,
	// returns false on error (also on exception) and the caller is released by threadProcedure_
	static bool threadSetup_(VariableThread *thread, TPointer<Variable> &threadFunction, TPointer<Variable> &functionThis, TPointer<Variable> &functionArguments) {
		try {
			if (!ExecutiveX::initExecutive(thread->executive->mainCmdN, thread->executive->mainCmdS, thread->executive->applicationInitExecutive)) {
				return false;
			};
			if (!getThreadFunction(thread, threadFunction)) {
				return false;
			};

			(Extension::Thread::getContext())->currentThread = thread;

			// CurrentThread.this_=thread;
			Symbol currentThreadSymbol = Context::getSymbol("CurrentThread");
			if (TIsTypeExact<VariableUndefined>((Context::getGlobalObject())->getPropertyBySymbol(currentThreadSymbol))) {
				(Context::getGlobalObject())->setPropertyBySymbol(currentThreadSymbol, VariableObject::newVariable());
			};

			TPointer<Variable> currenThread = (Context::getGlobalObject())->getPropertyBySymbol(currentThreadSymbol);
			currenThread->setPropertyBySymbol(Context::getSymbol("this_"), VariableResource::newVariable(thread, nullptr));

			try {
				functionThis = thread->functionThis->clone(*(thread->symbolList));
			} catch (...) {
				functionThis = Context::getValueUndefined();
			};

			try {
				if (TIsType<VariableArray>(thread->functionArguments)) {
					functionArguments = thread->functionArguments->clone(*(thread->symbolList));
					if (!TIsType<VariableArray>(functionArguments)) {
						functionArguments = VariableArray::newVariable();
					};
				} else {
					functionArguments = VariableArray::newVariable();
				};
			} catch (...) {
				functionArguments = VariableArray::newVariable();
			};

			return true;
		} catch (...) {
		};
		return false;
	};

	static void threadProcedure_(VariableThread *thread) {
		TPointer<Variable> threadFunction;
		TPointer<Variable> functionThis;
		TPointer<Variable> functionArguments;

		if (!threadSetup_(thread, threadFunction, functionThis, functionArguments)) {
			threadFunction.deleteMemory();
			functionThis.deleteMemory();
			functionArguments.deleteMemory();
			(Extension::Thread::getContext())->currentThread = nullptr;
			thread->threadStarted.notify();
			ExecutiveX::endProcessing();
			return;
		};

		thread->startedOk = true;
		thread->threadStarted.notify();

		try {

			thread->returnValue = threadFunction->functionApply(functionThis, (VariableArray *)functionArguments.value());
			if (!thread->returnValue) {
				thread->returnValue = Context::getValueUndefined();
			};

		} catch (...) {
			thread->returnValue = Context::getValueUndefined();
		};

		thread->functionSymbolList = &Context::getSymbolList();
		threadFunction.deleteMemory();
		functionThis.deleteMemory();
		functionArguments.deleteMemory();
		thread->threadTerminated.notify();
		thread->threadWaitCopyReturnValue.wait();
		thread->returnValue.deleteMemory();

		(Extension::Thread::getContext())->currentThread = nullptr;
		ExecutiveX::endProcessing();
	};

	bool VariableThread::start(Executive *executive_, String sourceCode_, Variable *functionThis_, Variable *functionArguments_) {
		join();
		sourceCode = sourceCode_;
		bytecode.length = 0;
		functionHint = ParserFunctionHint::All;
		return start_(executive_, functionThis_, functionArguments_);
	};

	bool VariableThread::startBytecode(Executive *executive_, const Buffer &bytecode_, int functionHint_, Variable *functionThis_, Variable *functionArguments_) {
		join();
		sourceCode = "";
		bytecode.copy(bytecode_);
		functionHint = functionHint_;
		return start_(executive_, functionThis_, functionArguments_);
	};

	bool VariableThread::start_(Executive *executive_, Variable *functionThis_, Variable *functionArguments_) {
		executive = executive_;
		functionThis = functionThis_;
		functionArguments = functionArguments_;
		startedOk = false;
		threadEnded = false;
		requestToTerminateThread.set(false);
		symbolList = &Context::getSymbolList();
		if (thread.start((ThreadProcedure)threadProcedure_, this)) {
			threadStarted.wait();
			// the thread has its own copy of the start data
			releaseStartData_();
			if (startedOk) {
				return true;
			};
			thread.join();
		};
		threadEnded = true;
		releaseStartData_();
		return false;
	};

	void VariableThread::releaseStartData_() {
		sourceCode = "";
		bytecode.activeDestructor();
		functionThis = nullptr;
		functionArguments = nullptr;
	};

	// The thread function ended, copy the returned value while the thread
	// waits in threadWaitCopyReturnValue, the thread is released also if the copy fails
	void VariableThread::collectReturnValue_() {
		threadEnded = true;
		threadTerminated.wait();
		try {
			returnedValue = returnValue->clone(*functionSymbolList);
		} catch (...) {
			returnedValue = Context::getValueUndefined();
		};
		threadWaitCopyReturnValue.notify();
	};

	void VariableThread::join() {
		if (!threadEnded) {
			if (thread.isRunning()) {
				collectReturnValue_();
			} else {
				threadEnded = true;
			};
		};
		thread.join();
	};

	bool VariableThread::isRunning() {
		if (threadEnded) {
			return false;
		};
		if (!thread.isRunning()) {
			threadEnded = true;
			return false;
		};
		if (threadTerminated.peek()) {
			collectReturnValue_();
			return false;
		};
		return true;
	};

};
