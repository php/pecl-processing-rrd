--TEST--
rrd_graph test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
$command = "rrdtool graph " . dirname(__FILE__) . "/rrd_graph_speed_exec.png "
 . "--start 920804400 --end 920808000 "
 . "--vertical-label m/s "
 . "DEF:myspeed=$rrdFile:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting image via exec\n";
exec($command);

$outputPngFile = dirname(__FILE__) . "/rrd_graph_speed.png";
var_dump(rrd_graph($outputPngFile, array(
	"--start", "920804400",
	"--end", "920808000",
	"--vertical-label", "m/s",
	"DEF:myspeed=$rrdFile:speed:AVERAGE",
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
