// Quantum Script Extension Thread
// Copyright (c) 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/QuantumScript.Extension/Thread/Copyright.hpp>
#include <XYO/QuantumScript.Extension/Thread/Copyright.rh>

namespace XYO::QuantumScript::Extension::Thread::Copyright {

	static const char *copyright_ = XYO_QUANTUMSCRIPT_EXTENSION_THREAD_COPYRIGHT;
	static const char *publisher_ = XYO_QUANTUMSCRIPT_EXTENSION_THREAD_PUBLISHER;
	static const char *company_ = XYO_QUANTUMSCRIPT_EXTENSION_THREAD_COMPANY;
	static const char *contact_ = XYO_QUANTUMSCRIPT_EXTENSION_THREAD_CONTACT;

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

};
