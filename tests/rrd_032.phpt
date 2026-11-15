--TEST--
__toString() cannot repoint the object after the path has been checked
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
class Repoint {
    public static $obj;
    public static $target;
    public function __toString(): string {
        self::$obj->__construct(self::$target);
        return "1";
    }
}

$dir      = dirname(__FILE__);
$intended = "$dir/toctou-intended.rrd";
$other    = "$dir/toctou-other.rrd";
@unlink($intended); @unlink($other);

foreach (array($intended, $other) as $f) {
    rrd_create($f, array("--start", "920804400", "--step", "300",
                         "DS:speed:GAUGE:600:U:U", "RRA:AVERAGE:0.5:1:24"));
}

$updater = new RRDUpdater($intended);
Repoint::$obj = $updater;
Repoint::$target = $other;

try { $updater->update(array("speed" => new Repoint), "920804700"); }
catch (Throwable $e) { echo "caught\n"; }

/* the write must land on the path that was checked, not the repointed one */
var_dump(rrd_last($intended), rrd_last($other));

@unlink($intended); @unlink($other);
?>
--EXPECT--
int(920804700)
int(920804400)
