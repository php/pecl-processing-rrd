--TEST--
RRDGraph test
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
$outputPngFile = dirname(__FILE__) . "/rrdGraph-speed.png";
$graphObj = new RRDGraph($outputPngFile);
$graphObj->setOptions(array(
	"--start" => "920804400",
	"--end" => 920808000,
	"--vertical-label" => "m/s",
	"DEF:myspeed=$data_updatedDb:speed:AVERAGE",
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
  int(%d)
  ["calcpr"]=>
  NULL
}
bool(true)
