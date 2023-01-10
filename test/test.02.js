// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

Script.requireExtension("Console");
Script.requireExtension("Thread");

var thread1=Thread.newThread(function() {

	if(
		(.a==1)&&
		(.b==2)&&
		(.c==3)&&
		(arguments[0]==101)&&
		(arguments[1]==102)&&
		(arguments[2]==103)
	) {
		return "> test 02 ok <";
	};

	return "> test 02 fail <";

}, {a:1, b:2, c:3}, [101, 102, 103]);

thread1.join();
Console.writeLn(thread1.getReturnedValue());

