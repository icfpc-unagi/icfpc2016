<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$hash = NULL;

if (php_sapi_name() == 'cli') {
  if (!isset($argv[1])) {
    fwrite(STDERR, "Usage: blob.php hash\n");
    return 1;
  }
  $hash = $argv[1];
} else {
  header('Content-Type: text/plain');
  $hash = $_GET['hash'];
}

echo GetBlob($hash);
