--TEST--
rrd_lastupdate test
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
var_dump(rrd_lastupdate($data_updatedDb));
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