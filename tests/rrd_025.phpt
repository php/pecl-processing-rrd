--TEST--
RRDCreator::save() with no data source and no archive
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$creator = new RRDCreator(dirname(__FILE__) . "/empty-create.rrd");
var_dump($creator->save());
?>
--EXPECTF--
Warning: cannot allocate arguments options in %s on line %d
bool(false)
