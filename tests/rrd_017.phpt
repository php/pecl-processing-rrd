--TEST--
rrd_xport test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/testData/speed.rrd";
var_dump(rrd_xport(array(
	"--start=920804400",
	"--end=920808000",
	"DEF:myspeed=$rrdFile:speed:AVERAGE",
	"CDEF:realspeed=myspeed,1000,*",
	"XPORT:myspeed:myspeed",
	"XPORT:realspeed:realspeed"
)));
?>
--EXPECTF--
array(4) {
  ["start"]=>
  int(920804700)
  ["end"]=>
  int(920808300)
  ["step"]=>
  int(300)
  ["data"]=>
  array(2) {
    [0]=>
    array(2) {
      ["legend"]=>
      string(7) "myspeed"
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
    [1]=>
    array(2) {
      ["legend"]=>
      string(9) "realspeed"
      ["data"]=>
      array(13) {
        [920804700]=>
        float(NAN)
        [920805000]=>
        float(40)
        [920805300]=>
        float(20)
        [920805600]=>
        float(0)
        [920805900]=>
        float(0)
        [920806200]=>
        float(33.333333333333)
        [920806500]=>
        float(33.333333333333)
        [920806800]=>
        float(33.333333333333)
        [920807100]=>
        float(20)
        [920807400]=>
        float(20)
        [920807700]=>
        float(20)
        [920808000]=>
        float(13.333333333333)
        [920808300]=>
        float(16.666666666667)
      }
    }
  }
}
