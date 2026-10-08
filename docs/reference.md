# API reference

## Script

These symbols are available after `Script.requireExtension("Thread")`, in
the thread that called it.

### Thread

| Symbol | Returns | Notes |
|--------|---------|-------|
| `Thread()` / `new Thread()` | Thread | nothing started; arguments ignored; `typeof` is `"Thread.Thread"` |
| `Thread.isThread(x)` | Boolean | type test |
| `Thread.newThread(fn, this_, arguments_)` | Thread / `null` | `new Thread()` + `start` |
| `Thread.newThreadFromFile(file, this_, arguments_)` | Thread / `null` | `new Thread()` + `startFromFile` |
| `Thread.newThreadFromString(code, this_, arguments_)` | Thread / `null` | `new Thread()` + `startFromString` |

| Method | Returns | Behavior |
|--------|---------|----------|
| `t.start(fn, this_, arguments_)` | Boolean | waits for a previous run; moves `fn` as source (or bytecode if stripped); copies `this_` / `arguments_` (Array); waits for the setup; `false` on a compile / setup error; throws `Thread start: ...` if a stripped function cannot be moved |
| `t.startFromFile(file, this_, arguments_)` | Boolean | the file is the function body; `.qsbc` detected by header; path relative to the working directory; `false` if unreadable / not compilable |
| `t.startFromString(code, this_, arguments_)` | Boolean | `code` is the function body; `false` on a compile error |
| `t.join()` | `undefined` | waits for the end and collects the result; safe to repeat |
| `t.getReturnedValue()` | any | copy of the last collected result; `undefined` before, on exception, or if not copyable; functions inside become `undefined` |
| `t.isRunning()` | Boolean | no wait; collects the result when it sees the end; `false` before `start` |
| `t.isTerminated()` | Boolean | `!t.isRunning()`; `true` before `start` |
| `t.requestToTerminate()` | `undefined` | sets the stop flag; cooperative; cleared by the next `start` |

### CurrentThread

| Symbol | Returns | Notes |
|--------|---------|-------|
| `CurrentThread.sleep(milliSeconds)` | `undefined` | suspends the calling thread; any thread |
| `CurrentThread.isRequestToTerminate()` | Boolean / `undefined` | stop flag of the running `Thread`; `undefined` in the main thread |

### Atomic

| Symbol | Returns | Notes |
|--------|---------|-------|
| `Atomic()` / `new Atomic()` | Atomic | new empty shared cell; `typeof` is `"Thread.Atomic"` |
| `Atomic.isAtomic(x)` | Boolean | type test |
| `a.get()` | Boolean / Number / String / `null` | copy read under the lock; `null` when empty; throws `invalid parameter` if `this` is not an Atomic |
| `a.set(value)` | `undefined` | Boolean / Number / String kept; `null` / `undefined` empty it; others stored as `toString()`; throws `invalid parameter` if `this` is not an Atomic |

Copying an `Atomic` into a thread shares the cell. No compare and swap,
no increment.

### Processor

| Symbol | Returns | Notes |
|--------|---------|-------|
| `Processor.getCount()` | Number | logical processors |

### What crosses a thread boundary

| Value | Crosses as |
|-------|------------|
| `undefined`, `null`, Boolean, Number, String | copy |
| Array, Object | deep copy |
| Function | `undefined` (pass its source text, or load it in the thread) |
| Atomic | same shared cell |
| Thread | new empty Thread |
| variables of the enclosing scope, globals | not visible |

## C++

Namespace `XYO::QuantumScript::Extension::Thread`; export macro
`XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT` (empty with
`XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY`).

| Symbol | Header | Notes |
|--------|--------|-------|
| `registerInternalExtension(Executive *)` | `Thread.hpp` | registers `"Thread"` as internal |
| `initExecutive(Executive *, void *extensionId)` | `Thread.hpp` | extension init, called by the engine |
| `quantumScriptExtension(Executive *, void *)` | DLL export | `extern "C"`, DLL builds only |
| `VariableThread` | `Thread/VariableThread.hpp` | type `"Thread.Thread"`; `newVariable`, `start`, `startBytecode`, `join`, `isRunning`, `thread`, `requestToTerminateThread`, `returnedValue` |
| `VariableAtomic` | `Thread/VariableAtomic.hpp` | type `"Thread.Atomic"`; `newVariable` (new cell), `value` (`Atomic *`); `clone` shares the cell |
| `Atomic` | `Thread/Atomic.hpp` | `typeUnknown` / `typeBoolean` / `typeNumber` / `typeString`, `get`, `setBoolean`, `setNumber`, `setString`, `clear`, `incReferenceCount`, `decReferenceCount` |
| `ThreadContext`, `getContext()` | `Thread/Context.hpp` | per thread: prototypes, symbols, `currentThread` |

| Build project | Kind | Defines |
|---------------|------|---------|
| `quantum-script--thread` | DLL / shared library | exports `quantumScriptExtension` |
| `quantum-script--thread.static` | static library, static CRT | `XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY` for users |
