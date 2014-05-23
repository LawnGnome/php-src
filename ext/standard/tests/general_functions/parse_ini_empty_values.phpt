--TEST--
parse_ini_file(): empty values
--FILE--
<?php

$input = <<<EOD
; Comment starts with semi-colon(;)
; Section starts with [<section name>]

; start of ini file

[section]
foo=bar
bar=
quux
zzzz
EOD;

var_dump(parse_ini_string($input, false, INI_SCANNER_NORMAL));
var_dump(parse_ini_string($input, false, INI_SCANNER_RAW));
var_dump(parse_ini_string($input, true, INI_SCANNER_NORMAL));
var_dump(parse_ini_string($input, true, INI_SCANNER_RAW));

?>
--EXPECT--
array(4) {
  ["foo"]=>
  string(3) "bar"
  ["bar"]=>
  string(0) ""
  ["quux"] =>
  NULL
  ["zzzz"] =>
  NULL
}
array(4) {
  ["foo"]=>
  string(3) "bar"
  ["bar"]=>
  string(0) ""
  ["quux"] =>
  NULL
  ["zzzz"] =>
  NULL
}
array(1) {
  ["section"]=>
  array(4) {
    ["foo"]=>
    string(3) "bar"
    ["bar"]=>
    string(0) ""
    ["quux"] =>
    NULL
    ["zzzz"] =>
    NULL
  }
}
array(1) {
  ["section"]=>
  array(4) {
    ["foo"]=>
    string(3) "bar"
    ["bar"]=>
    string(0) ""
    ["quux"] =>
    NULL
    ["zzzz"] =>
    NULL
  }
}
