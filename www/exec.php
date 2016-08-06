<?php

function Fail($status, $message) {
  header('HTTP/1.1 ' . $status);
  echo trim($message) . "\n";
  exit;
}

function Run($command, $input) {
  $process = proc_open(
      $command,
      [0 => ['pipe', 'r'],
       1 => ['pipe', 'w'],
       2 => ['pipe', 'w']],
      $pipes, NULL, NULL);
  if ($process === NULL) {
    Fail('500 Internal Error', "Failed to run $command");
  }
  fwrite($pipes[0], str_replace("\r\n", "\n", $input));
  fclose($pipes[0]);
  $stdout = stream_get_contents($pipes[1]);
  $stderr = stream_get_contents($pipes[2]);
  fclose($pipes[1]);
  fclose($pipes[2]);
  $return_value = proc_close($process);
  return ['command' => $command,
          'stdout' => $stdout,
          'stderr' => $stderr,
          'code' => $return_value];
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

echo json_encode(Run(
    'timeout 10s /alloc/global/bin/' .$command . $flags, $input));

foreach ($files as $file) {
  @unlink($file);
}
