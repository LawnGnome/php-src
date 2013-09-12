--TEST--
Catchable fatal error [1]
--FILE--
<?php
	class Foo {
	}

	function blah (Foo $a)
	{
	}

	function error()
	{
		$a = func_get_args();
		var_dump($a);
	}

	blah (new StdClass);
	echo "ALIVE!\n";
?>
--EXPECTF--
Catchable fatal error: Argument 1 passed to blah() must be an instance of the class Foo, instance of stdClass given, called in %scatchable_error_001.php on line 15 and defined in %scatchable_error_001.php on line 5
