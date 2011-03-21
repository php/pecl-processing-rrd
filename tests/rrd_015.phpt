--TEST--
rrd_restore test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$xmlFile = dirname(__FILE__) . "/data/speed-dump.xml";
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

