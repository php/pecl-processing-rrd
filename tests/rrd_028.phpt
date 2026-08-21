--TEST--
options arrays are not modified by value conversion
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$options = array("--step", 300, "DS:speed:COUNTER:600:U:U", "RRA:AVERAGE:0.5:1:24");
$copy = $options;
rrd_create(dirname(__FILE__) . "/no-mutate.rrd", $options);
var_dump($options[1], $copy[1]);
@unlink(dirname(__FILE__) . "/no-mutate.rrd");

$values = array("speed" => 1.5);
$values_copy = $values;
$updater = new RRDUpdater(dirname(__FILE__) . "/no-mutate.rrd");
try { $updater->update($values); } catch (Exception $e) {}
var_dump($values["speed"], $values_copy["speed"]);
?>
--EXPECT--
int(300)
int(300)
float(1.5)
float(1.5)
