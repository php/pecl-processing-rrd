--TEST--
rrd_info test
--SKIPIF--
<?php
include('skipif.inc');
include('data/definition.inc');
if (!file_exists($data_updatedDb)) {
	die("skip $data_updatedDb doesn't exist");
}
?>
--FILE--
<?php
include('data/definition.inc');
putenv('LANG=C');
setlocale(LC_ALL, 'C');
var_dump($info = rrd_info($data_updatedDb));
var_dump($info["filename"] == $data_updatedDb);
?>
--EXPECTF--
array(27) {
  ["filename"]=>
  string(%d) %s
  ["rrd_version"]=>
  string(4) %s
  ["step"]=>
  int(300)
  ["last_update"]=>
  int(920808900)
  ["header_size"]=>
  int(%d)
  ["ds[speed].index"]=>
  int(0)
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
  int(%d)
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
  int(%d)
  ["rra[1].pdp_per_row"]=>
  int(6)
  ["rra[1].xff"]=>
  float(0.5)
  ["rra[1].cdp_prep[0].value"]=>
  float(0.02666666666%s)
  ["rra[1].cdp_prep[0].unknown_datapoints"]=>
  int(0)
}
bool(true)
