--TEST--
RRDCreator, RRDUpdater and RRDGraph without a constructor call
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
foreach (array("RRDCreator", "RRDUpdater", "RRDGraph") as $class) {
    $obj = (new ReflectionClass($class))->newInstanceWithoutConstructor();
    try {
        switch ($class) {
            case "RRDCreator":
                $obj->addDataSource("speed:COUNTER:600:U:U");
                $obj->addArchive("AVERAGE:0.5:1:24");
                $obj->save();
                break;
            case "RRDUpdater":
                $obj->update(array("speed" => 1));
                break;
            case "RRDGraph":
                $obj->setOptions(array("--start" => "920804400"));
                $obj->save();
                break;
        }
        echo $class, ": no exception\n";
    } catch (Exception $e) {
        echo $class, ": ", $e->getMessage(), "\n";
    }
}
?>
--EXPECT--
RRDCreator: the object was not constructed
RRDUpdater: the object was not constructed
RRDGraph: the object was not constructed
