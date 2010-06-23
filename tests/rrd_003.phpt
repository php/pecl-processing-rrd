--TEST--
RRDCreator test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$rrdFile = dirname(__FILE__) . "/createOrig.rrd";
$command = "rrdtool create $rrdFile "
 . "--start 920804400 "
 . "DS:speed:COUNTER:600:U:U "
 . "RRA:AVERAGE:0.5:1:24 "
 . "RRA:AVERAGE:0.5:6:10";
echo "creating original rrd for RRDCreator via exec\n";
exec($command);

$creator = new RRDCreator(dirname(__FILE__) . "/createTest.rrd","now -10d", 500);
$creator->addDataSource("speed:COUNTER:600:U:U");
$creator->addArchive("AVERAGE:0.5:1:24");
$creator->addArchive("AVERAGE:0.5:6:10");
$creator->save();
?>
--EXPECTF--
creating original rrd for RRDCreator via exec
