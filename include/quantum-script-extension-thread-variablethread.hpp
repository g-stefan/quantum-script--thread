//
// Quantum Script Extension Thread
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_VARIABLETHREAD_HPP
#define QUANTUM_SCRIPT_VARIABLETHREAD_HPP

#ifndef QUANTUM_SCRIPT_VARIABLE_HPP
#include "quantum-script-variable.hpp"
#endif

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				class VariableThread;

			};
		};

	};
};


namespace XYO {
	namespace ManagedMemory {
		template<>
		class TMemory<Quantum::Script::Extension::Thread::VariableThread>:
			public TMemoryPoolActive<Quantum::Script::Extension::Thread::VariableThread> {};
	};
};


namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				using namespace XYO;

				class VariableThread :
					public Variable {
						XYO_DISALLOW_COPY_ASSIGN_MOVE(VariableThread);
					protected:
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static const char *strTypeThread;
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static const char *typeThreadKey;
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static const void *typeThread;
					public:

						XYO::Thread thread;
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

						inline VariableThread() {
							variableType = registerType(typeThread, typeThreadKey);
							requestToTerminateThread = false;
							returnValue = nullptr;
							executive = nullptr;
							sourceCode = "";
							startedOk = false;
							threadEnded = true;
							returnedValue = Variable::newVariable();
						};

						inline ~VariableThread() {
							join();
						};

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT void activeDestructor();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static Variable *newVariable();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT String getType();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT Variable &operatorReference(Symbol symbolId);
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT Variable *instancePrototype();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT Variable *clone(SymbolList &inSymbolList);

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT bool toBoolean();
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT String toString();

						// ---

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT bool start(Executive *executive_, String sourceCode_, Variable *functionThis_, Variable *functionArguments_);
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT void join();
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT bool isRunning();
						//
						inline static bool isVariableThread(const Variable *value) {
							if(typeThread == nullptr) {
								typeThread = registerType(typeThread, typeThreadKey);
							};
							return (value->variableType == typeThread);
						};

				};

			};
		};
	};
};


#endif

