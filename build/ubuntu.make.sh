#!/bin/sh
# Public domain
# http://unlicense.org/
# Created by Grigore Stefan <g_stefan@yahoo.com>

ACTION=$1
if [ "$ACTION" = "" ]; then
	ACTION=make
fi

echo "-> $ACTION quantum-script-extension-thread"

cmdX(){
	if ! "$@" ; then
		echo "Error: $ACTION"
		exit 1
	fi
}

cmdX file-to-cs --touch=source/quantum-script-extension-thread.cpp --file-in=source/quantum-script-extension-thread.js --file-out=source/quantum-script-extension-thread.src --is-string --name=extensionThreadSource
cmdX xyo-cc --mode=$ACTION @build/source/quantum-script-extension-thread.static.compile
cmdX xyo-cc --mode=$ACTION @build/source/quantum-script-extension-thread.dynamic.compile
