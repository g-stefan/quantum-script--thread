//
// Quantum Script Library
//
// Copyright (c) 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <XYO/QuantumScript.Extension/Thread/VariableAtomic.hpp>
#include <XYO/QuantumScript.Extension/Thread/Context.hpp>

namespace XYO::QuantumScript::Extension::Thread {

	XYO_DYNAMIC_TYPE_IMPLEMENT(VariableAtomic, "{89225310-A868-445B-B620-171D417FC882}");
	const char *VariableAtomic::strTypeAtomic = "Thread.Atomic";

	VariableAtomic::VariableAtomic() {
		XYO_DYNAMIC_TYPE_PUSH(VariableAtomic);
		value = nullptr;
	};

	VariableAtomic::~VariableAtomic() {
		if (value != nullptr) {
			value->decReferenceCount();
		};
	};

	String VariableAtomic::getVariableType() {
		return strTypeAtomic;
	};

	Variable *VariableAtomic::newVariable() {
		VariableAtomic *retV = TMemory<VariableAtomic>::newMemory();
		retV->value = new Atomic();
		retV->value->referenceCount++;
		return (Variable *)retV;
	};

	Variable *VariableAtomic::instancePrototype() {
		return (Extension::Thread::getContext())->prototypeAtomic->prototype;
	};

	void VariableAtomic::activeDestructor() {
		if (value != nullptr) {
			value->decReferenceCount();
			value = nullptr;
		};
	};

	Variable *VariableAtomic::clone(SymbolList &inSymbolList) {
		VariableAtomic *out = TMemory<VariableAtomic>::newMemory();
		out->value = value;
		if (out->value) {
			out->value->incReferenceCount();
		};
		return out;
	};

	bool VariableAtomic::toBoolean() {
		return true;
	};

	String VariableAtomic::toString() {
		return strTypeAtomic;
	};

};
