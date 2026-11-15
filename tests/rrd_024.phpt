--TEST--
RRDUpdater::update() rejects a values array with integer keys
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$updater = new RRDUpdater(dirname(__FILE__) . "/int-keys.rrd");
try {
    $updater->update(array(1.0));
} catch (Exception $e) {
    echo get_class($e), ": ", $e->getMessage(), "\n";
}
?>
--EXPECT--
Exception: values array must be keyed by data source name
