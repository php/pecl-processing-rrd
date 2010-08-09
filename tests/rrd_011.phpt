--TEST--
rrd_first test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
var_dump(rrd_first($rrdFile));
var_dump(rrd_first($rrdFile, 1));
var_dump(rrd_first($rrdFile, -1));
var_dump(rrd_first($rrdFile, 2));
?>
--EXPECTF--
int(920802000)
int(920791800)
bool(false)
bool(false)
