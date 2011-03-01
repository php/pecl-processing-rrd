--TEST--
RRDUpdater test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/rrd_updater_test.rrd";
$command = "rrdtool create $rrdFile "
 . "--start 920804400 "
 . "DS:speed:COUNTER:600:U:U "
 . "RRA:AVERAGE:0.5:1:24 "
 . "RRA:AVERAGE:0.5:6:10";
echo "creating rrdfile for RRDUpdater test via exec\n";
exec($command);

$updator = new RRDUpdater($rrdFile);
$updator->update(array("speed" => "12345"), "920804700");
$updator->update(array("speed" => "12357"), "920805000");
$updator->update(array("speed" => "12363"), "920805300");
$updator->update(array("speed" => "12363"), "920805600");
$updator->update(array("speed" => "12363"), "920805900");
$updator->update(array("speed" => "12373"), "920806200");
$updator->update(array("speed" => "12383"), "920806500");
$updator->update(array("speed" => "12393"), "920806800");
$updator->update(array("speed" => "12399"), "920807100");
$updator->update(array("speed" => "12405"), "920807400");
$updator->update(array("speed" => "12411"), "920807700");
$updator->update(array("speed" => "12415"), "920808000");
$updator->update(array("speed" => "12420"), "920808300");
$updator->update(array("speed" => "12422"), "920808600");
$updator->update(array("speed" => "12423"), "920808900");

$command = "rrdtool graph " . dirname(__FILE__) . "/rrd_updater_test.png "
 . "--start 920804400 --end 920808000 "
 . "--vertical-label m/s "
 . "DEF:myspeed=$rrdFile:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting rrd_updater_test.png via exec\n";
exec($command);

$command = "rrdtool fetch $rrdFile AVERAGE --start 920804400 --end 920809200";
$output = array();
exec($command, $output);
$originalFetch = file(dirname(__FILE__)."/testData/rrd_updater_fetch.txt", FILE_IGNORE_NEW_LINES);
echo "comparing original and current fetch\n";
var_dump(array_diff($output, $originalFetch));
?>
--EXPECTF--
creating rrdfile for RRDUpdater test via exec
exporting rrd_updater_test.png via exec
comparing original and current fetch
array(0) {
}