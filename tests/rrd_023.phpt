--TEST--
RRDCreator::__construct() with no startTime argument
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$file = dirname(__FILE__) . "/ctor-no-start.rrd";
$primer_file = dirname(__FILE__) . "/ctor-primer.rrd";
@unlink($file);

/* leaves a zend_string pointer in the stack slot the next constructor call
   would read startTime from */
$primer = new RRDCreator($primer_file, "920804400", 300);
unset($primer);

$creator = new RRDCreator($file);
$creator->addDataSource("speed:COUNTER:600:U:U");
$creator->addArchive("AVERAGE:0.5:1:24");
var_dump($creator->save());
var_dump(file_exists($file));

@unlink($file);
@unlink($primer_file);
?>
--EXPECT--
bool(true)
bool(true)
