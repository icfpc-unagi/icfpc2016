<?php

date_default_timezone_set('Asia/Tokyo');

require_once(dirname(__FILE__) . '/database.php');

Database::Initialize('52.193.177.29:3306', 'icfpc2016', 'ioJJyZi8');
ini_set('memory_limit', '1G');
