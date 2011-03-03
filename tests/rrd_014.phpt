--TEST--
rrd_lastupdate test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
var_dump(rrd_lastupdate($rrdFile));
?>
--EXPECTF--
array(4) {
  ["last_update"]=>
  int(920808900)
  ["ds_cnt"]=>
  int(1)
  ["ds_navm"]=>
  array(1) {
    [0]=>
    string(5) "speed"
  }
  ["data"]=>
  array(1) {
    [920808900]=>
    string(5) "12423"
  }
}