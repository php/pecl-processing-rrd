--TEST--
rrd_info test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
var_dump(rrd_info($rrdFile));
?>
--EXPECTF--
array(25) {
  ["filename"]=>
  string(61) %s
  ["rrd_version"]=>
  string(4) "0003"
  ["step"]=>
  int(300)
  ["last_update"]=>
  int(920808900)
  ["ds[speed].type"]=>
  string(7) "COUNTER"
  ["ds[speed].minimal_heartbeat"]=>
  int(600)
  ["ds[speed].min"]=>
  float(NAN)
  ["ds[speed].max"]=>
  float(NAN)
  ["ds[speed].last_ds"]=>
  string(5) "12423"
  ["ds[speed].value"]=>
  float(0)
  ["ds[speed].unknown_sec"]=>
  int(0)
  ["rra[0].cf"]=>
  string(7) "AVERAGE"
  ["rra[0].rows"]=>
  int(24)
  ["rra[0].cur_row"]=>
  int(9)
  ["rra[0].pdp_per_row"]=>
  int(1)
  ["rra[0].xff"]=>
  float(0.5)
  ["rra[0].cdp_prep[0].value"]=>
  float(NAN)
  ["rra[0].cdp_prep[0].unknown_datapoints"]=>
  int(0)
  ["rra[1].cf"]=>
  string(7) "AVERAGE"
  ["rra[1].rows"]=>
  int(10)
  ["rra[1].cur_row"]=>
  int(0)
  ["rra[1].pdp_per_row"]=>
  int(6)
  ["rra[1].xff"]=>
  float(0.5)
  ["rra[1].cdp_prep[0].value"]=>
  float(0.026666666666667)
  ["rra[1].cdp_prep[0].unknown_datapoints"]=>
  int(0)
}