//
// Quantum Script Extension Thread
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

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

