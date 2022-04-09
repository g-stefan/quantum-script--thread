//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD__EXPORT_HPP
#define QUANTUM_SCRIPT_EXTENSION_THREAD__EXPORT_HPP

#ifndef XYO__EXPORT_HPP
#	include "xyo--export.hpp"
#endif

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
#	ifdef QUANTUM_SCRIPT_EXTENSION_THREAD_INTERNAL
#		define QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT XYO_LIBRARY_EXPORT
#	else
#		define QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT XYO_LIBRARY_IMPORT
#	endif
#else
#	define QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT
#endif

#endif
