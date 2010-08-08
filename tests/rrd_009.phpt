--TEST--
rrd_error test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
rrd_create(dirname(__FILE__) . "/rrd_create_test.rrd", array("badParam"));
var_dump(rrd_error());
var_dump(rrd_error());
?>
--EXPECTF--
string(31) "can't parse argument 'badParam'"
NULL