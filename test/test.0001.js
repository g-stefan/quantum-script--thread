// Public domain
// http://unlicense.org/

Script.requireExtension("Console");
Script.requireExtension("Thread");

var thread1=Thread.newThreadFromFile("test/test.0001.thread.js");
thread1.join();
Console.writeLn(thread1.getReturnedValue());
