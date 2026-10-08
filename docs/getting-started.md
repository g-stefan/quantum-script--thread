# Getting started

## 1. Build and install

The extension is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. First install `quantum-script`
(with everything below it: `xyo-system`, `xyo-multithreading`, ...) and
`quantum-script--console` to the SDK. Then, from the repository root:

```bash
fabricare make       # build into output/ (make.prepare first turns Library.js into Library.Source.cpp)
fabricare test       # build and run test.01 / test.03, then run test/test.02.js (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

`fabricare test` runs the C++ hosts `test.01` and `test.03` from
`output/test` with `output/bin` first on the `PATH`, so the extension just
built is used. Then it runs `quantum-script test/test.02.js` from the
repository root.

The build produces two libraries:

| Project                         | Kind                                | Use it when                                         |
|---------------------------------|-------------------------------------|-----------------------------------------------------|
| `quantum-script--thread`        | DLL / shared library (`dll-or-lib`) | scripts run by `quantum-script`, or a host using the engine DLL |
| `quantum-script--thread.static` | static library, static CRT          | self-contained hosts built with `quantum-script.static` |

After `fabricare install`, the SDK `bin` folder holds
`quantum-script--thread.dll` (Windows) / `libquantum-script--thread.so`
(Linux) next to `quantum-script.exe`. `Script.requireExtension("Thread")`
finds it there.

## 2. A first script

```javascript
Script.requireExtension("Console");
Script.requireExtension("Thread");

// The function runs in another thread, with its own globals:
// it sees only `this`, its arguments and what it loads itself.
var sum = function(from, to) {
	var s = 0;
	for (var i = from; i < to; ++i) {
		s += i;
	};
	return s;
};

var count = Processor.getCount();
var size = 1000000;
var step = (size - size % count) / count;
var threads = [];
for (var k = 0; k < count; ++k) {
	var to = (k == count - 1) ? size : (k + 1) * step;
	threads[k] = Thread.newThread(sum, null, [k * step, to]);
	if (!threads[k]) {
		throw "thread " + k + " did not start";
	};
};

var total = 0;
for (var k = 0; k < count; ++k) {
	threads[k].join();                      // wait for the end
	total += threads[k].getReturnedValue(); // a copy of what the function returned
};
Console.writeLn("total: " + total + " on " + count + " threads");
```

Run it with:

```bash
quantum-script first-thread.js
```

`Script.requireExtension("Thread")` first looks for an external
`quantum-script--thread` library: the file as named, then each include
path folder (next to the interpreter, next to the script). If none is
found, it uses an internal extension registered by the host. Loading the
extension twice does nothing. A missing extension throws
`Unable to open "Thread"`.

> **The new thread starts empty.** It has its own global object: `Console`,
> `Thread`, `Atomic`, `CurrentThread.sleep` and every variable or function
> of the main script are not there. Call `Script.requireExtension(...)`
> inside the thread function for each extension it uses. See
> [Thread model](thread-model.md).

## 3. fabricare build scripts

`fabricare` registers `Thread` as an internal extension and its library
already calls `Script.requireExtension("Thread")`, so build scripts can use
`Thread`, `Atomic`, `CurrentThread` and `Processor` directly:

```javascript
// fabricare/make.js (excerpt)
var jobs = [];
for (var k = 0; k < files.length; ++k) {
	jobs[k] = Thread.newThread(function(file) {
		Script.requireExtension("Shell");
		return Shell.system("tool --in=" + file);
	}, null, [files[k]]);
};
for (var k = 0; k < jobs.length; ++k) {
	jobs[k].join();
	exitIf(jobs[k].getReturnedValue());
};
```

For many commands with a limit on how many run at once, and a time limit,
use the `Job` extension (`quantum-script--job`): `Job.addThread(fn, fnThis,
parameters, info, synchronizedKey)` starts its threads with
`Thread.newThread` and sizes its pool with `Processor.getCount()`.

## 4. Register it in a C++ host

A host that embeds Quantum Script registers the extension in its init
callback. The same callback runs again for **every** new thread, so the
thread executives have the same internal extensions as the main one.

```cpp
#include <XYO/QuantumScript.hpp>
#include <XYO/QuantumScript.Extension/Console.hpp>
#include <XYO/QuantumScript.Extension/Thread.hpp>

using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {
	Extension::Console::registerInternalExtension(executive);
	Extension::Thread::registerInternalExtension(executive);
};

int main(int cmdN, char *cmdS[]) {
	if (ExecutiveX::initExecutive(cmdN, cmdS, initExecutive)) {
		if (!ExecutiveX::executeFile("script.js")) {
			printf("%s\n%s", ExecutiveX::getError().value(), ExecutiveX::getStackTrace().value());
		};
		ExecutiveX::endProcessing();
	};
	return 0;
};
```

Registering only makes the extension *available*. Scripts still call
`Script.requireExtension("Thread")`, in the main thread and in each new
thread. See `test/test.01.cpp`.

In `fabricare.json`, depend on `quantum-script--thread`:

```json
{
	"name" : "my-host",
	"make" : "exe",
	"dependency" : [
		"quantum-script--thread"
	]
}
```

## 5. Static builds

Link `quantum-script--thread.static` (it depends on
`quantum-script.static` and `quantum-script--console.static`). The static
project defines `XYO_QUANTUMSCRIPT_EXTENSION_THREAD_LIBRARY`, which:

- makes `XYO_QUANTUMSCRIPT_EXTENSION_THREAD_EXPORT` empty, and
- leaves out the `quantumScriptExtension` DLL entry point.

So in a static host the extension must be registered with
`Extension::Thread::registerInternalExtension(executive)`; there is no
library to load.
