--TEST--
__toString() freeing the options array while it is being walked
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
class Reenter {
    public static $graph;
    public function __toString(): string {
        /* replaces, and so frees, the array RRDGraph::save() is walking */
        self::$graph->setOptions(array("--start" => "920804400"));
        return "920804400";
    }
}
class Stop {
    /* throws once the walk has moved past the freed buckets, so librrd is
       never reached and the test does not depend on a renderer */
    public function __toString(): string { throw new RuntimeException("stop"); }
}

$graph = new RRDGraph(dirname(__FILE__) . "/reenter.png");
Reenter::$graph = $graph;

$options = array("--start" => new Reenter);
for ($i = 0; $i < 64; $i++) { $options["--pad$i"] = str_repeat("A", 64); }
$options["--end"] = new Stop;
$graph->setOptions($options);

try { $graph->save(); } catch (Throwable $e) { echo get_class($e), "\n"; }
echo "survived\n";
?>
--EXPECTF--
Warning: cannot allocate arguments options in %s on line %d
RuntimeException
survived
