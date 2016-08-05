<?php

$result = str_replace("\r\n", "\n", file_get_contents('php://stdin'));

list($header, $body) = explode("\n\n", $result);
$header = explode("\n", $header);

if ($header[0] != 'HTTP/1.1 200 OK') {
  fwrite(STDERR, $body);
  exit(1);
}

$data = json_decode($body, TRUE);
if (!isset($data['code'])) {
  fwrite(STDERR, "Invalid response: $body\n");
  exit(1);
}

fwrite(STDOUT, $data['stdout']);
fwrite(STDOUT, $data['stderr']);
exit(intval($data['code']));
