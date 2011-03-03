--TEST--
RRDGraph saveVerbose full export test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
$outputPngFile = "-";
$graphObj = new RRDGraph($outputPngFile);
$graphObj->setOptions(array(
	"--start" => "920804400",
	"--end" => 920808000,
	"--vertical-label" => "m/s",
	"DEF:myspeed=$rrdFile:speed:AVERAGE",
	"CDEF:realspeed=myspeed,1000,*",
	"LINE2:realspeed#FF0000"
));
$output = $graphObj->saveVerbose();
$imgData = $output["image"]; unset($output["image"]);
//output without img data
var_dump($output);
//detection of correct PNG header
var_dump(substr($imgData, 0, 8) == "\x89PNG\x0d\x0a\x1a\x0a");
?>
--EXPECTF--
array(10) {
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
  int(148)
  ["graph_start"]=>
  int(920804400)
  ["graph_end"]=>
  int(920808000)
  ["value_min"]=>
  float(0)
  ["value_max"]=>
  float(40)
}
bool(true)
