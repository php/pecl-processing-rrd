--TEST--
rrd_last test
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
var_dump(rrd_last($data_updatedDb));
?>
--EXPECTF--
int(920808900)