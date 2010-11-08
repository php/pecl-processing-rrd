--TEST--
RRDGraph saveVerbose test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
$outputPngFile = dirname(__FILE__) . "/rrdGraph-saveVerbose.png";
$graphObj = new RRDGraph($outputPngFile);
$graphObj->setOptions(array(
	"--start" => "920804400",
	"--end" => 920808000,
	"--vertical-label" => "m/s",
	"DEF:myspeed=$rrdFile:speed:AVERAGE",
	"CDEF:realspeed=myspeed,1000,*",
	"LINE2:realspeed#FF0000"
));
var_dump($graphObj->saveVerbose());
?>
--EXPECTF--
array(8) {
  ["graph_left"]=>
  int(67)
  ["graph_top"]=>
  int(22)
  ["graph_width"]=>
  int(400)
  ["graph_height"]=>
  int(100)
  ["image_width"]=>
  int(497)
  ["image_height"]=>
  int(149)
  ["value_min"]=>
  float(0)
  ["value_max"]=>
  float(40)
}
