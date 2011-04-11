--TEST--
rrd_restore test
--SKIPIF--
<?php include('skipif.inc');
include('data/definition.inc');
if (!file_exists($data_xmdDbdump)) {
	die("skip $data_xmdDbdump doesn't exist");
}
?>
--FILE--
<?php
include('data/definition.inc');
$xmlFile = $data_xmdDbdump;
$rrdFile = dirname(__FILE__) . "/restore-result.rrd";
//if rrd file isn't deleted, rrd_restore without options fails
@unlink($rrdFile);
var_dump(rrd_restore($xmlFile, $rrdFile));
var_dump(file_exists($rrdFile) && filesize($rrdFile));

//test "-f" options - force to overwrite
var_dump(rrd_restore($xmlFile, $rrdFile, array("-f")));
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

