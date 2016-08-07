<?php

function Prettify($data) {
  return json_encode(json_decode($data), JSON_PRETTY_PRINT);
}

date_default_timezone_set('Asia/Tokyo');
ini_set('memory_limit', '1G');

$path = dirname(__FILE__) . '/snapshot.json';
file_put_contents($path,
    Prettify(file_get_contents('http://db.sx9.jp/view/snapshot.php?plain=1')));

$contest = json_decode(file_get_contents($path), TRUE);
echo "Snapshot at " . date('Y-m-d H:i:s', $contest['snapshot_time']) . "\n";
foreach ($contest['problems'] as $problem) {
  $path = dirname(__FILE__) . '/problems/' . $problem['problem_id'] . '.txt';
  if (file_exists($path)) continue;
  sleep(1);
  echo "Fetching {$problem['problem_spec_hash']}...\n";
  file_put_contents($path, file_get_contents(
      'http://db.sx9.jp/api/blob.php?blob_id=' .
      $problem['problem_spec_hash']));
}
