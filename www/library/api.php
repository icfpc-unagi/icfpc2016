<?php

require_once(dirname(__FILE__) . '/config.php');
// define('DEBUG_MODE', 'sql');
define('API_HOST', 'http://db.sx9.jp');

function Fail($message, $status = '400 Bad Request') {
  header('HTTP/1.1 ' . $status);
  echo trim($message) . "\n";
  exit();
}

function Execute($params) {
  $data = http_build_query($params, "", "&");
  $context = stream_context_create([
      'http' => [
          'method' => 'POST',
          'header' =>
              "Content-Type: application/x-www-form-urlencoded\r\n" .
              "Content-Length: " . strlen($data),
          'content' => $data]]);
  $output = file_get_contents(API_HOST . '/exec.php', false, $context);
  if (strlen($output) == 0) {
    return NULL;
  }
  return json_decode($output, TRUE);
}

function GetParameter($name) {
  if (isset($_REQUEST[$name])) {
    return $_REQUEST[$name];
  } else if (isset($_FILES[$name])) {
    return file_get_contents($_FILES[$name]['tmp_name']);
  } else if (isset($_ENV[$name])) {
    return $_ENV[$name];
  } else if (isset($_ENV[$name . '_file'])) {
    return file_get_contents($_ENV[$name . '_file']);
  }
  return NULL;
}

function GetParameterOrDie($name) {
  $result = GetParameter($name);
  if (is_null($result)) {
    Fail("$name is required.");
  }
  return $result;
}

function GetToken() {
  for ($i = 0; $i < 10; $i++) {
    $current_time = intval(array_sum(array_map(
        'floatval', explode(' ', microtime()))) * 1000000);
    Database::Command('
        UPDATE `token` SET `token_value` = {value}
        WHERE `token_id` = "api" AND
              `token_value` < {value} - 1100000 LIMIT 1',
        ['value' => $current_time]);
    if (Database::AffectedRows() > 0) {
      return TRUE;
    }
    usleep(500 * 1000);
  }
  return FALSE;
}

function CallApi($path, $params = NULL) {
  $url = "http://2016sv.icfpcontest.org/api${path}";

  for ($i = 0; $i < 3; $i++) {
    if (!GetToken()) {
      sleep(rand(1, pow(2, $i)));
      continue;
    }
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
                ['X-API-Key: 42-59d13e150cd0de2292ef5adf36cb1e26',
                 'Expect: ']);
    $output = curl_exec($curl);
    $status = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    curl_close($curl);
    Database::Command(
        'INSERT INTO `api`{api}',
        ['api' => [
            'api_path' => $path,
            'api_output' => $output,
            'api_status' => intval($status)]]);
    if ($status != 429) break;
    sleep(pow(2, $i));
  }
  if ($status != 200 && $status != 400 && $status != 403) {
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

function SubmitSolution($problem_id, $data) {
  $data = CallApi('/solution/submit',
                  ['problem_id' => $problem_id, 'solution_spec' => $data]);
  if (is_null($data)) {
    return NULL;
  }
  $data = json_decode($data, TURE);
  if (!isset($data['ok'])) {
    return NULL;
  }
  return $data;
}

function SubmitProblem($publish_time, $data) {
  $data = CallApi('/problem/submit',
                  ['publish_time' => $publish_time, 'solution_spec' => $data]);
  if (is_null($data)) {
    return NULL;
  }
  $data = json_decode($data, TURE);
  if (!isset($data['ok'])) {
    return NULL;
  }
  return $data;
}

function FormatData($data) {
  $data = preg_replace('%\\s*[\\r\\n]+\\s*%', "\n", trim($data));
  return $data . "\n";
}

function RenderPage($buffer) {
  $output = '<!doctype html><html><head><meta charset="UTF-8">';
  $output .= '<title>ICFPC 2016</title>';
  $output .= '<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css">';
  $output .= '<style>table.layout { width: 100%; table-layout: fixed } table.layout > tbody > tr > td { padding: 20px; vertical-align: top; } .pending { color: #aaa } .navbar-default { background-color: #def; border-color: #abc; } </style>';
  $output .= '</head><body>';
  $output .= '<nav class="navbar navbar-default">
      <div class="container">
        <div class="navbar-header">
          <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar-collapse" aria-expanded="false">
            <span class="sr-only">Toggle navigation</span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </button>
          <a class="navbar-brand" href="snapshot.php">🍤 Team Unagi</a>
        </div>
        <div class="collapse navbar-collapse" id="navbar-collapse">
          <ul class="nav navbar-nav">
            
            <li class="False">
              <a href="snapshot.php">
                Leaderboard
              </a>
            </li>
            <li class="False">
              <a href="list.php">
                Snapshots
              </a>
            </li>
          </ul>
        </div>
      </div>
    </nav>';
  $output .= '<div class="container">';
  $output .= "\n$buffer\n";
  $output .= "</div></body></html>\n";
  return $output;
}

function StartPage() {
  ob_start('RenderPage');
}
