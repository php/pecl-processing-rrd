--TEST--
rrd_tune test
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
$destFile = dirname(__FILE__) . "/tune-test.rrd";
copy($data_updatedDb, $destFile);
var_dump(rrd_tune($destFile, array("--data-source-rename=speed:new-speed")));
var_dump(rrd_tune($destFile, array("--lala")));
//maybe bug in rrd_tune?
var_dump(rrd_tune($destFile, array("true")));
?>
--EXPECTF--
bool(true)
bool(false)
bool(true)
%s