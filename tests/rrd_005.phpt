--TEST--
RRDUpdater default timestamp test
--SKIPIF--
<?php
include('skipif.inc');
include('rrdtool-bin.inc');
include('data/definition.inc');
if (!file_exists($data_emptyDb)) {
	die("skip $data_emptyDb doesnt' exist");
}
?>
--FILE--
<?php
include('rrdtool-bin.inc');
include('data/definition.inc');
$rrdFile = dirname(__FILE__) . "/rrd_updater_default_val.rrd";
copy($data_emptyDb, $rrdFile);

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

//mostly for "visual test"
$command = "$rrdtool_bin graph "
 . dirname(__FILE__) . "/rrd_updater_default_val.png "
 . "--start -5s --end now "
 . "--vertical-label m/s "
 . "DEF:myspeed=$rrdFile:speed:AVERAGE "
 . "CDEF:realspeed=myspeed,1000,* "
 . "LINE2:realspeed#FF0000";

echo "exporting graph via exec\n";
exec($command);
?>
--EXPECTF--
exporting %s via exec
