//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_VARIABLEATOMIC_HPP
#define QUANTUM_SCRIPT_VARIABLEATOMIC_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_ATOMIC_HPP
#	include <XYO/QuantumScript.Extension/Thread/Atomic.hpp>
#endif

namespace XYO::QuantumScript::Extension::Thread {

	class VariableAtomic;

};

namespace XYO::ManagedMemory {
	template <>
	class TMemory<XYO::QuantumScript::Extension::Thread::VariableAtomic> : public TMemoryPoolActive<XYO::QuantumScript::Extension::Thread::VariableAtomic> {};

};

namespace XYO::QuantumScript::Extension::Thread {

	class VariableAtomic : public Variable {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(VariableAtomic);
			XYO_DYNAMIC_TYPE_DEFINE(XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT, VariableAtomic);

		protected:
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT static const char *strTypeAtomic;

		public:
			Atomic *value;

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT VariableAtomic();
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT ~VariableAtomic();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT void activeDestructor();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT static Variable *newVariable();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT String getVariableType();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT Variable *instancePrototype();

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT Variable *clone(SymbolList &inSymbolList);

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT bool toBoolean();
			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT String toString();
	};

};

#endif
