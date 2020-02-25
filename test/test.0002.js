// Public domain
// http://unlicense.org/

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
		return "> test 0002 ok <";
	};

	return "> test 0002 fail <";

}, {a:1, b:2, c:3}, [101, 102, 103]);

thread1.join();
Console.writeLn(thread1.getReturnedValue());

