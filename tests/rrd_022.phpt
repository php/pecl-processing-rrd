--TEST--
rrdc_disconnect test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
var_dump(rrdc_disconnect());
?>
--EXPECTF--
NULL
