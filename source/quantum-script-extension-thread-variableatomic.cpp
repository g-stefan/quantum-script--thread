//
// Quantum Script Library
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "quantum-script-context.hpp"
#include "quantum-script-variablenumber.hpp"
#include "quantum-script-variablestring.hpp"

#include "quantum-script-extension-thread.hpp"
#include "quantum-script-extension-thread-variableatomic.hpp"


namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				using namespace XYO;

				const char *VariableAtomic::typeAtomicKey = "{89225310-A868-445B-B620-171D417FC882}";
				const void *VariableAtomic::typeAtomic;
				const char *VariableAtomic::strTypeAtomic = "Thread.Atomic";

				VariableAtomic::VariableAtomic() {
					variableType = registerType(typeAtomic, typeAtomicKey);
					value = nullptr;
				};

				VariableAtomic::~VariableAtomic() {
					if(value != nullptr) {
						value->decReferenceCount();
					};
				};

				String VariableAtomic::getType() {
					return strTypeAtomic;
				};

				Variable *VariableAtomic::newVariable() {
					VariableAtomic *retV = TMemory<VariableAtomic>::newMemory();
					retV->value = new Atomic();
					retV->value->referenceCount++;
					return (Variable *) retV;
				};

				Variable &VariableAtomic::operatorReference(Symbol symbolId) {
					return operatorReferenceX(symbolId, (Extension::Thread::getContext())->prototypeAtomic->prototype);
				};

				Variable *VariableAtomic::instancePrototype() {
					return (Extension::Thread::getContext())->prototypeAtomic->prototype;
				};

				void VariableAtomic::activeDestructor() {
					if(value != nullptr) {
						value->decReferenceCount();
						value = nullptr;
					};
				};

				Variable *VariableAtomic::clone(SymbolList &inSymbolList) {
					VariableAtomic *out = TMemory<VariableAtomic>::newMemory();
					out->value = value;
					if(out->value) {
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
		};
	};
};


