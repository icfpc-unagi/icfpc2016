<html><title>Snapshots</title>
<body>
<ul>
<?php

require_once(dirname(__FILE__) . '/../library/api.php');

foreach (Database::Select(
    'SELECT * FROM `snapshot` ORDER BY `snapshot_id` DESC') as $row) {
  echo '<li><a href="snapshot.php?blob_id=' . $row['blob_id'] . '">' .
       date('Y-m-d H:i:s', $row['snapshot_id']) . "</a></li>\n";
}

?>
</ul>
</body>
