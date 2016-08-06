<?php

require_once(dirname(__FILE__) . '/../library/api.php');

function Call() {
  $solution = GetParameterOrDie('solution');

  return Execute([
      'command' => 'solution',
      'alsologtostderr' => 1,
      'stdin' => $solution]);
}

if (count(debug_backtrace()) == 0) {
  echo json_encode(Call()) . "\n";
}
