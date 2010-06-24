--TEST--
rrd_create test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/rrd_create_orig.rrd";
$params = array(
	"--start",920804400,
	"DS:speed:COUNTER:600:U:U",
	"RRA:AVERAGE:0.5:1:24",
	"RRA:AVERAGE:0.5:6:10"
);

$command = "rrdtool create $rrdFile " . implode(" ", $params);
echo "creating original rrd for rrd_create test via exec\n";
exec($command);

$retval = rrd_create(dirname(__FILE__) . "/rrd_create_test.rrd", $params);
var_dump($retval);
?>
--EXPECTF--
creating original rrd for rrd_create test via exec
bool(true)
