# Recipes

Every recipe below is a complete script, tested with `quantum-script`.
Each thread function calls `Script.requireExtension` for what it uses,
because a new thread starts with empty globals (see
[Thread model](thread-model.md)).

## Parallel map on all cores

Split the input into one slice per processor, run the slices in parallel,
and put the results back in input order. Functions cannot be passed to a
thread, but their **source** can: `"" + work` is the text of the function,
and `Script.getFunctionFromString("return " + text + ";")()` rebuilds it in
the thread. The rebuilt function sees only its own arguments.

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

// Apply work(item) to every item, on all cores; results keep the input order.
function parallelMap(items, work) {
	var count = Processor.getCount();
	if (count > items.length) {
		count = items.length;
	};
	var threads = [];
	for (var k = 0; k < count; ++k) {
		// slice k: items k, k + count, k + 2 * count, ...
		var slice = [];
		for (var i = k; i < items.length; i += count) {
			slice[slice.length] = items[i];
		};
		threads[k] = Thread.newThread(function(work, slice) {
			var fn = Script.getFunctionFromString("return " + work + ";")();
			var out = [];
			for (var i = 0; i < slice.length; ++i) {
				out[i] = fn(slice[i]);
			};
			return out;
		}, null, ["" + work, slice]);
	};
	var results = [];
	for (var k = 0; k < count; ++k) {
		threads[k].join();
		var out = threads[k].getReturnedValue();
		for (var i = 0; i < out.length; ++i) {
			results[k + i * count] = out[i];
		};
	};
	return results;
};

var squares = parallelMap([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], function(x) {
	return x * x;
});
Console.writeLn(squares.join(","));
```

Prints `1,4,9,16,25,36,49,64,81,100`.

Use many small items per slice: each thread costs an executive set up
and a copy of its slice, so a thread per item is slower than a loop for
cheap work.

## Background job with progress

The main thread stays free while the job runs. An `Atomic` carries the
progress; `isRunning()` polls without waiting, and collects the result when
it sees the end, so no `join()` is needed afterwards.

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

var progress = new Atomic();
progress.set(0);

var job = Thread.newThread(function(progress, total) {
	Script.requireExtension("Thread");    // for progress.set and CurrentThread
	for (var i = 1; i <= total; ++i) {
		CurrentThread.sleep(20);           // ... one step of real work ...
		progress.set(i * 100 / total);
	};
	return "done";
}, null, [progress, 50]);

var spinner = ["|", "/", "-", "\\"];
var k = 0;
while (job.isRunning()) {                  // also collects the result at the end
	Console.write("\r" + spinner[k % 4] + " " + progress.get() + "%   ");
	++k;
	CurrentThread.sleep(100);
};
Console.writeLn("\r" + job.getReturnedValue() + "        ");
```

Only the job writes `progress` and only the main thread reads it, so the
lack of compare and swap does not matter here.

## Timeout and cooperative stop

A thread cannot be killed. The owner asks it to stop with
`requestToTerminate()`; the thread checks `CurrentThread.isRequestToTerminate()`
between steps and returns what it has.

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

// Run fn in a thread, give up after timeoutMs; fn must poll CurrentThread.isRequestToTerminate()
function runWithTimeout(fn, this_, arguments_, timeoutMs) {
	var t = Thread.newThread(fn, this_, arguments_);
	if (!t) {
		return {ok: false, error: "not started"};
	};
	var waited = 0;
	while (t.isRunning()) {
		if (waited >= timeoutMs) {
			t.requestToTerminate();
			t.join();
			return {ok: false, error: "timeout", partial: t.getReturnedValue()};
		};
		CurrentThread.sleep(10);
		waited += 10;
	};
	return {ok: true, value: t.getReturnedValue()};
};

var r = runWithTimeout(function(limit) {
	Script.requireExtension("Thread");
	var n = 0;
	while (n < limit) {
		if (CurrentThread.isRequestToTerminate()) {
			return n;                       // partial result
		};
		++n;
		CurrentThread.sleep(1);
	};
	return n;
}, null, [1000000], 200);
Console.writeLn(r.ok + " " + r.error + " partial>0: " + (r.partial > 0));
```

Prints `false timeout partial>0: true`.

## Stop many threads with one flag

`requestToTerminate()` targets one thread. To stop a group, share one
`Atomic` and let each thread test it.

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

// One Atomic stops many threads at once
var stop = new Atomic();
stop.set(false);
var workers = [];
for (var k = 0; k < 4; ++k) {
	workers[k] = Thread.newThread(function(stop, id) {
		Script.requireExtension("Thread");
		var loops = 0;
		while (!stop.get()) {
			++loops;
			CurrentThread.sleep(5);
		};
		return "worker " + id + " stopped";
	}, null, [stop, k]);
};
CurrentThread.sleep(100);
stop.set(true);
for (var k = 0; k < workers.length; ++k) {
	workers[k].join();
	Console.writeLn(workers[k].getReturnedValue());
};
```

## A pool with at most N threads

Start a thread per task, but never more than `maxRunning` at once. Results
are handled in the main thread as each task ends. For process and thread
jobs with time limits, the `Job` extension (`quantum-script--job`) does the
same with more options.

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

// Run fn(task) for every task, at most maxRunning threads at once.
// onDone(task, result) is called in the main thread as each one ends.
function runPool(tasks, fn, maxRunning, onDone) {
	var running = [];                     // {thread, task}
	var next = 0;
	while ((next < tasks.length) || (running.length > 0)) {
		// start while there is room
		while ((next < tasks.length) && (running.length < maxRunning)) {
			var thread = Thread.newThread(fn, null, [tasks[next]]);
			if (thread) {
				running[running.length] = {thread: thread, task: tasks[next]};
			} else {
				onDone(tasks[next], undefined);
			};
			++next;
		};
		// collect the ones that ended
		var stillRunning = [];
		for (var k = 0; k < running.length; ++k) {
			if (running[k].thread.isRunning()) {
				stillRunning[stillRunning.length] = running[k];
			} else {
				onDone(running[k].task, running[k].thread.getReturnedValue());
			};
		};
		running = stillRunning;
		CurrentThread.sleep(1);
	};
};

var tasks = [];
for (var i = 0; i < 20; ++i) {
	tasks[i] = i;
};
var sum = 0;
runPool(tasks, function(n) {
	Script.requireExtension("Thread");
	CurrentThread.sleep(10 * (n % 3));
	return n * n;
}, Processor.getCount(), function(task, result) {
	sum += result;
});
Console.writeLn("sum of squares: " + sum);
```

Prints `sum of squares: 2470`.

## A worker file

The file is the **body** of the thread function: `this` and `arguments`
are the values passed to `newThreadFromFile`, and a top level `return`
gives the result. The path is relative to the working directory.

`workers/hash.js`:

```javascript
// Thread body: this = options, arguments = the files to process
Script.requireExtension("Thread");
var out = [];
for (var i = 0; i < arguments.length; ++i) {
	if (CurrentThread.isRequestToTerminate()) {
		break;
	};
	out[out.length] = this.prefix + arguments[i];
};
return out;
```

The script:

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

var t = Thread.newThreadFromFile("workers/hash.js", {prefix: "done: "}, ["a.txt", "b.txt"]);
if (!t) {
	throw "workers/hash.js: cannot read or compile";
};
t.join();
Console.writeLn(t.getReturnedValue().join(", "));
```

Prints `done: a.txt, done: b.txt`.

## Shared helper functions

The thread does not see the functions of the script that started it. Put
them in a file and include it in the thread. Pass an absolute path:
`Script.include` in a thread resolves a relative path against the working
directory, not the folder of the main script.

`lib/helpers.js`:

```javascript
function greet(name) { return "Hello, " + name + "!"; };
```

The script:

```javascript
Script.requireExtension("Console");
Script.requireExtension("Shell");
Script.requireExtension("Thread");

// The thread does not see the functions of this script: load them there,
// with an absolute path (a relative one depends on the working directory)
var helpers = Shell.realPath("lib/helpers.js");
var t = Thread.newThread(function(helpers, name) {
	Script.include(helpers);
	return greet(name);
}, null, [helpers, "thread"]);
t.join();
Console.writeLn(t.getReturnedValue());
```

Prints `Hello, thread!`.

## Reporting errors

An uncaught exception in a thread only makes its returned value
`undefined`; the message is lost. Catch it in the thread and return it.

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

var t = Thread.newThread(function(name) {
	try {
		if (name == "") {
			throw "empty name";
		};
		return {ok: true, value: name.toUpperCaseASCII()};
	} catch (e) {
		return {ok: false, error: "" + e};
	};
}, null, [""]);
t.join();
var r = t.getReturnedValue();
Console.writeLn(r.ok ? r.value : "thread failed: " + r.error);
```

Prints `thread failed: Error: empty name`.

## Threads in bytecode programs

Threads work in compiled programs, also stripped ones. In a `--strip`ped
program functions have no source text, so `start` moves the function as
bytecode instead.

```bash
quantum-script --compile=tool.qsbc --strip tool.js
quantum-script tool.qsbc
```

A worker file can be compiled too. `newThreadFromFile` recognizes a
bytecode file by its header, whatever its name:

```bash
quantum-script --compile=workers/hash.qsbc workers/hash.js
```

```javascript
var t = Thread.newThreadFromFile("workers/hash.qsbc", {prefix: "done: "}, ["a.txt"]);
```
