# Thread model

This page explains what happens when a script starts a thread: where the
code runs, what the new thread can see, what is copied, what is shared, and
how a thread ends. Read it before you write threaded scripts: most
surprises come from the isolation rules below.

## One executive per thread

Quantum Script keeps its whole state per system thread: the executive
(`ExecutiveX`), the global object, the symbol table, the prototypes of the
extensions and the garbage collector. Starting a thread
(`VariableThread::start_`) does this:

1. The calling thread starts a system thread (`XYO::Multithreading::Thread`)
   and **waits** until the new thread has finished its setup.
2. The new thread creates its own executive with
   `ExecutiveX::initExecutive(mainCmdN, mainCmdS, applicationInitExecutive)`:
   the same command line and the same host init callback as the main
   executive. Internal extensions the host registered are available to it;
   none of them is *loaded* yet.
3. It builds the thread function (see [Moving the code](#moving-the-code))
   and sets `CurrentThread`.
4. It copies `this_` and `arguments_` into its own heap.
5. It tells the caller the start succeeded (or failed). `start` returns
   `true` (or `false`) at this point. The function then runs in parallel
   with the caller.
6. When the function returns, the thread waits until the owner collects the
   result (`join`, or `isRunning` / `isTerminated` seeing the end). The
   result is copied into the owner's heap, and then the thread frees its
   executive and ends.

So a new thread is a **fresh, empty script world**. It shares no script
objects with its creator.

| In the thread function | Available? |
|------------------------|------------|
| `this`, the arguments | yes, as copies |
| `Script`, `String`, `Array`, the core language | yes |
| Variables / functions of the enclosing script (closures, globals) | **no**, they are `undefined` |
| `Console`, `Shell`, `File`, ... | only after `Script.requireExtension("...")` in the thread |
| `Thread`, `Atomic`, `CurrentThread.sleep`, `CurrentThread.isRequestToTerminate`, `Processor` | only after `Script.requireExtension("Thread")` in the thread |
| Methods of an `Atomic` passed as argument | only after `Script.requireExtension("Thread")` in the thread |
| `Script.include("x.js")` | yes; a relative path is resolved against the process working directory |

A missing extension does not crash the thread: the call throws, the thread
function ends, and its returned value is `undefined`. When a thread
returns `undefined` unexpectedly, check its `Script.requireExtension`
calls first.

## Moving the code

The thread function is compiled again in the new executive. How it gets
there depends on how the thread was started:

| Start | What the new thread compiles |
|-------|------------------------------|
| `start(fn, ...)`, `fn` with source (normal script) | `"return " + fn.toString() + ";"`: the **source text** of the function |
| `start(fn, ...)`, `fn` without source (stripped bytecode, `--strip`) | the function is unlinked (`Linker::unlinkFunction`) into a code unit, saved as bytecode, loaded by the thread |
| `startFromFile(file, ...)`, source file | `"return function(){" + contents + "\n};"`: the file is the **body** of the function |
| `startFromFile(file, ...)`, `.qsbc` file (detected by its header, not its name) | the bytecode unit becomes the function, its included units are registered |
| `startFromString(code, ...)` | `"return function(){" + code + "\n};"` |

Consequences:

- **Closures are not carried over.** Only the text (or code) of the
  function moves, never the scope it was created in. Pass everything the
  function needs as `this_` / arguments, or load it inside the thread
  (`Script.include`, `Script.requireExtension`).
- A compile error in the moved code makes `start*` return `false` and
  `Thread.newThread*` return `null`.
- `start` accepts any value: it is converted with `toString`. A string that
  is not a function expression usually fails to compile (`false`). A value
  that compiles but is not callable (a number, a native function such as
  `Console.writeLn`) starts, and its returned value is `undefined`. Pass a
  script function.
- A file or string body can use `return`, `this` and `arguments`, and may end
  with a `//` comment (a new line is added before `}`).
- In a `--strip`ped bytecode program, functions have no source. `start`
  moves them as bytecode, so threads work in stripped programs as well.

## Copied values

`this_`, the arguments and the returned value cross threads by **deep
copy** (`Variable::clone`) into the other thread's heap. Changing a copy
never changes the original.

| Value | Crosses as |
|-------|------------|
| `undefined`, `null`, Boolean, Number, String | a copy of the value |
| Array, Object | a deep copy (nested arrays / objects are copied too) |
| Function (script or native) | **dropped**: becomes `undefined`, also inside an array / object |
| `Atomic` | a new handle to the **same** shared value (see below) |
| `Thread` | a new, **empty** `Thread` object: not the running thread |
| other extension values | whatever their `clone` does; most copy their content |

If copying `this_` fails, the thread gets `undefined` as `this`. If
`arguments_` is not an Array, or copying it fails, the thread gets no
arguments. If copying the returned value fails, `getReturnedValue()` is
`undefined`.

Copies cost time and memory proportional to their size. Large inputs are
fine (an array of 100 000 numbers copies in milliseconds), but pass each
thread only its own slice when you can.

## Shared values: Atomic

An `Atomic` is a reference counted cell (`Atomic` in `Atomic.hpp`) that
lives outside every script heap. Each `VariableAtomic` is a handle to it.
Copying the handle into a thread (as `this_`, an argument, or inside one)
adds a reference to the **same** cell. The cell is freed when the last
handle in any thread is released.

- It holds one of: Boolean, Number, String, or nothing (`get` returns
  `null`).
- `get` and `set` take the cell's `CriticalSection`. A reader always sees a
  whole value, never a string being changed. Strings are copied in and out,
  and memory is allocated outside the lock.
- `set` stores Boolean / Number / String as they are. `null` and
  `undefined` clear the cell. Any other value is stored as its `toString()`
  (an array becomes `"1,2"`).
- There is **no compare and swap and no increment**. `a.set(a.get() + 1)`
  from two threads loses updates. Use an `Atomic` for a flag, a status, or
  the latest progress value written by one thread, and return per-thread
  results with `getReturnedValue()` instead of counting in an `Atomic`.

## Lifetime of a Thread object

| State | `isRunning()` | `isTerminated()` | `getReturnedValue()` |
|-------|---------------|------------------|----------------------|
| new, never started | `false` | `true` | `undefined` |
| started, function running | `true` | `false` | value of the previous run, or `undefined` |
| function returned, result not collected yet | first call sees the end, collects, returns `false` | `true` | the new result once collected |
| after `join()` | `false` | `true` | the copied result |

- `join()` waits for the end and collects the result. It returns at once
  when there is nothing to wait for, so calling it twice is safe.
- `isRunning()` / `isTerminated()` never wait. When they see that the
  function returned, they collect the result just like `join()`. A poll loop
  with `isRunning()` therefore does not need a `join()` afterwards to read
  the value (calling `join()` anyway is harmless).
- `start*` on a `Thread` object that already ran first **waits** for the
  previous run to end (an implicit `join`), then starts again. The returned
  value of the previous run stays readable until the new run is collected.
- A finished thread stays parked until its result is collected. Collect
  every thread you start: `join()` it or poll it until it is not running.
- When a `Thread` object is released (the garbage collector, or the end of
  the script) while its thread still runs, it **requests termination and
  then waits** for the thread. A thread that never checks
  `CurrentThread.isRequestToTerminate()` keeps the process alive until it
  ends by itself.

## Stopping a thread

There is no way to kill a script thread. Stopping is **cooperative**:

```javascript
// owner
t.requestToTerminate();     // sets a flag, returns at once
t.join();                   // waits for the thread to notice and return

// in the thread
Script.requireExtension("Thread");
while (!CurrentThread.isRequestToTerminate()) {
	// ... a slice of work ...
	CurrentThread.sleep(10);
};
return "stopped";
```

- The flag is cleared when the object starts a new run.
- `CurrentThread.isRequestToTerminate()` returns `undefined` in the main
  thread (and in any thread not started by this extension). Use `!` or
  `== true`, which both handle that.
- A thread blocked in a long native call (a process wait, a socket read)
  notices the flag only after that call returns. Use timeouts in such calls.

## Errors

- An exception thrown in the thread function, and not caught there, ends
  the function. The returned value is `undefined`. The error message is
  **not** reported to the owner. To report errors, catch them in the thread
  and return them:

  ```javascript
  var t = Thread.newThread(function(file) {
  	try {
  		Script.requireExtension("File");
  		// ...
  		return {ok: true, value: result};
  	} catch (e) {
  		return {ok: false, error: "" + e};
  	};
  }, null, [file]);
  ```

- Start errors (compile error, unreadable file, executive init failure)
  make `start*` return `false` / `Thread.newThread*` return `null`.
- A `--strip`ped function that cannot be unlinked makes `start` **throw**
  `Thread start: <reason>`.

## Threads and the host

- Each thread runs `applicationInitExecutive`, the init callback the host
  gave to `ExecutiveX::initExecutive`. Register in that callback every
  internal extension the threads may need.
- The thread's executive is ended with `ExecutiveX::endProcessing()` when
  the thread ends.
- The extension keeps per-thread state in `ThreadContext`
  (`TSingleton<ThreadContext>`, one per thread): the `Thread` / `Atomic`
  prototypes, created when the thread requires the extension, and
  `currentThread`, the `VariableThread` running this system thread
  (`nullptr` in the main thread).
- `CurrentThread.this_` holds a resource pointing to the running
  `VariableThread`. It is internal; scripts should not use it.
