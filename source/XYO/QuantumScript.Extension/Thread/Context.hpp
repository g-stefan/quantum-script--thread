// Quantum Script Extension ProcessInteractive
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_CONTEXT_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_THREAD_CONTEXT_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/Thread/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::Thread {

	class ThreadContext : public Object {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(ThreadContext);

		public:
			Symbol symbolFunctionThread;
			Symbol symbolFunctionAtomic;
			TPointerX<Prototype> prototypeThread;
			TPointerX<Prototype> prototypeAtomic;

			XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT ThreadContext();
	};

	XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT ThreadContext *getContext();
};

#endif
