//
// Quantum Script Extension Thread
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_VARIABLEATOMIC_HPP
#define QUANTUM_SCRIPT_VARIABLEATOMIC_HPP

#ifndef QUANTUM_SCRIPT_VARIABLE_HPP
#include "quantum-script-variable.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_ATOMIC_HPP
#include "quantum-script-extension-thread-atomic.hpp"
#endif

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				class VariableAtomic;

			};
		};
	};
};


namespace XYO {
	namespace ManagedMemory {
		template<>
		class TMemory<Quantum::Script::Extension::Thread::VariableAtomic>:
			public TMemoryPoolActive<Quantum::Script::Extension::Thread::VariableAtomic> {};
	};
};


namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				using namespace XYO;

				class VariableAtomic :
					public Variable {
						XYO_DISALLOW_COPY_ASSIGN_MOVE(VariableAtomic);
					protected:
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static const char *strTypeAtomic;
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static const char *typeAtomicKey;
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static const void *typeAtomic;
					public:

						Atomic *value;

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT VariableAtomic();
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT ~VariableAtomic();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT void activeDestructor();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT static Variable *newVariable();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT String getType();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT Variable &operatorReference(Symbol symbolId);
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT Variable *instancePrototype();

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT Variable *clone(SymbolList &inSymbolList);

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT bool toBoolean();
						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT String toString();

						//
						inline static bool isVariableAtomic(const Variable *value) {
							if(typeAtomic == nullptr) {
								typeAtomic = registerType(typeAtomic, typeAtomicKey);
							};
							return (value->variableType == typeAtomic);
						};

				};


			};
		};
	};
};


#endif

