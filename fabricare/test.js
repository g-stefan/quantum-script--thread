// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

Script.include("fabricare://test.js");

// ---

for(var k=2;k<=2;++k){
	exitIf(Shell.execute("quantum-script  --execution-time test/test.0"+k+".js"));
};
