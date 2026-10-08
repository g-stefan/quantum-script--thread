# Quantum Script Extension Thread — Documentation

`quantum-script--thread` is the **multithreading extension of Quantum
Script**. You load it with `Script.requireExtension("Thread")`. It lets a
script run script functions on other operating system threads, in
parallel, and collect what they return. Windows and Linux use the same API.

Quantum Script runs one script per thread: each thread has its own
executive, its own global object, its own symbols and its own garbage
collector. The extension builds on that model:

- **Start a thread.** `Thread.newThread(fn, this_, arguments_)` starts a
  new system thread with a fresh Quantum Script executive and calls `fn`
  there. The function can also come from a file
  (`Thread.newThreadFromFile`) or from a string
  (`Thread.newThreadFromString`).
- **Pass data in, get a result back.** `this_` and `arguments_` are
  **copied** into the new thread. When the function returns, its result
  is **copied** back and `getReturnedValue()` gives it.
- **Wait, poll, stop.** `join()` waits for the end. `isRunning()` /
  `isTerminated()` poll without waiting. `requestToTerminate()` raises a
  flag that the thread reads with `CurrentThread.isRequestToTerminate()`:
  stopping is cooperative.
- **Share a value.** An `Atomic` is the one object that is *shared*, not
  copied, between threads. It holds a boolean, a number, a string or
  `null`, and every `get` / `set` is protected by a lock.
- **Size the work.** `Processor.getCount()` returns the number of logical
  processors. `CurrentThread.sleep(ms)` pauses the current thread.

```
scripts: fabricare build scripts, quantum-script .js, tools
quantum-script--job                    (Job.addThread: thread pool with time limits, built on this)
quantum-script--thread  <-- this extension: Thread, Atomic, CurrentThread, Processor
quantum-script                         (Executive, ExecutiveX, Variable, Bytecode, Linker)
xyo-system, xyo-multithreading         (XYO::Multithreading::Thread, Semaphore, CriticalSection, TAtomic)
xyo-encoding, xyo-data-structures, xyo-managed-memory, xyo-platform
```

## Why it exists

| Need | What the extension gives |
|------|--------------------------|
| Use every core for a CPU heavy job (hashing, parsing, image work) | `Processor.getCount()` threads, each with its own slice, results with `getReturnedValue()` |
| Keep doing something while a slow job runs | `isRunning()` polling loop, `CurrentThread.sleep(ms)` |
| Run a script file as a background worker | `Thread.newThreadFromFile("worker.js", this_, args)`, also `.qsbc` bytecode |
| Stop a long running loop cleanly | `requestToTerminate()` + `CurrentThread.isRequestToTerminate()` |
| Report progress or a status flag across threads | an `Atomic` shared by all threads |
| Run build steps in parallel | `fabricare` registers the extension as internal; `quantum-script--job` uses it for `Job.addThread` |

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Load the extension | `Script.requireExtension("Thread");` | **in every thread** that uses `Thread`, `Atomic` or `CurrentThread` |
| Start a function | `var t = Thread.newThread(fn, this_, [a, b]);` | `null` if it cannot start; `fn` sees **no outer variables** |
| Start a file | `Thread.newThreadFromFile("worker.js", this_, args)` | the file body is the function body; `return` works; `.qsbc` too |
| Start a string | `Thread.newThreadFromString("return arguments[0] * 2;", null, [21])` | the string is the function body |
| Wait and get the result | `t.join(); var r = t.getReturnedValue();` | result is a **copy**; `undefined` if the function threw |
| Poll | `t.isRunning()`, `t.isTerminated()` | when they see the end they also collect the result |
| Stop | `t.requestToTerminate(); t.join();` | cooperative: the thread must check `CurrentThread.isRequestToTerminate()` |
| Pause | `CurrentThread.sleep(ms)` | milliseconds; works in the main thread too |
| Share a value | `var a = new Atomic(); a.set(1); a.get();` | Boolean / Number / String / `null`; no compare and swap |
| Core count | `Processor.getCount()` | logical processors |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build and install, a first script, fabricare scripts, register it in a C++ host, static builds |
| [Thread model](thread-model.md) | One executive per thread, what is copied and what is shared, how the function is moved, lifetime, stopping, errors |
| [Script API](script-api.md) | Every function: arguments, exact behavior, edge cases, return values |
| [Recipes](recipes.md) | Parallel map over all cores, background job with polling, stop flag, progress, worker files, bytecode |
| [C++ API](cpp-api.md) | `registerInternalExtension`, `VariableThread`, `VariableAtomic`, `Atomic`, using them from native code |
| [API reference](reference.md) | Every script and C++ symbol on one page |

The underlying C++ classes (`XYO::Multithreading::Thread`, `Semaphore`,
`CriticalSection`, `TAtomic`) are documented in the `xyo-multithreading`
and `xyo-platform` repositories. Quantum Script itself (the language,
`Script.requireExtension`, bytecode, embedding) is documented in the
`quantum-script` repository, `docs/`.

## Source map

```
source/XYO/QuantumScript.Extension/Thread.hpp            umbrella header (Library.hpp)
source/XYO/QuantumScript.Extension/Thread.Amalgam.cpp    the whole extension in one translation unit
source/XYO/QuantumScript.Extension/Thread/
    Dependency.hpp                 <XYO/QuantumScript.hpp>, export macro
    Library[.hpp/.cpp]             initExecutive, registerInternalExtension, every native function
    Library.js                     Thread.newThread / newThreadFromFile / newThreadFromString, in Quantum Script
    Library.Source.cpp             Library.js as a C string (generated by fabricare/make.prepare.js)
    Context.hpp                    ThreadContext: prototypes per thread, the current Thread object
    VariableThread[.hpp/.cpp]      the Thread value: starts the system thread, its executive, copies values in and out
    Atomic.hpp                     Atomic: the reference counted shared cell, guarded by a CriticalSection
    VariableAtomic[.hpp/.cpp]      the Atomic value type (wraps Atomic)
    Copyright / License / Version  library metadata
fabricare/make.prepare.js          file-to-cs: Library.js -> Library.Source.cpp
fabricare/test.js                  builds and runs test.01 / test.03 (in output/test), then test/test.02.js with quantum-script
test/test.01.cpp, test.01.js       C++ host with Console and Thread internal, a thread from a file (test.01.thread.js)
test/test.02.js                    this_ and arguments copied into a thread
test/test.03.cpp, test.03.js       Atomic under concurrent readers / writers, stop request, start from string
```

## AI assistant skill

A Claude Code skill describing how to use this extension lives in
[`.claude/skills/quantum-script--thread/`](../.claude/skills/quantum-script--thread/SKILL.md).
Claude Code loads it automatically inside this repository. To use it in the
projects that use `Thread` (fabricare scripts, Quantum Script tools, other
extensions), copy the folder to `~/.claude/skills/`.
