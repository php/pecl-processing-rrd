--TEST--
rrd module presence test
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
echo "rrd extension loaded";
?>
--EXPECTF--
rrd extension loaded
