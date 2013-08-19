--TEST--
Throwing an exception with null bytes in the message
--FILE--
<?php 
throw new Exception("foo\0bar");
?>
--EXPECTF--

Fatal error: Uncaught exception 'Exception' with message 'foo bar' in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
