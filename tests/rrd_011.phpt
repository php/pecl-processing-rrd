--TEST--
rrd_first test
--SKIPIF--
<?php
include('skipif.inc');
include('data/definition.inc');
if (!file_exists($data_updatedDb)) {
	die("skip $data_updatedDb doesnt' exist");
}
?>
--FILE--
<?php
include('data/definition.inc');
var_dump(rrd_first($data_updatedDb));
var_dump(rrd_first($data_updatedDb, 1));
var_dump(rrd_first($data_updatedDb, -1));
var_dump(rrd_first($data_updatedDb, 2));
?>
--EXPECTF--
int(920802000)
int(920791800)
bool(false)
bool(false)
