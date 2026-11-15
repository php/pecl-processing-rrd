--TEST--
Failed option conversion stops before writing on every supported PHP version
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
/* Before PHP 7.4, object conversion reports a recoverable error. */
set_error_handler(function ($severity, $message) {
    throw new RuntimeException($message);
});
$file = __DIR__ . '/conversion-error.rrd';
$value = new stdClass;
$options = array('--step', $value,
    'DS:speed:GAUGE:600:U:U', 'RRA:AVERAGE:0.5:1:24');
try {
    rrd_create($file, $options);
    echo "conversion succeeded\n";
} catch (Throwable $e) {
    echo "conversion failed\n";
} finally {
    restore_error_handler();
}
var_dump(file_exists($file), $options[1] === $value);
@unlink($file);
?>
--EXPECT--
conversion failed
bool(false)
bool(true)
