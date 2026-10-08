//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_ATOMIC_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_THREAD_ATOMIC_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_THREAD_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/Thread/Dependency.hpp>
#endif

#ifndef XYO_MULTITHREADING_CRITICALSECTIONLOCK_HPP
#	include <XYO/Multithreading/CriticalSectionLock.hpp>
#endif

namespace XYO::QuantumScript::Extension::Thread {

	// Value shared between threads, all access to the value is done with
	// criticalSection entered. valueString is nullptr if the type is not string.
	// Memory is allocated and freed outside of criticalSection.
	class Atomic {
		public:
			CriticalSection criticalSection;

			static const int typeUnknown = 0;
			static const int typeBoolean = 1;
			static const int typeNumber = 2;
			static const int typeString = 3;

			int atomicType;
			bool valueBoolean;
			Number valueNumber;
			char *valueString;
			TAtomic<int> referenceCount;

			inline Atomic() {
				atomicType = typeUnknown;
				valueBoolean = false;
				valueNumber = 0;
				valueString = nullptr;
				referenceCount.set(0);
			};

			inline ~Atomic() {
				delete[] valueString;
			};

			inline void decReferenceCount() {
				if (referenceCount.fetchSub(1) == 1) {
					delete this;
				};
			};

			inline void incReferenceCount() {
				referenceCount.fetchAdd(1);
			};

			// Copy of the value, returns the type
			inline int get(bool &valueBoolean_, Number &valueNumber_, String &valueString_) {
				CriticalSectionLock lock(criticalSection);
				switch (atomicType) {
				case typeBoolean:
					valueBoolean_ = valueBoolean;
					break;
				case typeNumber:
					valueNumber_ = valueNumber;
					break;
				case typeString:
					valueString_ = valueString;
					break;
				default:
					break;
				};
				return atomicType;
			};

			inline void clear() {
				char *oldString;
				{
					CriticalSectionLock lock(criticalSection);
					oldString = valueString;
					valueString = nullptr;
					atomicType = typeUnknown;
				};
				delete[] oldString;
			};

			inline void setBoolean(bool value) {
				char *oldString;
				{
					CriticalSectionLock lock(criticalSection);
					oldString = valueString;
					valueString = nullptr;
					atomicType = typeBoolean;
					valueBoolean = value;
				};
				delete[] oldString;
			};

			inline void setNumber(Number value) {
				char *oldString;
				{
					CriticalSectionLock lock(criticalSection);
					oldString = valueString;
					valueString = nullptr;
					atomicType = typeNumber;
					valueNumber = value;
				};
				delete[] oldString;
			};

			inline void setString(const String &value) {
				size_t length = value.length();
				char *newString = new char[length + 1];
				memcpy(newString, value.value(), length + 1);
				char *oldString;
				{
					CriticalSectionLock lock(criticalSection);
					oldString = valueString;
					valueString = newString;
					atomicType = typeString;
				};
				delete[] oldString;
			};
	};

};

#endif
