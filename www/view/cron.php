<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function GetCronToken($name, $duration) {
  Database::Command('
      UPDATE `token` SET `token_value` = UNIX_TIMESTAMP()
      WHERE `token_id` = {token_id} AND
            `token_value` < UNIX_TIMESTAMP() - {duration} LIMIT 1',
      ['token_id' => "cron_$name", 'duration' => $duration]);
  if (Database::AffectedRows() > 0) {
    return TRUE;
  }
  return FALSE;
}

function Run() {
  $output = '';
  $output .= trim(file_get_contents('http://db.sx9.jp/view/submit.php')) . "\n";
  $output .= trim(file_get_contents('http://db.sx9.jp/view/submit_problem.php')) . "\n";
  if (GetCronToken('unlock_auto', 30)) {
    $output .= 'Unlock auto: ' . trim(file_get_contents('http://db.sx9.jp/view/unlock_auto.php')) . "\n";
  }
  if (GetCronToken('update', 120)) {
    $output .= 'Update: ' . trim(file_get_contents('hhttp://db.sx9.jp/api/update.php')) . "\n";
  }
  return $output;
}

$output = Run();
Database::Command(
    'INSERT INTO `cron`{cron}', ['cron' => ['cron_output' => $output]]);
