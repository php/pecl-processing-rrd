--TEST--
RRDGraph test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
$outputPngFile = dirname(__FILE__) . "/rrdGraph-speed.png";
$graphObj = new RRDGraph($outputPngFile);
$graphObj->setOptions(array(
	"--start" => "920804400",
	"--end" => 920808000,
	"--vertical-label" => "m/s",
	"DEF:myspeed=$rrdFile:speed:AVERAGE",
	"CDEF:realspeed=myspeed,1000,*",
	"LINE2:realspeed#FF0000"
));
var_dump($graphObj->save());
var_dump(file_exists($outputPngFile));
?>
--EXPECTF--
array(3) {
  ["xsize"]=>
  int(497)
  ["ysize"]=>
  int(148)
  ["calcpr"]=>
  NULL
}
bool(true)