---
name: quantum-script--thread
description: >-
  How to use the Quantum Script Thread extension (quantum-script--thread),
  the multithreading extension loaded with Script.requireExtension("Thread"):
  Thread.newThread(fn, this_, arguments_) / newThreadFromFile /
  newThreadFromString, new Thread() with start / startFromFile /
  startFromString, join, getReturnedValue, isRunning, isTerminated,
  requestToTerminate; CurrentThread.sleep(ms) and
  CurrentThread.isRequestToTerminate(); Atomic (get / set, a value shared
  between threads); Processor.getCount(); the isolation rules (each thread
  is a fresh executive with empty globals, the function is moved as source
  text so closures are lost, this_ / arguments / return value are deep
  copied, functions become undefined, extensions must be required again in
  the thread), cooperative stop, lost exceptions, bytecode / --strip;
  the C++ side (VariableThread, VariableAtomic, Atomic,
  registerInternalExtension). Use when writing or reviewing Quantum Script
  or fabricare .js code that starts threads, runs work in parallel, polls a
  background job or shares a flag between threads, C++ code that includes
  <XYO/QuantumScript.Extension/Thread.hpp>, a fabricare.json depending on
  "quantum-script--thread", or when working inside the
  quantum-script--thread repository.
---

# quantum-script--thread

This is the multithreading extension of Quantum Script. The rules of the
`quantum-script` skill (the language and its differences from JavaScript)
and of the `xyo-multithreading` skill (the underlying
`XYO::Multithreading::Thread`) apply here too. Purpose: **run script
functions on other system threads, in parallel, and collect their
results**. Use it to spread CPU work over all cores, keep the main thread
free while a slow job runs, or run worker scripts in the background.
Windows and Linux use the same API.

Full documentation lives in `docs/` of the quantum-script--thread
repository (`X:\Storage\XYO\Gitea\CPP\quantum-script--thread\docs` on this
machine):

- README
- getting-started
- **thread-model**: one executive per thread, what is copied / shared, how
  the code is moved, lifetime, stopping, errors
- **script-api**: the exact behavior of every function
- **recipes**: parallel map, progress, timeout, stop flag, pool, worker
  files, shared helpers, error reporting, bytecode
- cpp-api
- reference

Read the matching page when you need more than this summary. When in doubt,
read `source/XYO/QuantumScript.Extension/Thread/Library.cpp` (~350 lines),
`VariableThread.cpp` (~300 lines) and `Library.js`.

## Script API

```javascript
Script.requireExtension("Thread");        // fabricare scripts have it already

var t = Thread.newThread(function(a, b) { // started Thread, or null
	return this.k + a + b;                 // sees ONLY this, its arguments, what it loads
}, {k: 1}, [2, 3]);                       // this_ and arguments_ are DEEP COPIED in
t.join();                                 // wait; safe to repeat
t.getReturnedValue();                     // 6: a COPY; undefined if it threw / returned nothing

Thread.newThreadFromFile("worker.js", this_, args);    // file = function BODY (top level return); .qsbc too; path vs working dir
Thread.newThreadFromString("return arguments[0] * 2;", null, [21]);   // string = function BODY

var u = new Thread();                     // or Thread(); nothing started
u.start(fn, this_, args);                 // true / false (compile or setup error); waits for a previous run first
u.startFromFile(file, this_, args);  u.startFromString(code, this_, args);
u.isRunning();                            // no wait; collects the result when it sees the end
u.isTerminated();                         // !isRunning(); TRUE before start
u.requestToTerminate();                   // sets a flag only; cleared by the next start
Thread.isThread(x);                       // typeof(t) == "Thread.Thread"

CurrentThread.sleep(ms);                  // milliseconds, any thread
CurrentThread.isRequestToTerminate();     // in a started thread: Boolean; main thread: undefined

var a = new Atomic();                     // shared cell, empty; typeof(a) == "Thread.Atomic"
a.set(5); a.get();                        // Boolean / Number / String kept; null / undefined empty it -> get() null
a.set([1, 2]);                            // anything else stored as toString(): "1,2"
Atomic.isAtomic(x);

Processor.getCount();                     // logical processors
```

## Hard rules

1. **Each thread is a fresh script world.** It has its own executive,
   globals, symbols and garbage collector. Variables and functions of the
   creating script are **not visible** (no closures, no globals). `Console`,
   `Shell`, `Thread`, `Atomic`, `CurrentThread.sleep` /
   `isRequestToTerminate` are missing until the thread itself calls
   `Script.requireExtension(...)`. A missing name throws inside the thread,
   which ends it with `getReturnedValue()` → `undefined`.
2. **Require `Thread` in the thread** before calling `get` / `set` on an
   `Atomic` passed in, or `CurrentThread.sleep` /
   `CurrentThread.isRequestToTerminate()`.
3. **The function is moved as its source text** (`"return " + fn + ";"`
   compiled in the thread). Only `--strip`ped functions (no source) are moved
   as bytecode. `start` accepts any value via `toString`; pass a script
   function, not a native one (`Console.writeLn` "starts" and returns
   `undefined`).
4. **Data crosses by deep copy**: `this_`, `arguments_` (must be an Array,
   else no arguments) and the returned value. Changing a copy never changes
   the original. **Functions become `undefined`**, also inside arrays /
   objects. To pass one, pass `"" + fn` and rebuild it in the thread with
   `Script.getFunctionFromString("return " + text + ";")()`. A `Thread`
   copies as a new **empty** Thread.
5. **`Atomic` is the only shared value.** A copy is a handle to the same
   cell. `get` / `set` are each locked, but there is **no compare and swap
   and no increment**: `a.set(a.get() + 1)` from several threads loses
   updates. Use it for flags, status, progress written by one thread. Return
   per-thread results with `getReturnedValue()` and combine them in the
   owner.
6. **Exceptions are lost.** An uncaught exception in the thread makes the
   returned value `undefined`, with no message. Wrap the body in
   `try { ... return {ok: true, value: v}; } catch (e) { return {ok: false, error: "" + e}; }`.
7. **Start errors**: `start*` → `false`, `Thread.newThread*` → `null`
   (compile error, unreadable file). Always check before `join()`. A
   stripped function that cannot be unlinked makes `start` throw
   `Thread start: ...`.
8. **Stopping is cooperative.** There is no kill. `requestToTerminate()`
   only sets a flag; the thread must test
   `CurrentThread.isRequestToTerminate()` (or a shared `Atomic`) between
   steps and return, then the owner `join()`s. `isRequestToTerminate()` is
   `undefined` in the main thread, so `!CurrentThread.isRequestToTerminate()`
   loops forever there.
9. **Collect every thread.** A finished thread waits until the owner
   collects its result with `join()` or an `isRunning()` / `isTerminated()`
   that sees the end. A poll loop `while (t.isRunning()) { ... }` needs no
   `join()` afterwards to read the value.
10. **Releasing a running Thread blocks.** When a Thread object is garbage
    collected or the script ends, it requests termination and **waits**. A
    thread that ignores the request keeps the process alive.
11. **`start` on a used object waits** for the previous run (implicit
    `join`) before starting the new one. `start` itself also waits until the
    new thread has compiled its code, so it can take a little time.
12. **Relative paths are against the working directory** in
    `newThreadFromFile` and in `Script.include` inside a thread (not the
    main script folder). Pass absolute paths, e.g. `Shell.realPath(...)`.
13. **`isTerminated()` is `true` before `start`**, and
    `getReturnedValue()` keeps the previous run's value until the next run
    is collected.
14. **Threads cost an executive each.** Start about `Processor.getCount()`
    CPU bound threads, give each a slice of many items, not one thread per
    small item.

## Patterns

```javascript
// parallel over all cores, results in order
var n = Processor.getCount(), threads = [];
for (var k = 0; k < n; ++k) {
	threads[k] = Thread.newThread(function(slice) { /* ... */ return out; }, null, [slices[k]]);
};
for (var k = 0; k < n; ++k) { threads[k].join(); merge(threads[k].getReturnedValue()); };

// background job + progress
var progress = new Atomic(); progress.set(0);
var job = Thread.newThread(function(progress) {
	Script.requireExtension("Thread");
	for (var i = 1; i <= 100; ++i) { /* step */ progress.set(i); };
	return "done";
}, null, [progress]);
while (job.isRunning()) { show(progress.get()); CurrentThread.sleep(100); };
job.getReturnedValue();

// timeout: poll isRunning, after the limit requestToTerminate(); join();
// stop a group: one shared Atomic flag, each thread loops while (!stop.get())
// pool of N: start while running.length < N, drop entries whose isRunning() is false
```

`docs/recipes.md` has complete, tested versions of these, plus worker
files, shared helper files and error reporting. For process / thread jobs
with a concurrency limit and time limits, use `quantum-script--job`
(`Job.addThread`), which is built on this extension.

## C++

```cpp
#include <XYO/QuantumScript.Extension/Thread.hpp>                  // register / init
#include <XYO/QuantumScript.Extension/Thread/VariableThread.hpp>   // Thread value
#include <XYO/QuantumScript.Extension/Thread/VariableAtomic.hpp>   // Atomic value + Atomic cell
using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {                         // host init callback,
	Extension::Thread::registerInternalExtension(executive);       // runs again in EVERY new thread:
};                                                                 // register all extensions threads need

if (!TIsType<Extension::Thread::VariableAtomic>(arguments->index(0))) { throw(Error("invalid parameter")); };
Extension::Thread::Atomic *cell = ((Extension::Thread::VariableAtomic *)(arguments->index(0)).value())->value;
bool b = false; Number n = 0; String s;
int type = cell->get(b, n, s);            // Atomic::typeUnknown / typeBoolean / typeNumber / typeString
cell->setNumber(1); cell->setString("x"); cell->clear();
```

- Inside `XYO::QuantumScript::Extension`, `Thread` is the namespace: write
  `XYO::Multithreading::Thread` for the system thread class.
- `VariableThread::start(executive, sourceCode, this_, args)`: `sourceCode`
  must `return` the function (`"return function(){ ... };"`).
  `startBytecode` takes a `Bytecode::save`d unit and a
  `ParserFunctionHint`. `join()`, `isRunning()`, `returnedValue`,
  `requestToTerminateThread.set(true)`.
- `VariableAtomic::newVariable()` creates a **new** cell; `clone` shares
  it. `Atomic` is reference counted (`incReferenceCount` /
  `decReferenceCount`), not managed memory.
- Static build: `quantum-script--thread.static` defines
  `XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY` (empty export macro, no
  `quantumScriptExtension` entry point). Register it as internal.

## Working in this repository

- Build with `fabricare make`, `fabricare test` and `fabricare install` (see
  the `fabricare` skill). `fabricare test` builds and runs `test.01` and
  `test.03` (C++ hosts with Console and Thread internal, run in
  `output/test`, scripts referenced as `../../test/...`) and then runs
  `quantum-script test/test.02.js`. Install `quantum-script` and
  `quantum-script--console` first.
- Native methods live in `Thread/Library.cpp` as
  `static TPointer<Variable> threadName(VariableFunction *, Variable *this_, VariableArray *arguments)`.
  They check `TIsType<VariableThread>(this_)` (or `VariableAtomic`) first and
  are registered in `initExecutive` with
  `executive->setFunction2("Thread.prototype.name(args)", fn)`.
  Static functions use `"Thread.name(args)"`.
- The thread machinery (start, setup, copying values in and out, the
  `threadStarted` / `threadTerminated` / `threadWaitCopyReturnValue`
  semaphores, `join`, `isRunning`) is in `VariableThread.cpp`. The shared
  cell is `Atomic.hpp`.
- `Thread.newThread*` are written in script in `Library.js`.
  `fabricare/make.prepare.js` (`file-to-cs`) turns it into
  `Library.Source.cpp` (`librarySource`), compiled at the end of
  `initExecutive`. Edit `Library.js`, not the generated file.
- When you add a method, update `README.md`, `docs/script-api.md`,
  `docs/reference.md` and this skill.
- Code style: tabs (width 8), `.clang-format`, CRLF, statements and blocks
  end with `};`, camelCase. SPDX headers: MIT for `source/` and `docs/`,
  Unlicense for `test/`, `fabricare/` and `.claude/` (see `.reuse/dep5`).
