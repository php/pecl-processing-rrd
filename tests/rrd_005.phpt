--TEST--
RRDUpdater default timestamp test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/updateTestDefaultVal.rrd";
$command = "rrdtool create $rrdFile "
 . "--start now "
 . "--step 1 "
 . "DS:speed:COUNTER:600:U:U "
 . "RRA:AVERAGE:0.5:1:24 "
 . "RRA:AVERAGE:0.5:6:10";
echo "creating rrdfile for update test via exec\n";
exec($command);

$updator = new RRDUpdater($rrdFile);
$updator->update(array("speed" => "12345"));
sleep(1);
$updator->update(array("speed" => "12357"));
sleep(1);
$updator->update(array("speed" => "12363"));
sleep(1);
$updator->update(array("speed" => "12363"));
sleep(1);
$updator->update(array("speed" => "12363"));
sleep(1);

$command = "rrdtool graph ".dirname(__FILE__)."/updateTestDefaultVal.png "
 . "--start -5s --end now "
 . "--vertical-label m/s "
 . "DEF:myspeed=$rrdFile:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting updateTestDefaultVal.png via exec\n";
exec($command);
?>
--EXPECTF--
creating rrdfile for update test via exec
exporting updateTestDefaultVal.png via exec
