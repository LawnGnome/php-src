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
header("X-Foo6: quux\r quux");
header("X-Foo7: quux\n quux");
header("X-Foo8: quux\r\n quux");
header("X-Foo9: quux\r\tquux");
header("X-Foo10: quux\n\tquux");
header("X-Foo11: quux\r\n\tquux");
?>
--EXPECTF--
Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain NUL bytes in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d

Warning: Header may not contain multiple lines in %s on line %d
--EXPECTHEADERS--
X-Foo1: bar
