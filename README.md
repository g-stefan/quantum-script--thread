# Quantum Script Extension Thread

Quantum Script extension
- Run script functions on other system threads, in parallel, and collect
what they return (`Thread.newThread`, `join`, `getReturnedValue`).
- Start a thread from a function, a script file (also `.qsbc` bytecode) or a
string; poll it (`isRunning`) or stop it cooperatively (`requestToTerminate`).
- Share a value between threads with `Atomic`; size the work with
`Processor.getCount()`.
- Windows and Linux, the same API; each thread runs its own Quantum Script
executive, data is copied in and out.

```javascript
Script.requireExtension("Thread");

CurrentThread;
CurrentThread.sleep(milliSeconds);
CurrentThread.isRequestToTerminate();
Thread();
Thread.isThread(x);
Thread.prototype.start(function_,this_,arguments_);
Thread.prototype.startFromFile(file_,this_,arguments_);
Thread.prototype.startFromString(string_,this_,arguments_);
Thread.prototype.join();
Thread.prototype.getReturnedValue();
Thread.prototype.isRunning();
Thread.prototype.isTerminated();
Thread.prototype.requestToTerminate();
Thread.newThread(function_,this_,arguments_);
Thread.newThreadFromFile(file_,this_,arguments_);
Thread.newThreadFromString(string_,this_,arguments_);
Atomic();
Atomic.isAtomic(x);
Atomic.prototype.get();
Atomic.prototype.set(value);
Processor.getCount();
```

Built on `quantum-script` and `XYO::Multithreading` (`xyo-multithreading`),
part of the XYO C++ SDK.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, a first script, fabricare scripts, register in a C++ host, static builds
- [Thread model](docs/thread-model.md) - one executive per thread, copied and shared values, lifetime, stopping, errors
- [Script API](docs/script-api.md) - every function: exact behavior, edge cases
- [Recipes](docs/recipes.md) - parallel map, progress, timeout, stop flag, pool, worker files, error reporting
- [C++ API](docs/cpp-api.md) - `VariableThread`, `VariableAtomic`, `Atomic`, using them from native code
- [API reference](docs/reference.md)

A Claude Code skill for this extension is in
[.claude/skills/quantum-script--thread](.claude/skills/quantum-script--thread/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
