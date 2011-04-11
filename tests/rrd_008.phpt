--TEST--
rrd_graph test
--SKIPIF--
include('skipif.inc');
include('rrdtool-bin.inc');
include('data/definition.inc');
if (!file_exists($data_updatedDb)) {
	die("skip $data_updatedDb doesnt' exist");
}
?>
--FILE--
<?php
include('rrdtool-bin.inc');
include('data/definition.inc');

$command = "rrdtool graph " . dirname(__FILE__) . "/rrd_graph_speed_exec.png "
 . "--start 920804400 --end 920808000 "
 . "--vertical-label m/s "
 . "DEF:myspeed=$data_updatedDb:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting image via exec\n";
exec($command);

$outputPngFile = dirname(__FILE__) . "/rrd_graph_speed.png";
var_dump(rrd_graph($outputPngFile, array(
	"--start", "920804400",
	"--end", "920808000",
	"--vertical-label", "m/s",
	"DEF:myspeed=$data_updatedDb:speed:AVERAGE",
	"CDEF:realspeed=myspeed,1000,*",
	"LINE2:realspeed#FF0000"
)));
?>
--EXPECTF--
exporting image via exec
array(3) {
  ["xsize"]=>
  int(497)
  ["ysize"]=>
  int(148)
  ["calcpr"]=>
  NULL
}
