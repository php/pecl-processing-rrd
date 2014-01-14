--TEST--
rrd_version test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$versionArr = explode(".", $ver = rrd_version());
var_dump($ver);
var_dump($versionArr[0] == 1, $versionArr[1] >= 2);
?>
--EXPECTF--
string(%i) "%s"
bool(true)
bool(true)
