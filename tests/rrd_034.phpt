--TEST--
rrd_fetch on a file whose header repeats a data source name
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
$file = dirname(__FILE__) . "/duplicate-ds.rrd";
@unlink($file);

rrd_create($file, array("--start", "920804400", "--step", "300",
                        "DS:speed1:GAUGE:600:U:U", "DS:speed2:GAUGE:600:U:U",
                        "RRA:AVERAGE:0.5:1:24"));
rrd_update($file, array("920804700:1:2", "920805000:3:4", "920805300:5:6"));

/* rename the second data source over the first, which rrd_create would
   never produce but any file on disk can contain */
$raw = file_get_contents($file);
$at  = strpos($raw, "speed2");
file_put_contents($file, substr_replace($raw, "speed1", $at, 6));

$result = rrd_fetch($file, array("AVERAGE", "--start", "920804400", "--end", "920805600"));
var_dump(is_array($result), array_keys($result["data"]));
echo "survived\n";

@unlink($file);
?>
--EXPECT--
bool(true)
array(1) {
  [0]=>
  string(6) "speed1"
}
survived
