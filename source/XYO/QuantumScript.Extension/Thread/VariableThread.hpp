//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_VARIABLETHREAD_HPP
#define QUANTUM_SCRIPT_VARIABLETHREAD_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/Thread/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::Thread {

	class VariableThread;

};

namespace XYO::ManagedMemory {
	template <>
	class TMemory<XYO::QuantumScript::Extension::Thread::VariableThread> : public TMemoryPoolActive<XYO::QuantumScript::Extension::Thread::VariableThread> {};
};

namespace XYO::QuantumScript::Extension::Thread {

	using namespace XYO;

	class VariableThread : public Variable {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(VariableThread);
			XYO_DYNAMIC_TYPE_DEFINE(XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT, VariableThread);

		protected:
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT static const char *strTypeThread;

		public:
			XYO::Multithreading::Thread thread;
			bool requestToTerminateThread;
			TPointer<Variable> returnValue;
			TPointer<Variable> returnedValue;
			Semaphore threadStarted;
			Semaphore threadWaitCopyReturnValue;
			Semaphore threadTerminated;
			//
			Executive *executive;
			String sourceCode;
			bool startedOk;
			bool threadEnded;
			SymbolList *symbolList;
			SymbolList *functionSymbolList;
			TPointer<Variable> functionThis;
			TPointer<Variable> functionArguments;

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT VariableThread();

			inline ~VariableThread() {
				join();
			};

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT void activeDestructor();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT static Variable *newVariable();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT String getVariableType();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT Variable *instancePrototype();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT Variable *clone(SymbolList &inSymbolList);

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT bool toBoolean();
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT String toString();

			// ---

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT bool start(Executive *executive_, String sourceCode_, Variable *functionThis_, Variable *functionArguments_);
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT void join();
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT bool isRunning();
	};

};

#endif
