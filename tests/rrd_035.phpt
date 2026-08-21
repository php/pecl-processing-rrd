--TEST--
re-constructing RRDGraph and RRDUpdater replaces the previous path
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$dir = dirname(__FILE__);
$first  = "$dir/reconstruct-a.rrd";
$second = "$dir/reconstruct-b.rrd";
@unlink($first); @unlink($second);

foreach (array($first, $second) as $f) {
    rrd_create($f, array("--start", "920804400", "--step", "300",
                         "DS:speed:GAUGE:600:U:U", "RRA:AVERAGE:0.5:1:24"));
}

/* RRDUpdater: the second construction decides where the write lands */
$updater = new RRDUpdater($first);
$updater->__construct($second);
$updater->update(array("speed" => 7), "920804700");
var_dump(rrd_last($first), rrd_last($second));

/* RRDGraph: open_basedir is checked before librrd is called at all, so a
   re-construction pointing outside it proves the path was replaced without
   rendering anything */
$graph = new RRDGraph("$dir/reconstruct.png");
$graph->setOptions(array("--start" => "920804400"));
$graph->__construct(sys_get_temp_dir() . "/reconstruct-outside.png");
ini_set("open_basedir", $dir . DIRECTORY_SEPARATOR);
var_dump($graph->save());

@unlink($first); @unlink($second);
?>
--EXPECTF--
int(920804400)
int(920804700)

Warning: RRDGraph::save(): open_basedir restriction in effect. File(%sreconstruct-outside.png) is not within the allowed path(s): (%s) in %s on line %d
bool(false)
