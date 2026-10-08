// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Atomic shared by several threads, stop request, start from string

Script.requireExtension("Console");
Script.requireExtension("Thread");

var testName = "03";
var failed = 0;

function check(name, value, expected) {
	if (value === expected) {
		return;
	};
	Console.writeLn("-> test " + testName + " fail: " + name + " = " + value + " (expected " + expected + ")");
	++failed;
};

// --- Atomic, writers and readers at the same time

// strings of different sizes, a string read while it is changed is not one of these
var values = [];
var length = [1, 17, 100, 1000, 5000];
var letter = ["A", "B", "C", "D", "E"];
for (var k = 0; k < length.length; ++k) {
	var value = "";
	for (var i = 0; i < length[k]; ++i) {
		value += letter[k];
	};
	values[values.length] = value;
};

// set strings, numbers, booleans and null, to change the type too
var writer = function(atomic, values, iterations, id) {
	Script.requireExtension("Thread");
	for (var i = 0; i < iterations; ++i) {
		var k = (i + id) % (values.length + 3);
		if (k < values.length) {
			atomic.set(values[k]);
		} else if (k == values.length) {
			atomic.set(i);
		} else if (k == values.length + 1) {
			atomic.set((i % 2) == 0);
		} else {
			atomic.set(null);
		};
	};
	return iterations;
};

// returns the count of invalid values read
var reader = function(atomic, values, iterations) {
	Script.requireExtension("Thread");
	var invalid = 0;
	for (var i = 0; i < iterations; ++i) {
		var value = atomic.get();
		if (value === null || value === true || value === false || Script.isNumber(value)) {
			continue;
		};
		var found = false;
		for (var k = 0; k < values.length; ++k) {
			if (value === values[k]) {
				found = true;
				break;
			};
		};
		if (!found) {
			++invalid;
		};
	};
	return invalid;
};

var iterations = 20000;
var threadCount = 4;
var atomic = new Atomic();
var writers = [];
var readers = [];

for (var k = 0; k < threadCount; ++k) {
	writers[k] = Thread.newThread(writer, null, [atomic, values, iterations, k]);
	readers[k] = Thread.newThread(reader, null, [atomic, values, iterations]);
	check("writer " + k + " started", Thread.isThread(writers[k]), true);
	check("reader " + k + " started", Thread.isThread(readers[k]), true);
};

// main thread reads too
var mainInvalid = 0;
for (var i = 0; i < iterations; ++i) {
	var value = atomic.get();
	if (Script.isString(value)) {
		var found = false;
		for (var k = 0; k < values.length; ++k) {
			if (value === values[k]) {
				found = true;
				break;
			};
		};
		if (!found) {
			++mainInvalid;
		};
	};
};
check("main invalid reads", mainInvalid, 0);

for (var k = 0; k < threadCount; ++k) {
	writers[k].join();
	readers[k].join();
	check("writer " + k + " iterations", writers[k].getReturnedValue(), iterations);
	check("reader " + k + " invalid reads", readers[k].getReturnedValue(), 0);
	check("writer " + k + " terminated", writers[k].isTerminated(), true);
};

// all atomic objects share the same value
var atomicCheck = new Atomic();
var atomicThread = Thread.newThread(function(atomicCheck) {
	Script.requireExtension("Thread");
	atomicCheck.set("from thread");
	return true;
}, null, [atomicCheck]);
atomicThread.join();
check("value set by thread", atomicCheck.get(), "from thread");

// --- Stop request

var stopThread = Thread.newThread(function() {
	Script.requireExtension("Thread");
	while (!CurrentThread.isRequestToTerminate()) {
		CurrentThread.sleep(1);
	};
	return "stopped";
});
check("stop thread is running", stopThread.isRunning(), true);
stopThread.requestToTerminate();
stopThread.join();
check("stop thread returned value", stopThread.getReturnedValue(), "stopped");
check("stop thread is terminated", stopThread.isTerminated(), true);

// not in a thread
check("main thread is request to terminate", CurrentThread.isRequestToTerminate(), undefined);

// a released thread object asks the thread to stop, then waits for it
var releasedThread = Thread.newThread(function() {
	Script.requireExtension("Thread");
	while (!CurrentThread.isRequestToTerminate()) {
		CurrentThread.sleep(1);
	};
	return true;
});
releasedThread = null;

// --- Start from string, code ending with a comment without new line

var stringThread = Thread.newThreadFromString("return 7; // comment");
check("start from string with comment", Thread.isThread(stringThread), true);
if (Thread.isThread(stringThread)) {
	stringThread.join();
	check("start from string returned value", stringThread.getReturnedValue(), 7);
};

// --- Atomic used in a thread that did not require the extension, no crash

var atomicNoExtension = new Atomic();
atomicNoExtension.set(5);
var noExtensionThread = Thread.newThread(function(atomic) {
	return atomic.get();
}, null, [atomicNoExtension]);
noExtensionThread.join();
var noExtensionValue = noExtensionThread.getReturnedValue();
check("atomic in thread without extension", (noExtensionValue === undefined) || (noExtensionValue === 5), true);

if (failed) {
	throw "test " + testName + " failed";
};
Console.writeLn("-> test " + testName + " ok");
