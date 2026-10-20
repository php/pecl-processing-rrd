--TEST--
calling __construct a second time resets the object
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$first  = dirname(__FILE__) . "/reconstruct-first.rrd";
$second = dirname(__FILE__) . "/reconstruct-second.rrd";
@unlink($first); @unlink($second);

$creator = new RRDCreator($first, "920804400", 300);
$creator->addDataSource("speed:COUNTER:600:U:U");
$creator->addArchive("AVERAGE:0.5:1:24");

/* the second construction must not inherit the first one's sources */
$creator->__construct($second, "920804400", 600);
try {
    var_dump($creator->save());
} catch (Exception $e) {
    echo get_class($e), ": ", $e->getMessage(), "\n";
}
var_dump(file_exists($first), file_exists($second));

@unlink($first); @unlink($second);
?>
--EXPECT--
Exception: you must define at least one Round Robin Archive
bool(false)
bool(false)
