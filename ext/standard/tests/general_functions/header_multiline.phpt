--TEST--
header() cannot generate multiline headers
--FILE--
<?php
ob_start();
header("X-Foo1: bar");
header("X-Foo2: quux\nquux");
header("X-Foo3: quux\rquux");
header("X-Foo4: quux\r\nquux");
header("X-Foo5: quux\0quux");
?>
--EXPECTF--
Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain NUL bytes in %s on line %d
--EXPECTHEADERS--
X-Foo1: bar
