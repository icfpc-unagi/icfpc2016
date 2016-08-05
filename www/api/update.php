<?php

require_once(dirname(__FILE__) . '/../library/api.php');

if (php_sapi_name() != 'cli') {
  header('Content-Type: text/plain');
}

echo UpdateSnapshot();
