// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

Script.requireExtension("Console");
Script.requireExtension("Thread");

var thread1=Thread.newThreadFromFile("test/test.01.thread.js");
thread1.join();
Console.writeLn(thread1.getReturnedValue());
