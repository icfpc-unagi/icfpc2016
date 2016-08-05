<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$solution_id = NULL;
if (isset($_GET['solution_id'])) {
  $solution_id = $_GET['solution_id'];
} else if (isset($_ENV['solution_id'])) {
  $solution_id = $_ENV['solution_id'];
} else {
  die('solution_id must be given.');
}

$result = Database::SelectCell('SELECT `solution_data` FROM `solution` WHERE `solution_id` = {solution_id}', ['solution_id' => $solution_id]);

if ($result === FALSE) {
  die('Solution is not found.');
}

header('Content-Type: text/plain');
echo $result;
