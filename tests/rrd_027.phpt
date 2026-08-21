--TEST--
rrd_fetch and rrd_xport with timestamps wider than 10 digits
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

/* keys must be the full timestamps, not the first 10 digits of them */
$xport = rrd_xport(array(
	"--start=1700000000100",
	"--end=1700000000700",
	"--step=300",
	"DEF:myspeed=$data_updatedDb:speed:AVERAGE",
	"XPORT:myspeed:myspeed",
));
var_dump(array_keys($xport["data"][0]["data"]));

/* an --end past 2^32 used to wrap rrd_fetch's 32-bit loop counter */
$fetch = rrd_fetch($data_updatedDb, array(
	"AVERAGE",
	"--start", "4294967000",
	"--end", "4294968200",
	"--resolution", "300",
));
var_dump(count($fetch["data"]["speed"]));
?>
--EXPECT--
array(2) {
  [0]=>
  int(1700000000400)
  [1]=>
  int(1700000000700)
}
int(5)
