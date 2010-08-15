--TEST--
rrd_tune test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$destFile = dirname(__FILE__) . "/tune-test.rrd";
copy(dirname(__FILE__) . "/testData/speed.rrd", $destFile);
var_dump(rrd_tune($destFile, array("--data-source-rename=speed:new-speed")));
var_dump(rrd_tune($destFile, array("--lala")));
//maybe bug in rrd_tune?
var_dump(rrd_tune($destFile, array("true")));
?>
--EXPECTF--
bool(true)
bool(false)
bool(true)
DS[new-speed] typ: COUNTER      hbt: 600        min: nan        max: nan
