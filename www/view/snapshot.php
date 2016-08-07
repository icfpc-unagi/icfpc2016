<?php

require_once(dirname(__FILE__) . '/../library/api.php');

$snapshot = GetSnapshot();

if (is_null($snapshot)) {
  die('Invalid snapshot');
}

if (GetParameter('plain')) {
  header('Content-Type: text/plain');
  echo json_encode($snapshot);
  exit();
}

$users = [];
for ($id = 1; $id < 10; $id++) {
  $users["$id"] = "Contest Organizer Problem Set $id";
}
foreach ($snapshot['users'] as $user) {
  $users[$user['username']] = $user['display_name'];
}

ksort($users);

StartPage();

ob_start();

?>
<table class="table table-condensed table-striped">
  <tbody>
<tr><th<?php if (!$full) echo ' colspan="2"'; ?>>ID</th><th>Owner</th><th>Problem Size</th><th>Owner's Solution Size</th><th>Solved</th><th>Solutions</th><th>Publish Time</th></tr>
<?php

$solutions = [];
foreach (Database::Select('
    SELECT
        `problem_id`,
        COUNT(*) AS `solution_count`,
        MAX(`solution_resemblance`) AS `solution_resemblance`
    FROM `solution`
    WHERE `solution_submission` IS NOT NULL
    GROUP BY `problem_id`') as $problem) {
  $solutions[$problem['problem_id']]['submitted'] = $problem;
}
foreach (Database::Select('
    SELECT
        `problem_id`,
        COUNT(*) AS `solution_count`,
        MAX(`solution_resemblance`) AS `solution_resemblance`
    FROM `solution`
    GROUP BY `problem_id`') as $problem) {
  $solutions[$problem['problem_id']]['all'] = $problem;
}

$not_solved = 0;
$stealth = 0;
$num_problems = 0;
foreach ($snapshot['problems'] as $problem) {
  $solved = 0;
  $resemblance = 0.0;
  foreach ($problem['ranking'] as $rank) {
    if ($rank['resemblance'] == 1) {
      $solved++;
    } else {
      $resemblance = max($resemblance, $rank['resemblance']);
    }
  }
  $solution = $solutions[intval($problem['problem_id'])];
  if ($solution['all']['solution_resemblance'] == 1 &&
      $solution['submitted']['solution_resemblance'] != 1) {
    $stealth++;
  }
  if ($problem['owner'] != '42') {
    $num_problems++;
  }
  if ($solution['all']['solution_resemblance'] == 1 ||
      $problem['owner'] == '42') {
    if (!$full) continue;
  } else {
    $not_solved++;
  }
  echo '<tr>';
  echo '<td><a href="problem.php?problem_id=' . $problem['problem_id'] .
       '">' . $problem['problem_id'] . "</a></td>";
  if (!$full) {
    echo '<td><img src="../summary/' . $problem['problem_id'] . '.png" width="32" height="32"></td>';
  }
  echo '<td>' . htmlspecialchars($users[$problem['owner']]) . "</td>";
  echo '<td>' . $problem['problem_size'] . "</td>";
  echo '<td>' . $problem['solution_size'] . "</td>";
  if ($solved > 0) {
    echo '<td>' . $solved . '</td>';
  } else {
    echo '<td>(' . $resemblance . ')</td>';
  }
  echo '<td><a href="problem.php?problem_id=' . $problem['problem_id'] . '">';
  if (!isset($solution['all']['solution_count'])) {
    echo 'No';
  } else if ($solution['submitted']['solution_resemblance'] == 1.0) {
    echo '<b>Solved</b>';
  } else if ($solution['all']['solution_resemblance'] == 1.0) {
    echo '<b style="color:red">Solved</b>';
  } else if ($solution['submitted']['solution_resemblance'] ==
             $solution['all']['solution_resemblance']) {
    echo $solution['submitted']['solution_resemblance'];
  } else if (is_null($solution['all']['solution_resemblance'])) {
    echo '??????';
  } else {
    echo '<span style="color:red">(' .
         $solution['all']['solution_resemblance'] . ')</span>';
  }
  echo '</a></td>';
  echo '<td>' . date('Y-m-d H:i:s', $problem['publish_time']) . "</td>";
  echo "</tr>\n";
}

?>
</table>
<?php

$contents = ob_get_clean();

?>
<div style="font-size:400%;font-weight:bold;text-align:center;">☆☆☆　全完まで<?php echo $not_solved; ?>問！　☆☆☆</div>
<h1 class="page-header">Problems at <?php
echo date('Y-m-d H:i:s', $snapshot['snapshot_time']);

echo ' （全' . $num_problems . '問中ステルス' . $stealth . '問）';

?></h1>
<div><?php
$full = !is_null(GetParameter('full'));
if (!$full) {
  echo '[<a href="?full=1">Full</a>]';
} else {
  echo '[<a href="?">Compact</a>]';
}
?></div>
<?php echo $contents; ?>
<h1 class="page-header">Leaderboard</h1>
<table class="table table-condensed table-striped">
  <tbody>
<tr><th>Rank</th><th>Team name</th><th>Score</th></tr>
<?php

$rank = 1;
foreach ($snapshot['leaderboard'] as $user) {
  echo '<tr>';
  echo '<td>' . $rank . '</td>';
  echo '<td>' . htmlspecialchars($users[$user['username']]) .
       ' (' . $user['username'] . ")</td>";
  echo '<td>' . $user['score'] . "</td>";
  echo "</tr>\n";
  $rank++;
}

?>
</table>
</body>
