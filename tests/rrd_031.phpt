--TEST--
an unconvertible option value aborts before anything is written
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
class NoString {}
class Throws { public function __toString(): string { throw new RuntimeException("nope"); } }

$file = dirname(__FILE__) . "/unconvertible.rrd";
@unlink($file);

foreach (array("NoString", "Throws") as $class) {
    try {
        @rrd_create($file, array("--step", "300", new $class,
                                 "DS:speed:COUNTER:600:U:U", "RRA:AVERAGE:0.5:1:24"));
        echo $class, ": no throw\n";
    } catch (Throwable $e) {
        echo $class, ": ", get_class($e), "\n";
    }
    var_dump(file_exists($file));
}

$updater = new RRDUpdater($file);
try {
    $updater->update(array("speed" => new NoString));
    echo "update: no throw\n";
} catch (Throwable $e) {
    echo "update: ", get_class($e), "\n";
}
var_dump(file_exists($file));
?>
--EXPECT--
NoString: Error
bool(false)
Throws: RuntimeException
bool(false)
update: Error
bool(false)
