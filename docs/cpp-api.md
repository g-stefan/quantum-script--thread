# C++ API

This page is for hosts that embed Quantum Script and for extensions that
work with `Thread` or `Atomic` values. Read the `quantum-script`
repository's `docs/embedding.md` and `docs/writing-extensions.md` first.
Native functions, `Variable` and `TPointer` work the same way here. For
threads in C++ only, use `XYO::Multithreading` directly (`Thread`,
`Worker`, `WorkerQueue`); this extension is the script binding that gives
every thread its own Quantum Script executive.

## Headers and namespace

```cpp
#include <XYO/QuantumScript.Extension/Thread.hpp>                  // Library.hpp: register / init
#include <XYO/QuantumScript.Extension/Thread/VariableThread.hpp>   // the Thread value type
#include <XYO/QuantumScript.Extension/Thread/VariableAtomic.hpp>   // the Atomic value type (includes Atomic.hpp)
#include <XYO/QuantumScript.Extension/Thread/Context.hpp>          // ThreadContext (rarely needed)

using namespace XYO::QuantumScript;
using Extension::Thread::VariableThread;
using Extension::Thread::VariableAtomic;
```

The namespace is `XYO::QuantumScript::Extension::Thread`. The export macro
is `XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT`. It is empty when
`XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY` is defined, as in static
builds. The umbrella header includes only `Library.hpp`. Include the
`Variable*.hpp` headers yourself when you need the types.

Inside the namespace, `Thread` is the namespace itself. Write
`XYO::Multithreading::Thread` for the system thread class.

## Registering the extension

```cpp
void Extension::Thread::registerInternalExtension(Executive *executive);
void Extension::Thread::initExecutive(Executive *executive, void *extensionId);
```

- `registerInternalExtension` registers `"Thread"` as an internal
  extension. Call it from the host's init callback (see
  [Getting started](getting-started.md#4-register-it-in-a-c-host)). That
  callback runs again in every thread the extension starts, so register
  there every internal extension the threads may need.
- `initExecutive` is the extension's init function. The engine runs it when
  a script first requires `Thread` in a thread. It does the following:
  1. Sets the extension name, info (license text) and version, and marks the
     extension public.
  2. Creates the `Thread` and `Atomic` global functions and their
     prototypes (`ThreadContext`), and registers `deleteContext` to release
     them when the executive ends.
  3. Creates the `CurrentThread`, `Atomic` and `Processor` objects if they
     do not exist, and registers the native functions with `setFunction2`.
  4. Compiles `Library.js` (`librarySource`), which defines
     `Thread.newThread`, `Thread.newThreadFromFile` and
     `Thread.newThreadFromString`.

  Do not call it directly.
- The DLL build also exports
  `extern "C" void quantumScriptExtension(Executive *, void *)`, which
  forwards to `initExecutive`. This is the entry point
  `Script.requireExtension` looks up in `quantum-script--thread.dll`.

## VariableThread

```cpp
class VariableThread : public Variable {
	public:
		XYO::Multithreading::Thread thread;     // the system thread
		TAtomic<bool> requestToTerminateThread; // the stop request flag
		TPointer<Variable> returnedValue;       // copy of the last collected result (owner side)
		// ... start data, semaphores, internal state ...

		static Variable *newVariable();         // new Thread object, nothing started

		bool start(Executive *executive_, String sourceCode_, Variable *functionThis_, Variable *functionArguments_);
		bool startBytecode(Executive *executive_, const Buffer &bytecode_, int functionHint_, Variable *functionThis_, Variable *functionArguments_);
		void join();
		bool isRunning();
};
```

- `getVariableType()` / `toString()` return `"Thread.Thread"`; `toBoolean()`
  is `true`. Test with `TIsType<VariableThread>(value)`.
- `start(executive, sourceCode, this_, arguments_)`: `sourceCode` is
  compiled in the new thread with `ExecutiveX::executeString`. It must
  **return** the function to call, for example
  `"return function(a){ return a + 1; };"` (the script `start` builds
  `"return " + fn.toString() + ";"`). `executive` is the caller's executive
  (`&ExecutiveX::getExecutive()`); the thread reuses its command line and
  `applicationInitExecutive`. `this_` is cloned into the thread;
  `arguments_` is cloned if it is a `VariableArray`, otherwise the function
  gets no arguments. Blocks until the thread is set up. Returns `false` on
  a setup or compile error. Waits for a previous run first.
- `startBytecode(executive, bytecode, functionHint, this_, arguments_)`:
  the same with a saved code unit (`Bytecode::save`) instead of source. Its
  included units are registered with `setIncludeUnit` in the thread, and
  the function is built with `newFunctionFromUnit(unit, functionHint)`
  (`ParserFunctionHint::All` for a whole file).
- `join()` / `isRunning()`: as in the script API. Both collect the result
  into `returnedValue` when they see the end of the function.
- `requestToTerminateThread.set(true)` is what `requestToTerminate()` does.
- `activeDestructor` (the object is released) sets the stop flag, then
  calls `join()`. The C++ destructor also joins.

## Atomic and VariableAtomic

`Atomic` (in `Atomic.hpp`) is the shared cell. It is a plain C++ object,
not managed memory, reference counted with a `TAtomic<int>`. Every access
to its value takes its `CriticalSection`.

```cpp
class Atomic {
	public:
		static const int typeUnknown = 0;  // empty, script get() returns null
		static const int typeBoolean = 1;
		static const int typeNumber = 2;
		static const int typeString = 3;

		void incReferenceCount();
		void decReferenceCount();          // deletes the cell at zero

		int get(bool &valueBoolean_, Number &valueNumber_, String &valueString_); // fills the one matching the type, returns the type
		void setBoolean(bool value);
		void setNumber(Number value);
		void setString(const String &value);
		void clear();
};

class VariableAtomic : public Variable {
	public:
		Atomic *value;                     // one reference held by this handle
		static Variable *newVariable();    // new handle to a NEW empty cell
};
```

- `VariableAtomic::clone` returns a new handle to the **same** cell
  (reference count + 1). That is how an `Atomic` crosses threads.
- `activeDestructor` and the destructor release the handle's reference.
- `getVariableType()` / `toString()` return `"Thread.Atomic"`.

Reading and writing an `Atomic` passed to a native function:

```cpp
static TPointer<Variable> myFunction(VariableFunction *function, Variable *this_, VariableArray *arguments) {
	if (!TIsType<VariableAtomic>(arguments->index(0))) {
		throw(Error("invalid parameter"));
	};
	Extension::Thread::Atomic *atomic = ((VariableAtomic *)(arguments->index(0)).value())->value;

	bool valueBoolean = false;
	Number valueNumber = 0;
	String valueString;
	if (atomic->get(valueBoolean, valueNumber, valueString) == Extension::Thread::Atomic::typeNumber) {
		atomic->setNumber(valueNumber + 1);   // not atomic with the get: one writer only
	};
	return Context::getValueUndefined();
};
```

To share one cell between C++ code and scripts, keep your own reference
with `incReferenceCount()` and release it with `decReferenceCount()`.

## ThreadContext

```cpp
class ThreadContext : public Object {
	public:
		Symbol symbolFunctionThread, symbolFunctionAtomic;
		TPointerX<Prototype> prototypeThread, prototypeAtomic;
		VariableThread *currentThread;     // the Thread running this system thread, nullptr in the main thread
};
ThreadContext *Extension::Thread::getContext();   // TSingleton<ThreadContext>: one per system thread
```

The prototypes exist only in threads that required the extension. In other
threads `instancePrototype()` returns `nullptr`, so a `Thread` / `Atomic`
value has no methods there (calling one throws, without a crash).

## Static builds

`quantum-script--thread.static` defines
`XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY`: the export macro is empty
and there is no `quantumScriptExtension` entry point. Link it with
`quantum-script.static` and register the extension with
`registerInternalExtension`. The amalgamated source
`Thread.Amalgam.cpp` compiles the whole extension in one translation unit.
