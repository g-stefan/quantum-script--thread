//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_ATOMIC_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_THREAD_ATOMIC_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/Thread/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::Thread {

				class Atomic {
					public:
						CriticalSection criticalSection;

						static const int typeUnknown = 0;
						static const int typeBoolean = 1;
						static const int typeNumber = 2;
						static const int typeString = 3;

						int atomicType;
						bool valueBooloean;
						Number valueNumber;
						char *valueString;
						int referenceCount;

						inline Atomic() {
							atomicType = typeUnknown;
							valueBooloean = false;
							valueNumber = 0;
							valueString = nullptr;
							referenceCount = 0;
						};

						inline ~Atomic() {
							if (atomicType == typeString) {
								delete[] valueString;
							};
						};

						inline void decReferenceCount() {
							criticalSection.enter();
							--referenceCount;
							if (referenceCount == 0) {
								criticalSection.leave();
								delete this;
								return;
							};
							criticalSection.leave();
						};

						inline void incReferenceCount() {
							criticalSection.enter();
							++referenceCount;
							criticalSection.leave();
						};

						inline void clear() {
							criticalSection.enter();

							if (atomicType == typeString) {
								delete[] valueString;
							};
							atomicType = typeUnknown;
							criticalSection.leave();
						};

						inline void setBoolean(bool value) {
							criticalSection.enter();
							if (atomicType == typeString) {
								delete[] valueString;
							};

							atomicType = typeBoolean;
							valueBooloean = value;
							criticalSection.leave();
						};

						inline void setNumber(Number value) {
							criticalSection.enter();
							if (atomicType == typeString) {
								delete[] valueString;
							};

							atomicType = typeNumber;
							valueNumber = value;
							criticalSection.leave();
						};

						inline void setString(String value) {
							criticalSection.enter();
							if (atomicType == typeString) {
								delete[] valueString;
							};

							atomicType = typeString;
							valueString = new char[value.length() + 1];
							memcpy(valueString, value.value(), value.length() + 1);
							criticalSection.leave();
						};
				};

			};

#endif
