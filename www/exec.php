<?php

function Fail($status, $message) {
  header('HTTP/1.1 ' . $status);
  echo trim($message) . "\n";
  exit;
}

function Run($command, $input, $output) {
  $process = proc_open(
      "timeout 10s /alloc/global/bin/$command",
      [0 => ['pipe', 'r'],
       1 => ['pipe', 'w'],
       2 => ['pipe', 'w']],
      $pipes, NULL, NULL);
  if ($process === NULL) {
    Fail('500 Internal Error', "Failed to run $command");
  }
  fwrite($pipes[0], str_replace("\r\n", "\n", $data));
  fclose($pipes[0]);
  $stdout = stream_get_contents($pipes[1]);
  $stderr = stream_get_contents($pipes[2]);
  $return_value = proc_close($process);
  return ['stdout' => $stdout, 'stderr' => $stderr, 'code' => $return_value];
}

$params = $_REQUEST;
$files = [];
$command = '';
$input = '';
$flags = '';
foreach ($params as $key => $value) {
  if ($key == 'command') {
    $command = $value;
    continue;
  }
  if ($key == 'input') {
    $input = $value;
    continue;
  }
  if (preg_match('%^(.*)_file$%', $key, $match)) {
    $temp = tempnam(sys_get_temp_dir(), 'tmp');
    file_put_contents($temp, $value);
    $files[] = $temp;
    $flags .= ' ' . escapeshellarg("--{$match[1]}=$temp");
  } else {
    $flags .= ' ' . escapeshellarg("--$key=$value");
  }
}

if (strlen($command) == 0) {
  Fail('400 Bad Request', 'command is required');
}

echo json_encode(Run($command . $flags, $input));

foreach ($files as $file) {
  @unlink($file);
}
