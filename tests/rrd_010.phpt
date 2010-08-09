--TEST--
rrd_fetch test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/rrd_fetch_test.rrd";
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

var_dump(rrd_fetch($rrdFile, array(
	"--start", "920804400",
	"--end", "920808000",
	"AVERAGE"
)));
?>
--EXPECTF--
creting %s via exec
updating %s via exec
array(6) {
  ["start"]=>
  int(920804400)
  ["end"]=>
  int(920808300)
  ["step"]=>
  int(300)
  ["ds_cnt"]=>
  int(1)
  ["ds_navm"]=>
  array(1) {
    [0]=>
    string(5) "speed"
  }
  ["data"]=>
  array(13) {
    [920804700]=>
    float(NAN)
    [920805000]=>
    float(0.04)
    [920805300]=>
    float(0.02)
    [920805600]=>
    float(0)
    [920805900]=>
    float(0)
    [920806200]=>
    float(0.033333333333333)
    [920806500]=>
    float(0.033333333333333)
    [920806800]=>
    float(0.033333333333333)
    [920807100]=>
    float(0.02)
    [920807400]=>
    float(0.02)
    [920807700]=>
    float(0.02)
    [920808000]=>
    float(0.013333333333333)
    [920808300]=>
    float(0.016666666666667)
  }
}