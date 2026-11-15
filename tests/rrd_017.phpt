--TEST--
rrd_xport test
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
$result = rrd_xport(array(
	"--start=920804400",
	"--end=920808000",
	"DEF:myspeed=$data_updatedDb:speed:AVERAGE",
	"CDEF:realspeed=myspeed,1000,*",
	"XPORT:myspeed:myspeed",
	"XPORT:realspeed:realspeed"
));

/* librrd decides for itself whether the closing sample falls inside the
   window, so assert the shape and the samples rather than the row count */
var_dump(array_keys($result));
var_dump($result["start"], $result["step"]);
var_dump(array_column($result["data"], "legend"));

$speed = $result["data"][0]["data"];
$real  = $result["data"][1]["data"];

var_dump(array_slice(array_keys($speed), 0, 3));
var_dump(is_nan($speed[920804700]));
printf("%.4f %.4f %.4f\n", $speed[920805000], $speed[920805300], $speed[920806200]);
printf("%.4f %.4f %.4f\n", $real[920805000], $real[920805300], $real[920806200]);

/* every key is a step-aligned timestamp inside the window */
$aligned = true;
foreach (array_keys($speed) as $ts) {
	if (($ts - $result["start"]) % $result["step"] !== 0) { $aligned = false; }
	if ($ts < $result["start"] || $ts > $result["end"]) { $aligned = false; }
}
var_dump($aligned, count($speed) === count($real));
?>
--EXPECT--
array(4) {
  [0]=>
  string(5) "start"
  [1]=>
  string(3) "end"
  [2]=>
  string(4) "step"
  [3]=>
  string(4) "data"
}
int(920804700)
int(300)
array(2) {
  [0]=>
  string(7) "myspeed"
  [1]=>
  string(9) "realspeed"
}
array(3) {
  [0]=>
  int(920804700)
  [1]=>
  int(920805000)
  [2]=>
  int(920805300)
}
bool(true)
0.0400 0.0200 0.0333
40.0000 20.0000 33.3333
bool(true)
bool(true)
