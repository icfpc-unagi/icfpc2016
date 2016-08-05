<?php

require_once(dirname(__FILE__) . '/config.php');
// define('DEBUG_MODE', 'sql');

function CallApi($path, $params = NULL) {
  $url = "http://2016sv.icfpcontest.org/api${path}";

  for ($i = 0; $i < 3; $i++) {
    fwrite(STDERR, "Calling $url...");
    $curl = curl_init($url);
    if (!is_null($params)) {
      curl_setopt($curl, CURLOPT_POST, TRUE);
      curl_setopt($curl, CURLOPT_POSTFIELDS, http_build_query($params));
    }
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($curl, CURLOPT_FOLLOWLOCATION, TRUE);
    curl_setopt($curl, CURLOPT_ENCODING, '');
    curl_setopt($curl, CURLOPT_HTTPHEADER,
                ['X-API-Key: 42-59d13e150cd0de2292ef5adf36cb1e26']);
    $output = curl_exec($curl);
    $status = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    curl_close($curl);
    fwrite(STDERR, "Status: $status\n");
    if ($status != 429) break;
    sleep(pow(2, $i));
  }
  if ($status != 200) {
    return NULL;
  }
  return $output;
}

function GetBlob($hash) {
  $data = Database::SelectCell(
      'SELECT `blob_data` FROM `blob` WHERE `blob_id` = {blob_id}',
      ['blob_id' => $hash]);
  if ($data !== FALSE) {
    return $data;
  }
  $data = CallApi("/blob/$hash");
  if (is_null($data)) {
    return NULL;
  }
  Database::Command(
      'INSERT `blob`(`blob_id`, `blob_data`) VALUES({blob_id}, {blob_data})',
      ['blob_id' => $hash, 'blob_data' => $data]);
  return $data;
}

function UpdateSnapshot() {
  $data = json_decode(CallApi('/snapshot/list'), TRUE);
  if ($data['ok'] !== TRUE) {
    return -1;
  }
  $values = [];
  foreach ($data['snapshots'] as $snapshot) {
    $values[] = [
        'snapshot_id' => $snapshot['snapshot_time'],
        'blob_id' => $snapshot['snapshot_hash']];
  }
  Database::Command('INSERT IGNORE INTO `snapshot` {values}',
                    ['values' => $values]);
  return Database::AffectedRows();
}

function GetSnapshot() {
  global $argv;

  $blob_id = NULL;
  if (isset($_GET['blob_id'])) {
    $blob_id = $_GET['blob_id'];
  } else if (isset($argv[1])) {
    $blob_id = $argv[1];
  } else {
    $blob_id = Database::SelectCell(
        'SELECT `blob_id` FROM `snapshot` ORDER BY `snapshot_id` DESC LIMIT 1');
  }
  $data = GetBlob($blob_id);
  if (is_null($data)) {
    return NULL;
  }
  $data = json_decode($data, TURE);
  if (!isset($data['problems'])) {
    return NULL;
  }
  return $data;
}
