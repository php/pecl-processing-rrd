--TEST--
rrd_fetch test
--SKIPIF--
<?php
include('skipif.inc');
include('data/definition.inc');
if (!file_exists($data_moreDSDb)) {
	die("skip $data_moreDSDb doesnt' exist");
}
?>
--FILE--
<?php
include('data/definition.inc');
var_dump(rrd_fetch($data_moreDSDb, array(
	"--start", "920804400",
	"--end", "920808000",
	"AVERAGE"
)));
?>
--EXPECTF--
array(4) {
  ["start"]=>
  int(920804400)
  ["end"]=>
  int(920808300)
  ["step"]=>
  int(300)
  ["data"]=>
  array(2) {
    ["speed1"]=>
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
      float(0.03333333333%s)
      [920806500]=>
      float(0.03333333333%s)
      [920806800]=>
      float(0.03333333333%s)
      [920807100]=>
      float(0.02)
      [920807400]=>
      float(0.02)
      [920807700]=>
      float(0.02)
      [920808000]=>
      float(0.01333333333%s)
      [920808300]=>
      float(0.01666666666%s)
    }
    ["speed2"]=>
    array(13) {
      [920804700]=>
      float(NAN)
      [920805000]=>
      float(0.05666666666%s)
      [920805300]=>
      float(0.02)
      [920805600]=>
      float(0.00333333333%s)
      [920805900]=>
      float(0)
      [920806200]=>
      float(0.03)
      [920806500]=>
      float(0)
      [920806800]=>
      float(0.06666666666%s)
      [920807100]=>
      float(0.02)
      [920807400]=>
      float(0.02)
      [920807700]=>
      float(0.02)
      [920808000]=>
      float(0.01333333333%s)
      [920808300]=>
      float(0.01666666666%s)
    }
  }
}
