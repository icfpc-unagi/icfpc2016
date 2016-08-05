<?php

function Fetch($path, $params = NULL) {
  $header = [
      'X-API-Key: 42-59d13e150cd0de2292ef5adf36cb1e26',
      'Expect: ',
      'Accept-Encoding: gzip',
  ];
  $http = [
      'method'  => is_null($params) ? 'GET' : 'POST',
      'header'  => implode("\r\n", $header),
  ];
  if (!is_null($params)) {
    $params = http_build_query($params, '', '&');
    $header[] = 'Content-Type: application/x-www-form-urlencoded';
    $header[] = 'Content-Length: ' . strlen($params);
    $http['content'] = $params;
  };

  $url = "http://2016sv.icfpcontest.org/api${path}";
  for ($i = 0; $i < 3; $i++) {
    $result = file_get_contents(
        $url, false, stream_context_create(['http' => $http]));
    if ($result) break;
    fwrite(STDERR, "Failed to get.\n");
    sleep(pow(2, $i));
  }
  return gzdecode($result);
}

function Prettify($data) {
  return json_encode(json_decode($data), JSON_PRETTY_PRINT);
}

$list = json_decode(Fetch('/snapshot/list'), TRUE);
$snapshots = [];
foreach ($list['snapshots'] as $snapshot) {
  $snapshots[$snapshot['snapshot_time']] = $snapshot['snapshot_hash'];
}
ksort($snapshots);
$snapshot = array_pop($snapshots);
$path = dirname(__FILE__) . '/snapshot.json';
file_put_contents($path, Prettify(Fetch("/blob/$snapshot")));

$contest = json_decode(file_get_contents($path), TRUE);
foreach ($contest['problems'] as $problem) {
  $path = dirname(__FILE__) . '/problems/' . $problem['problem_id'] . '.txt';
  if (file_exists($path)) continue;
  sleep(1);
  file_put_contents($path, Fetch("/blob/{$problem['problem_spec_hash']}"));
}
