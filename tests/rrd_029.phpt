--TEST--
RRDCreator::save() and RRDGraph::saveVerbose() honour open_basedir
--SKIPIF--
<?php
include('skipif.inc');
include('data/definition.inc');
if (!file_exists($data_updatedDb)) {
	die("skip $data_updatedDb doesn't exist");
}
?>
--INI--
open_basedir=
--FILE--
<?php
include('data/definition.inc');
$outside = sys_get_temp_dir() . "/rrd-outside-basedir";
@mkdir($outside);
ini_set("open_basedir", dirname(__FILE__) . DIRECTORY_SEPARATOR);

$creator = new RRDCreator("$outside/created.rrd");
$creator->addDataSource("speed:COUNTER:600:U:U");
$creator->addArchive("AVERAGE:0.5:1:24");
var_dump($creator->save());

$graph = new RRDGraph("$outside/graph.png");
$graph->setOptions(array(
	"--start" => "920804400",
	"--end" => "920808000",
	0 => "DEF:myspeed=$data_updatedDb:speed:AVERAGE",
	1 => "LINE1:myspeed#FF0000",
));
var_dump($graph->saveVerbose());
?>
--EXPECTF--
Warning: RRDCreator::save(): open_basedir restriction in effect. File(%s) is not within the allowed path(s): (%s) in %s on line %d
bool(false)

Warning: RRDGraph::saveVerbose(): open_basedir restriction in effect. File(%s) is not within the allowed path(s): (%s) in %s on line %d
bool(false)
