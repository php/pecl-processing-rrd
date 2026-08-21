--TEST--
methods on an object whose constructor threw
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
/* RRDCreator rejects an empty path, so the object exists but was never set up */
try {
    $creator = new RRDCreator("");
} catch (Exception $e) {
    echo "construct: ", $e->getMessage(), "\n";
}

$creator = (new ReflectionClass("RRDCreator"))->newInstanceWithoutConstructor();
try { $creator->__construct(""); } catch (Exception $e) { }
try { $creator->save(); } catch (Exception $e) { echo "save: ", $e->getMessage(), "\n"; }

$updater = (new ReflectionClass("RRDUpdater"))->newInstanceWithoutConstructor();
try { $updater->update(array("speed" => 1)); } catch (Exception $e) { echo "update: ", $e->getMessage(), "\n"; }

$graph = (new ReflectionClass("RRDGraph"))->newInstanceWithoutConstructor();
$graph->setOptions(array("--start" => "920804400"));
try { $graph->saveVerbose(); } catch (Exception $e) { echo "saveVerbose: ", $e->getMessage(), "\n"; }
?>
--EXPECT--
construct: path for rrd file cannot be empty string
save: the object was not constructed
update: the object was not constructed
saveVerbose: the object was not constructed
