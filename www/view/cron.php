<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function Run() {
  $data = '';
  $data .= trim(file_get_contents('http://db.sx9.jp/view/submit.php')) . "\n";
  return $data;
}

$data = Run();
Database::Command(
    'INSERT INTO `cron`{cron}', ['cron' => ['cron_output' => $output]]);
