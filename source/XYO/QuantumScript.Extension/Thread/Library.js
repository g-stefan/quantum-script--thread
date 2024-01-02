// Quantum Script Extension Thread
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

Thread.newThread = function(function_, this_, arguments_) {
	var thread = new Thread();
	if(thread.start(function_, this_, arguments_)) {
		return thread;
	};
	return null;
};

Thread.newThreadFromFile = function(function_, this_, arguments_) {
	var thread = new Thread();
	if(thread.startFromFile(function_, this_, arguments_)) {
		return thread;
	};
	return null;
};

Thread.newThreadFromString = function(function_, this_, arguments_) {
	var thread = new Thread();
	if(thread.startFromString(function_, this_, arguments_)) {
		return thread;
	};
	return null;
};

