--TEST--
rrd_lastupdate test
--SKIPIF--
<?php
include('skipif.inc');
include('data/definition.inc');
foreach(array($data_updatedDb, $data_moreDSDb) as $file) {
	if (!file_exists($file)) {
		die("skip $file doesnt' exist");
	}
}
?>
--FILE--
<?php
include('data/definition.inc');
var_dump(rrd_lastupdate($data_updatedDb));
var_dump(rrd_lastupdate($data_moreDSDb));
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
    [0]=>
    string(5) "12423"
  }
}
array(4) {
  ["last_update"]=>
  int(920808900)
  ["ds_cnt"]=>
  int(2)
  ["ds_navm"]=>
  array(2) {
    [0]=>
    string(6) "speed1"
    [1]=>
    string(6) "speed2"
  }
  ["data"]=>
  array(2) {
    [0]=>
    string(5) "12423"
    [1]=>
    string(5) "11423"
  }
}