// Quantum Script Extension Thread
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_VERSION_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_THREAD_VERSION_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/Thread/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::Thread::Version {

	XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT const char *version();
	XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT const char *build();
	XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT const char *versionWithBuild();
	XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT const char *datetime();

};

#endif
