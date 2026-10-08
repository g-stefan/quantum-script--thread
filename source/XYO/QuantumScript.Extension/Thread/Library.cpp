// Quantum Script Extension Thread
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/QuantumScript.Extension/Thread/Library.hpp>
#include <XYO/QuantumScript.Extension/Thread/Copyright.hpp>
#include <XYO/QuantumScript.Extension/Thread/License.hpp>
#include <XYO/QuantumScript.Extension/Thread/Version.hpp>

#include <XYO/QuantumScript.Extension/Thread/VariableAtomic.hpp>
#include <XYO/QuantumScript.Extension/Thread/VariableThread.hpp>
#include <XYO/QuantumScript.Extension/Thread/Context.hpp>
#include <XYO/QuantumScript.Extension/Thread/Library.Source.cpp>
#include <XYO/QuantumScript/VariableVmFunction.hpp>

namespace XYO::QuantumScript::Extension::Thread {

	ThreadContext::ThreadContext() {
		currentThread = nullptr;
		symbolFunctionThread = 0;
		prototypeThread.pointerLink(this);
		symbolFunctionAtomic = 0;
		prototypeAtomic.pointerLink(this);
	};

	ThreadContext *getContext() {
		return TSingleton<ThreadContext>::getValue();
	};

	static TPointer<Variable> functionThread(VariableFunction *function, Variable *this_, VariableArray *arguments) {
		return VariableThread::newVariable();
	};

	static TPointer<Variable> functionAtomic(VariableFunction *function, Variable *this_, VariableArray *arguments) {
		return VariableAtomic::newVariable();
	};

	static void deleteContext() {
		ThreadContext *threadContext = getContext();
		threadContext->prototypeThread.deleteMemory();
		threadContext->symbolFunctionThread = 0;

		threadContext->prototypeAtomic.deleteMemory();
		threadContext->symbolFunctionAtomic = 0;
	};

	static void newContext(Executive *executive, void *extensionId) {
		VariableFunction *defaultPrototypeFunction;

		ThreadContext *threadContext = getContext();
		executive->setExtensionDeleteContext(extensionId, deleteContext);

		// Thread=function(){};
		threadContext->symbolFunctionThread = Context::getSymbol("Thread");
		threadContext->prototypeThread.newMemory();

		defaultPrototypeFunction = (VariableFunction *)VariableFunction::newVariable(nullptr, nullptr, nullptr, functionThread, nullptr, nullptr);
		(Context::getGlobalObject())->setPropertyBySymbol(threadContext->symbolFunctionThread, defaultPrototypeFunction);
		threadContext->prototypeThread = defaultPrototypeFunction->prototype;

		// Atomic=function(){};
		threadContext->symbolFunctionAtomic = Context::getSymbol("Atomic");
		threadContext->prototypeAtomic.newMemory();

		defaultPrototypeFunction = (VariableFunction *)VariableFunction::newVariable(nullptr, nullptr, nullptr, functionAtomic, nullptr, nullptr);
		(Context::getGlobalObject())->setPropertyBySymbol(threadContext->symbolFunctionAtomic, defaultPrototypeFunction);
		threadContext->prototypeAtomic = defaultPrototypeFunction->prototype;
	};

	static TPointer<Variable> isThread(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- script-is-thread\n");
#endif
		return VariableBoolean::newVariable(TIsType<VariableThread>(arguments->index(0)));
	};

	static TPointer<Variable> isAtomic(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- script-is-atomic\n");
#endif
		return VariableBoolean::newVariable(TIsType<VariableAtomic>(arguments->index(0)));
	};

	static TPointer<Variable> currentThreadSleep(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- current-thread-sleep\n");
#endif

		XYO::Multithreading::Thread::sleep((arguments->index(0))->toIndex());
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadStart(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-start\n");
#endif
		if (TIsType<VariableThread>(this_)) {

			// without source (stripped or protected), the code of the function is moved as bytecode
			TPointerX<Variable> &threadFunction = arguments->index(0);
			if (TIsType<VariableVmFunction>(threadFunction)) {
				if (((VariableVmFunction *)threadFunction.value())->fnSource == 0) {
					CodeUnit unit;
					Buffer bytecode;
					String error;
					if (!Linker::unlinkFunction(threadFunction, unit, error) || !Bytecode::save(unit, bytecode, error)) {
						String message("Thread start: ");
						message << error;
						throw Error(message);
					};
					return VariableBoolean::newVariable(((VariableThread *)this_)->startBytecode(&ExecutiveX::getExecutive(), bytecode, ((VariableVmFunction *)threadFunction.value())->functionHint, arguments->index(1), arguments->index(2)));
				};
			};

			String sourceCode = "return ";
			sourceCode += (arguments->index(0))->toString();
			sourceCode += ";";

			return VariableBoolean::newVariable(((VariableThread *)this_)->start(&ExecutiveX::getExecutive(), sourceCode, arguments->index(1), arguments->index(2)));
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadStartFromFile(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-start-from-file\n");
#endif
		if (TIsType<VariableThread>(this_)) {

			String fileName = (arguments->index(0))->toString();
			if (Bytecode::isBytecodeFile(fileName)) {
				Buffer bytecode;
				if (Shell::fileGetContents(fileName, bytecode)) {
					return VariableBoolean::newVariable(((VariableThread *)this_)->startBytecode(&ExecutiveX::getExecutive(), bytecode, ParserFunctionHint::All, arguments->index(1), arguments->index(2)));
				};
				return VariableBoolean::newVariable(false);
			};

			String sourceCode = "return function(){";
			String fileCode;
			if (Shell::fileGetContents(fileName, fileCode)) {
				sourceCode += fileCode;
				// new line, the code can end with a // comment
				sourceCode += "\n};";

				return VariableBoolean::newVariable(((VariableThread *)this_)->start(&ExecutiveX::getExecutive(), sourceCode, arguments->index(1), arguments->index(2)));
			};
			return VariableBoolean::newVariable(false);
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadStartFromString(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-start-from-string\n");
#endif
		if (TIsType<VariableThread>(this_)) {

			String sourceCode = "return function(){";
			sourceCode += (arguments->index(0))->toString();
			// new line, the code can end with a // comment
			sourceCode += "\n};";

			return VariableBoolean::newVariable(((VariableThread *)this_)->start(&ExecutiveX::getExecutive(), sourceCode, arguments->index(1), arguments->index(2)));
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadGetReturnedValue(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-get-returned-value\n");
#endif
		if (TIsType<VariableThread>(this_)) {
			return ((VariableThread *)this_)->returnedValue;
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadIsRunning(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-is-running\n");
#endif
		if (TIsType<VariableThread>(this_)) {
			return VariableBoolean::newVariable(((VariableThread *)this_)->isRunning());
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadIsTerminated(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-is-terminated\n");
#endif
		if (TIsType<VariableThread>(this_)) {
			return VariableBoolean::newVariable(!((VariableThread *)this_)->isRunning());
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> currentThreadIsRequestToTerminate(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- current-thread-is-request-to-terminate\n");
#endif
		VariableThread *currentThread = (getContext())->currentThread;
		if (currentThread) {
			return VariableBoolean::newVariable(currentThread->requestToTerminateThread.get());
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadJoin(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-join\n");
#endif
		if (TIsType<VariableThread>(this_)) {
			((VariableThread *)this_)->join();
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> threadRequestToTerminate(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- thread-request-to-terminate\n");
#endif
		if (TIsType<VariableThread>(this_)) {
			((VariableThread *)this_)->requestToTerminateThread.set(true);
		};
		return Context::getValueUndefined();
	};

	static TPointer<Variable> atomicGetValue(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- atomic-get\n");
#endif

		if (!TIsType<VariableAtomic>(this_)) {
			throw(Error("invalid parameter"));
		};

		// copy of the value, another thread can change it
		bool valueBoolean = false;
		Number valueNumber = 0;
		String valueString;
		switch (((VariableAtomic *)(this_))->value->get(valueBoolean, valueNumber, valueString)) {
		case Atomic::typeBoolean:
			return VariableBoolean::newVariable(valueBoolean);
		case Atomic::typeNumber:
			return VariableNumber::newVariable(valueNumber);
		case Atomic::typeString:
			return VariableString::newVariable(valueString);
		default:
			break;
		};

		return VariableNull::newVariable();
	};

	static TPointer<Variable> atomicSetValue(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- atomic-set\n");
#endif

		if (!TIsType<VariableAtomic>(this_)) {
			throw(Error("invalid parameter"));
		};

		Atomic *atomic = ((VariableAtomic *)(this_))->value;
		TPointerX<Variable> &value = arguments->index(0);

		if (TIsType<VariableBoolean>(value)) {
			atomic->setBoolean(value->toBoolean());
			return Context::getValueUndefined();
		};

		if (TIsType<VariableNumber>(value)) {
			atomic->setNumber(value->toNumber());
			return Context::getValueUndefined();
		};

		if (TIsType<VariableString>(value)) {
			atomic->setString(value->toString());
			return Context::getValueUndefined();
		};

		if (TIsType<VariableNull>(value)) {
			atomic->clear();
			return Context::getValueUndefined();
		};

		if (TIsTypeExact<VariableUndefined>(value)) {
			atomic->clear();
			return Context::getValueUndefined();
		};

		atomic->setString(value->toString());

		return Context::getValueUndefined();
	};

	static TPointer<Variable> processorGetCount(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- processor-get-count\n");
#endif
		return VariableNumber::newVariable(Processor::getCount());
	};

	void registerInternalExtension(Executive *executive) {
		executive->registerInternalExtension("Thread", initExecutive);
	};

	void initExecutive(Executive *executive, void *extensionId) {

		String info = "Thread\r\n";
		info << License::shortLicense().c_str();

		executive->setExtensionName(extensionId, "Thread");
		executive->setExtensionInfo(extensionId, info);
		executive->setExtensionVersion(extensionId, Extension::Thread::Version::versionWithBuild());
		executive->setExtensionPublic(extensionId, true);

		newContext(executive, extensionId);

		executive->compileStringX("if(Script.isNil(CurrentThread)){CurrentThread={};};");
		executive->setFunction2("CurrentThread.sleep(count)", currentThreadSleep);
		executive->setFunction2("CurrentThread.isRequestToTerminate()", currentThreadIsRequestToTerminate);
		//
		executive->setFunction2("Thread.isThread(x)", isThread);
		executive->setFunction2("Thread.prototype.start(function_,this_,arguments_)", threadStart);
		executive->setFunction2("Thread.prototype.startFromFile(file_,this_,arguments_)", threadStartFromFile);
		executive->setFunction2("Thread.prototype.startFromString(string_,this_,arguments_)", threadStartFromString);
		executive->setFunction2("Thread.prototype.join()", threadJoin);
		executive->setFunction2("Thread.prototype.getReturnedValue()", threadGetReturnedValue);
		executive->setFunction2("Thread.prototype.isRunning()", threadIsRunning);
		executive->setFunction2("Thread.prototype.isTerminated()", threadIsTerminated);
		executive->setFunction2("Thread.prototype.requestToTerminate()", threadRequestToTerminate);
		//
		executive->compileStringX("if(Script.isNil(Atomic)){Atomic={};};");
		executive->setFunction2("Atomic.isAtomic(x)", isAtomic);
		executive->setFunction2("Atomic.prototype.get()", atomicGetValue);
		executive->setFunction2("Atomic.prototype.set(value)", atomicSetValue);
		//
		executive->compileStringX("if(Script.isNil(Processor)){Processor={};};");
		executive->setFunction2("Processor.getCount()", processorGetCount);
		//

		executive->compileStringX(librarySource);
	};

};

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY
#	ifdef XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY
extern "C" XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT void quantumScriptExtension(XYO::QuantumScript::Executive *executive, void *extensionId) {
	XYO::QuantumScript::Extension::Thread::initExecutive(executive, extensionId);
};
#	endif
#endif
