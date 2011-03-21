--TEST--
rrd_last test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/data/speed.rrd";
var_dump(rrd_last($rrdFile));
?>
--EXPECTF--
int(920808900)