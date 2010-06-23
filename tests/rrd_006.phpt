--TEST--
rrd_update test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/rrd_update_1.rrd";

$command = "rrdtool create $rrdFile "
 . "--start 920804400 "
 . "DS:speed:COUNTER:600:U:U "
 . "RRA:AVERAGE:0.5:1:24 "
 . "RRA:AVERAGE:0.5:6:10";
echo "creating rrdfile for update test via exec\n";
exec($command);

$retval = rrd_update($rrdFile, array(
    "920804700:12345",
    "920805000:12357",
    "920805300:12363",
    "920805600:12363",
    "920805900:12363",
    "920806200:12373",
    "920806500:12383",
    "920806800:12393",
    "920807100:12399",
    "920807400:12405",
    "920807700:12411",
    "920808000:12415",
    "920808300:12420",
    "920808600:12422",
    "920808900:12423"
));

$command = "rrdtool graph ".dirname(__FILE__)."/rrd_update_1.png "
 . "--start 920804400 --end 920808000 "
 . "--vertical-label m/s "
 . "DEF:myspeed=$rrdFile:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting rrd_update_1.png via exec\n";
exec($command);
var_dump($retval);
?>
--EXPECTF--
creating rrdfile for update test via exec
exporting rrd_update_1.png via exec
bool(true)
