--TEST--
RRDUpdater more data source test
--SKIPIF--
<?php
include('skipif.inc');
include('rrdtool-bin.inc');
include('data/definition.inc');
foreach(array($data_moreDSEmptyDb, $data_moreDSUpdaterTxt) as $file) {
	if (!file_exists($file)) {
		die("skip $file doesnt' exist");
	}
}
?>
--FILE--
<?php
include('rrdtool-bin.inc');
include('data/definition.inc');
$rrdFile = dirname(__FILE__) . "/rrd_updater_moreDS_empty.rrd";
copy($data_moreDSEmptyDb, $rrdFile);

$updator = new RRDUpdater($rrdFile);
$updator->update(array("speed1" => 12345, "speed2" => 11340), 920804700);
$updator->update(array("speed1" => 12357, "speed2" => 11357), 920805000);
$updator->update(array("speed1" => 12363, "speed2" => 11363), 920805300);

//mostly for "visual test"
$command = "$rrdtool_bin fetch $rrdFile AVERAGE "
	. "--start 920804400 --end 920808000 "
	. ">" . dirname(__FILE__) . "/rrd_updater_moreDS_fetch.txt";
$output = array();
exec($command, $output);
$originalFetch = file($data_moreDSUpdaterTxt, FILE_IGNORE_NEW_LINES);
echo "comparing original and current fetch\n";
var_dump(array_diff($output, $originalFetch));
?>
--EXPECTF--
comparing original and current fetch
array(0) {
}


