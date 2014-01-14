--TEST--
rrdc_disconnect test
--SKIPIF--
<?php
include('skipif.inc');
if (!function_exists("rrdc_disconnect")) {
	die("skip rrdc_disconnect only in rrdtool >= 1.4");
}

?>
--FILE--
<?php
var_dump(rrdc_disconnect());
?>
--EXPECTF--
NULL
