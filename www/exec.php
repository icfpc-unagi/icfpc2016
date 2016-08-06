<?php

function Fail($status, $message) {
  header('HTTP/1.1 ' . $status);
  echo trim($message) . "\n";
  exit;
}

$params = $_REQUEST;
ksort($params);
$files = [];
$command = '';
$input = '';
$flags = '';
foreach ($params as $key => $value) {
  if ($key == 'command') {
    $command = $value;
    continue;
  }
  if ($key == 'stdin') {
    $input = $value;
    continue;
  }
  if (preg_match('%^(.*)_file$%', $key, $match)) {
    $temp = tempnam(sys_get_temp_dir(), 'tmp');
    file_put_contents($temp, $value);
    $files[] = $temp;
    $key = $match[1];
    $value = $temp;
  }
  if (preg_match('%^arg(\\d+)$%', $key)) {
    $flags .= ' ' . escapeshellarg("$value");
  } else {
    $flags .= ' ' . escapeshellarg("--$key=$value");
  }
}

if (strlen($command) == 0) {
  Fail('400 Bad Request', 'command is required');
}

$stdin = tempnam(sys_get_temp_dir(), 'tmp');
$stdout = tempnam(sys_get_temp_dir(), 'tmp');
$stderr = tempnam(sys_get_temp_dir(), 'tmp');
$return = tempnam(sys_get_temp_dir(), 'tmp');
file_put_contents($stdin, $input);
$files[] = $stdin;
$files[] = $stdout;
$files[] = $stderr;
$files[] = $return;
exec("timeout 10s /alloc/global/bin/$command$flags < $stdin >$stdout 2>$stderr; echo \$? > $return");
echo json_encode([
    'command' => "$command$flags",
    'stdout' => file_get_contents($stdout),
    'stderr' => file_get_contents($stderr),
    'code' => intval(file_get_contents($return))]) . "\n";

foreach ($files as $file) {
  @unlink($file);
}
