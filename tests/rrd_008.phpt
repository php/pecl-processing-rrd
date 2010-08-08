--TEST--
rrd_graph test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/graphTest.rrd";
$command = "rrdtool create $rrdFile "
 . "--start 920804400 "
 . "DS:speed:COUNTER:600:U:U "
 . "RRA:AVERAGE:0.5:1:24 "
 . "RRA:AVERAGE:0.5:6:10";
echo "creting $rrdFile via exec\n";
exec($command);

$updateCommand = array(
 "rrdtool update $rrdFile 920804700:12345 920805000:12357 920805300:12363",
 "rrdtool update $rrdFile 920805600:12363 920805900:12363 920806200:12373",
 "rrdtool update $rrdFile 920806500:12383 920806800:12393 920807100:12399",
 "rrdtool update $rrdFile 920807400:12405 920807700:12411 920808000:12415",
 "rrdtool update $rrdFile 920808300:12420 920808600:12422 920808900:12423"
);
echo "updating $rrdFile via exec\n";
foreach($updateCommand as $command) {
	exec($command);
}

$origPngFile = dirname(__FILE__) . "/speed-orig.png";
$command = "rrdtool graph $origPngFile "
 . "--start 920804400 --end 920808000 "
 . "--vertical-label m/s "
 . "DEF:myspeed=$rrdFile:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting $origPngFile via exec\n";
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
creting %s via exec
updating %s via exec
exporting %s via exec
array(3) {
  ["xsize"]=>
  int(497)
  ["ysize"]=>
  int(149)
  ["calcpr"]=>
  NULL
}
