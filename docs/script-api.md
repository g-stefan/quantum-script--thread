# Script API

This page lists everything the extension defines after
`Script.requireExtension("Thread")`. Remember that this call is needed **in
each thread** that uses these names (see [Thread model](thread-model.md)).
Arguments are converted the engine's way: `toString` for text and file
names, `toIndex` for counts. A missing argument is `undefined`.

## Thread

### `Thread()` / `new Thread()`

Creates a `Thread` object. No system thread is started yet. Arguments are
ignored. `typeof(t)` is `"Thread.Thread"` and `"" + t` is
`"Thread.Thread"`. A `Thread` object is always truthy.

### `Thread.isThread(x)`

`true` if `x` is a `Thread` object, `false` otherwise.

### `t.start(function_, this_, arguments_)`

Starts a new system thread that calls `function_` with `this_` and the
elements of `arguments_` as arguments.

- `function_`: a script function. Its source text is compiled again in the
  new thread, so it **cannot use variables of the enclosing scope**. A
  function without source (from `--strip`ped bytecode) is moved as
  bytecode instead. Other values are converted with `toString` (see
  [Moving the code](thread-model.md#moving-the-code)).
- `this_`: copied into the thread; `undefined` if missing.
- `arguments_`: an Array, copied into the thread; anything else means no
  arguments.

If the object already ran a thread, `start` first waits for it to end (an
implicit `join()`). It then waits until the new thread has created its
executive and compiled the function, and returns:

- `true`: the function is running in parallel.
- `false`: the thread could not be set up (compile error, executive init
  error).

Throws `Thread start: <reason>` if a function without source cannot be
moved as bytecode. Returns `undefined` if `this` is not a `Thread`.

### `t.startFromFile(file_, this_, arguments_)`

Like `start`, with the code read from the file `file_`. The path is opened
as given, so a relative path is resolved against the process working
directory, not the script folder.

- A source file is the **body** of the thread function: top level
  `return value;` gives the returned value, and `this` / `arguments` are
  `this_` / `arguments_`. It may end with a `//` comment.
- A Quantum Script bytecode file (`.qsbc`, recognized by its header, not by
  its extension) is loaded as a code unit. Units bundled in it with
  `--add` are available to `Script.include` in the thread.

Returns `false` if the file cannot be read or does not compile.

### `t.startFromString(string_, this_, arguments_)`

Like `start`, with `string_` as the **body** of the thread function:
`"return arguments[0] * 2;"`. It may end with a `//` comment. Returns
`false` on a compile error.

### `t.join()`

Waits for the thread function to return, then copies its returned value
into this thread (see `getReturnedValue`) and waits for the system thread
to end. Returns at once if the thread was never started or was already
collected; calling it again is safe. Returns `undefined`.

### `t.getReturnedValue()`

The value returned by the thread function of the last collected run, as a
copy owned by this thread. It is `undefined`:

- before the first run is collected (`join()`, or `isRunning()` /
  `isTerminated()` seeing the end),
- if the function returned nothing or threw an exception,
- if the value could not be copied.

Functions inside the returned value become `undefined` (see
[Copied values](thread-model.md#copied-values)). Every call returns the same
object, until the next run is collected.

### `t.isRunning()`

`true` while the thread function runs. Never waits. Returns `false` for a
never-started object and once the function has returned. When it is the
call that notices the end, it collects the returned value first, as `join()`
would.

### `t.isTerminated()`

`!t.isRunning()`, with the same collection side effect. Note that it is
`true` for an object that was never started.

### `t.requestToTerminate()`

Sets the stop request flag of the thread and returns at once. It does not
stop anything by itself: the thread function must test
`CurrentThread.isRequestToTerminate()` and return. The flag is cleared when
the object starts a new run. Releasing a running `Thread` object (garbage
collection, end of script) sets the flag too, then waits for the thread.

### `Thread.newThread(function_, this_, arguments_)`

```javascript
var thread = new Thread();
if (thread.start(function_, this_, arguments_)) {
	return thread;
};
return null;
```

Returns the started `Thread`, or `null`. Defined in script
(`Library.js`).

### `Thread.newThreadFromFile(file_, this_, arguments_)`

`new Thread()` + `startFromFile`; the `Thread`, or `null`.

### `Thread.newThreadFromString(string_, this_, arguments_)`

`new Thread()` + `startFromString`; the `Thread`, or `null`.

## CurrentThread

`CurrentThread` is a global object, created if it does not exist.

### `CurrentThread.sleep(milliSeconds)`

Suspends the calling thread for `milliSeconds` milliseconds (converted with
`toIndex`). Works in the main thread too. Returns `undefined`.

### `CurrentThread.isRequestToTerminate()`

In a thread started by this extension: `true` once its owner called
`requestToTerminate()` (or released the `Thread` object), `false` before.
In the main thread, or any thread not started by `Thread`: `undefined`.

`while (!CurrentThread.isRequestToTerminate())` is correct in both cases,
but in the main thread it never ends.

## Atomic

### `Atomic()` / `new Atomic()`

Creates a new shared value cell, empty (`get()` returns `null`). Arguments
are ignored. `typeof(a)` is `"Thread.Atomic"`. An `Atomic` is always truthy.

Passing an `Atomic` to a thread (as `this_`, an argument, or inside an
array / object) gives the thread a handle to the **same** cell. The thread
must `Script.requireExtension("Thread")` before it calls `get` / `set`.

### `Atomic.isAtomic(x)`

`true` if `x` is an `Atomic`, `false` otherwise.

### `a.get()`

Returns a copy of the current value, read under the cell's lock:

| Stored | Returns |
|--------|---------|
| Boolean | Boolean |
| Number | Number |
| String | String |
| nothing (new, or set to `null` / `undefined`) | `null` |

Throws `invalid parameter` if `this` is not an `Atomic`.

### `a.set(value)`

Replaces the value, under the cell's lock:

| `value` | Stored as |
|---------|-----------|
| Boolean, Number, String | itself (its type is kept: `1` stays a Number, `"1"` a String) |
| `null`, `undefined` (also missing) | nothing: `get()` returns `null` |
| anything else (Array, Object, ...) | `value.toString()`: `[1, 2]` becomes `"1,2"` |

Returns `undefined`. Throws `invalid parameter` if `this` is not an
`Atomic`.

`get` and `set` are each atomic, but a `get` followed by a `set` is not:
there is no compare and swap and no increment. Do not count across threads
with `a.set(a.get() + 1)`.

## Processor

### `Processor.getCount()`

The number of logical processors of the machine (`Processor::getCount()`
of `xyo-platform`), as a Number. Use it to decide how many CPU bound
threads to start. `Processor` is a global object, created if it does not
exist.
