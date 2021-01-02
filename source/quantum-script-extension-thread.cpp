//
// Quantum Script Extension Thread
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

#include "quantum-script-extension-thread-license.hpp"
#include "quantum-script-extension-thread.hpp"
#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_NO_VERSION
#include "quantum-script-extension-thread-version.hpp"
#endif
//
#include "quantum-script-variablenull.hpp"
#include "quantum-script-variableboolean.hpp"
#include "quantum-script-variablenumber.hpp"
#include "quantum-script-variablestring.hpp"
#include "quantum-script-variableresource.hpp"
//
#include "quantum-script-extension-thread-variablethread.hpp"
#include "quantum-script-extension-thread-variableatomic.hpp"

#include "quantum-script-extension-thread.src"

//#define QUANTUM_SCRIPT_VM_DEBUG_RUNTIME

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Thread {

				using namespace XYO;
				using namespace Quantum::Script;

				ThreadContext::ThreadContext() {
					symbolFunctionThread = 0;
					prototypeThread.pointerLink(this);
					symbolFunctionAtomic = 0;
					prototypeAtomic.pointerLink(this);
				};

				ThreadContext *getContext() {
					return TSingleton<ThreadContext>::getValue();
				};

				static TPointer<Variable> functionThread(VariableFunction *function, Variable *this_, VariableArray *arguments) {
					return VariableThread::newVariable();
				};

				static TPointer<Variable> functionAtomic(VariableFunction *function, Variable *this_, VariableArray *arguments) {
					return VariableAtomic::newVariable();
				};

				static void deleteContext() {
					ThreadContext *threadContext = getContext();
					threadContext->prototypeThread.deleteMemory();
					threadContext->symbolFunctionThread = 0;

					threadContext->prototypeAtomic.deleteMemory();
					threadContext->symbolFunctionAtomic = 0;
				};

				static void newContext(Executive *executive, void *extensionId) {
					VariableFunction *defaultPrototypeFunction;

					ThreadContext *threadContext = getContext();
					executive->setExtensionDeleteContext(extensionId, deleteContext);

					// Thread=function(){};
					threadContext->symbolFunctionThread = Context::getSymbol("Thread");
					threadContext->prototypeThread.newMemory();

					defaultPrototypeFunction = (VariableFunction *)VariableFunction::newVariable(nullptr, nullptr, nullptr, functionThread, nullptr, nullptr);
					((Context::getGlobalObject())->operatorReferenceOwnProperty(threadContext->symbolFunctionThread))=defaultPrototypeFunction;
					threadContext->prototypeThread = defaultPrototypeFunction->prototype;

					// Atomic=function(){};
					threadContext->symbolFunctionAtomic = Context::getSymbol("Atomic");
					threadContext->prototypeAtomic.newMemory();

					defaultPrototypeFunction = (VariableFunction *)VariableFunction::newVariable(nullptr, nullptr, nullptr, functionAtomic, nullptr, nullptr);
					((Context::getGlobalObject())->operatorReferenceOwnProperty(threadContext->symbolFunctionAtomic))=defaultPrototypeFunction;
					threadContext->prototypeAtomic = defaultPrototypeFunction->prototype;
				};

				static TPointer<Variable> isThread(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- script-is-thread\n");
#endif
					return VariableBoolean::newVariable(VariableThread::isVariableThread(arguments->index(0)));
				};

				static TPointer<Variable> isAtomic(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- script-is-atomic\n");
#endif
					return VariableBoolean::newVariable(VariableAtomic::isVariableAtomic(arguments->index(0)));
				};


				static TPointer<Variable> currentThreadSleep(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_VM_DEBUG_RUNTIME
					printf("- current-thread-sleep\n");
#endif

					XYO::Thread::sleep((arguments->index(0))->toIndex());
					return Context::getValueUndefined();
				};


				static TPointer<Variable> threadStart(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-start\n");
#endif
					if(VariableThread::isVariableThread(this_)) {

						String sourceCode = "return ";
						sourceCode += (arguments->index(0))->toString();
						sourceCode += ";";

						return VariableBoolean::newVariable(((VariableThread *)this_)->start(&ExecutiveX::getExecutive(), sourceCode, arguments->index(1), arguments->index(2)));
					};
					return Context::getValueUndefined();
				};

				static TPointer<Variable> threadStartFromFile(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-start-from-file\n");
#endif
					if(VariableThread::isVariableThread(this_)) {

						String sourceCode = "return function(){";
						String fileCode;
						if(Shell::fileGetContents((arguments->index(0))->toString(), fileCode)) {
							sourceCode += fileCode;
							sourceCode += "};";

							return VariableBoolean::newVariable(((VariableThread *)this_)->start(&ExecutiveX::getExecutive(), sourceCode, arguments->index(1), arguments->index(2)));
						};
						return VariableBoolean::newVariable(false);
					};
					return Context::getValueUndefined();
				};


				static TPointer<Variable> threadStartFromString(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-start-from-string\n");
#endif
					if(VariableThread::isVariableThread(this_)) {

						String sourceCode = "return function(){";
						sourceCode += (arguments->index(0))->toString();
						sourceCode += "};";

						return VariableBoolean::newVariable(((VariableThread *)this_)->start(&ExecutiveX::getExecutive(), sourceCode, arguments->index(1), arguments->index(2)));
					};
					return Context::getValueUndefined();
				};


				static TPointer<Variable> threadGetReturnedValue(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-get-returned-value\n");
#endif
					if(VariableThread::isVariableThread(this_)) {
						return ((VariableThread *)this_)->returnedValue;
					};
					return Context::getValueUndefined();
				};


				static TPointer<Variable> threadIsRunning(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-is-running\n");
#endif
					if(VariableThread::isVariableThread(this_)) {
						return VariableBoolean::newVariable(((VariableThread *)this_)->isRunning());
					};
					return Context::getValueUndefined();
				};

				static TPointer<Variable> threadIsTerminated(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-is-terminated\n");
#endif
					if(VariableThread::isVariableThread(this_)) {
						return VariableBoolean::newVariable(!((VariableThread *)this_)->isRunning());
					};
					return Context::getValueUndefined();
				};


				static TPointer<Variable> currentThreadIsRequestToTerminate(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- current-thread-is-request-to-terminate\n");
#endif
					if(VariableObject::isVariableObject(this_)) {
						TPointerX<Variable> &thisThread_ = (this_->operatorReferenceOwnProperty(Context::getSymbol("this_")));
						if(VariableResource::isVariableResource(thisThread_)) {
							return VariableBoolean::newVariable(((VariableThread *)(((VariableResource *)(thisThread_.value()))->resource))->requestToTerminateThread);
						};
					};
					return Context::getValueUndefined();
				};

				static TPointer<Variable> threadJoin(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-join\n");
#endif
					if(VariableThread::isVariableThread(this_)) {
						((VariableThread *)this_)->join();
					};
					return Context::getValueUndefined();
				};

				static TPointer<Variable> threadRequestToTerminate(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- thread-request-to-terminate\n");
#endif
					if(VariableThread::isVariableThread(this_)) {
						((VariableThread *)this_)->requestToTerminateThread = true;
					};
					return Context::getValueUndefined();
				};

				static TPointer<Variable> atomicGetValue(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- atomic-get\n");
#endif

					if(!VariableAtomic::isVariableAtomic(this_)) {
						throw(Error("invalid parameter"));
					};

					if(((VariableAtomic *)( this_ ))->value->atomicType == Atomic::typeBoolean) {
						return VariableBoolean::newVariable(((VariableAtomic *)( this_ ))->value->valueBooloean);
					};

					if(((VariableAtomic *)( this_ ))->value->atomicType == Atomic::typeNumber) {
						return VariableNumber::newVariable(((VariableAtomic *)( this_ ))->value->valueNumber);
					};

					if(((VariableAtomic *)( this_ ))->value->atomicType == Atomic::typeString) {
						return VariableString::newVariable(((VariableAtomic *)( this_ ))->value->valueString);
					};

					return VariableNull::newVariable();
				};


				static TPointer<Variable> atomicSetValue(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- atomic-set\n");
#endif

					if(!VariableAtomic::isVariableAtomic(this_)) {
						throw(Error("invalid parameter"));
					};

					TPointerX<Variable> &value = arguments->index(0);

					if(VariableBoolean::isVariableBoolean(value)) {
						((VariableAtomic *)( this_ ))->value->setBoolean(value->toBoolean());
						return Context::getValueUndefined();
					};

					if(VariableNumber::isVariableNumber(value)) {
						((VariableAtomic *)( this_ ))->value->setNumber(value->toNumber());
						return Context::getValueUndefined();
					};

					if(VariableString::isVariableString(value)) {
						((VariableAtomic *)( this_ ))->value->setString(value->toString());
						return Context::getValueUndefined();
					};

					if(VariableNull::isVariableNull(value)) {
						((VariableAtomic *)( this_ ))->value->clear();
						return Context::getValueUndefined();
					};

					if(VariableUndefined::isVariableUndefined(value)) {
						((VariableAtomic *)( this_ ))->value->clear();
						return Context::getValueUndefined();
					};


					((VariableAtomic *)( this_ ))->value->setString(value->toString());

					return Context::getValueUndefined();
				};


				void registerInternalExtension(Executive *executive) {
					executive->registerInternalExtension("Thread", initExecutive);
				};

				void initExecutive(Executive *executive, void *extensionId) {

					String info = "Thread\r\n";
					info << License::shortContent();

					executive->setExtensionName(extensionId, "Thread");
					executive->setExtensionInfo(extensionId, info);
#ifndef QUANTUM_SCRIPT_EXTENSION_THREAD_NO_VERSION
					executive->setExtensionVersion(extensionId, Extension::Thread::Version::versionWithBuild());
#endif
					executive->setExtensionPublic(extensionId, true);

					newContext(executive, extensionId);

					executive->compileStringX("if(Script.isNil(CurrentThread)){CurrentThread={};};");
					executive->setFunction2("CurrentThread.sleep(count)",  currentThreadSleep);
					executive->setFunction2("CurrentThread.isRequestToTerminate()",  currentThreadIsRequestToTerminate);
					//
					executive->setFunction2("Thread.isThread(x)", isThread);
					executive->setFunction2("Thread.prototype.start(function_,this_,arguments_)",  threadStart);
					executive->setFunction2("Thread.prototype.startFromFile(file_,this_,arguments_)",  threadStartFromFile);
					executive->setFunction2("Thread.prototype.startFromString(string_,this_,arguments_)",  threadStartFromString);
					executive->setFunction2("Thread.prototype.join()",  threadJoin);
					executive->setFunction2("Thread.prototype.getReturnedValue()",  threadGetReturnedValue);
					executive->setFunction2("Thread.prototype.isRunning()",  threadIsRunning);
					executive->setFunction2("Thread.prototype.isTerminated()",  threadIsTerminated);
					executive->setFunction2("Thread.prototype.requestToTerminate()",  threadRequestToTerminate);
					//
					executive->setFunction2("Atomic.isAtomic(x)", isAtomic);
					executive->setFunction2("Atomic.prototype.get()",  atomicGetValue);
					executive->setFunction2("Atomic.prototype.set(value)",  atomicSetValue);

					executive->compileStringX(extensionThreadSource);
				};
			};
		};
	};
};

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
extern "C" QUANTUM_SCRIPT_EXTENSION_THREAD_EXPORT void quantumScriptExtension(Quantum::Script::Executive *executive, void *extensionId) {
	Quantum::Script::Extension::Thread::initExecutive(executive, extensionId);
};
#endif
