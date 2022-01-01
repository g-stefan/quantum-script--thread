//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_HPP
#define QUANTUM_SCRIPT_EXTENSION_THREAD_HPP

#ifndef QUANTUM_SCRIPT_HPP
#include "quantum-script.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD__EXPORT_HPP
#include "quantum-script-extension-thread--export.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_COPYRIGHT_HPP
#include "quantum-script-extension-thread-copyright.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_LICENSE_HPP
#include "quantum-script-extension-thread-license.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_VERSION_HPP
#include "quantum-script-extension-thread-version.hpp"
#endif

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				using namespace Quantum::Script;

				class ThreadContext:
					public Object {
						XYO_DISALLOW_COPY_ASSIGN_MOVE(ThreadContext);
					public:

						Symbol symbolFunctionThread;
						Symbol symbolFunctionAtomic;
						TPointerX<Prototype> prototypeThread;
						TPointerX<Prototype> prototypeAtomic;

						QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT ThreadContext();
				};

				QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT ThreadContext *getContext();

				QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT void initExecutive(Executive *executive, void *extensionId);
				QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT void registerInternalExtension(Executive *executive);

			};
		};
	};
};

#endif

