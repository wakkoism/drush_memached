<?php
/**
 * @file
 * Provide Drush integration for release building and dependency building.
 */

/**
 * Use variable to force cache clear, where required,
 * even if no-cache-clear option is used. Defaults to false.
 */
$force_all_cache_clear = FALSE;

/**
 * Implements hook_drush_help().
 */
function flush_memached_drush_help($section) {
  switch ($section) {
    case 'drush:flush-memcached' :
      return dt('Flush all memcached socket.');
  }
}

/**
 * Implements hook_drush_command().
 */
function flush_memcached_drush_command() {
  $items = array();

  $items['flush-memcached'] = array('description' => 'Flush memcached sockets.', 'callback' => 'drush_flush_memcached', 'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap.
  'examples' => array('drush flushmc' => 'Flush sockets for current drupal site.', ), 'aliases' => array('flushmc'), );

  return $items;
}

/**
 * Callback for drush command to flush memcached data.
 */
function drush_flush_memcached() {
  if (!drush_bootstrap_to_phase(DRUSH_BOOTSTRAP_DRUPAL_DATABASE)) {
    return drush_set_error('DRUPAL_SITE_NOT_FOUND', dt('You need to specify an alias or run this command within a drupal site.'));
  }
  $servers = variable_get('memcache_servers');
  $protocol = 'unix://';
  $unix_string_length = strlen($protocol);

  $descriptors = array(
  // stdin
  0 => array('pipe', 'r'),
  // stdout
  1 => array('pipe', 'w'),
  // stderr
  2 => array('pipe', 'w'), );

  foreach ($servers as $server => $bin) {
    if (substr($server, 0, $unix_string_length) === $protocol) {
      $address = substr($server, $unix_string_length);
      $command = 'echo "flush_all" | nc -U ' . escapeshellarg($address);

      if ($h = proc_open($command, $descriptors, $pipes)) {
        $output = '';
        while (!feof($pipes[1])) {
          $output .= fgets($pipes[1]);
        }
        $error = '';
        while (!feof($pipes[2])) {
          $error .= fgets($pipes[2]);
        }

        fclose($pipes[0]);
        fclose($pipes[1]);
        fclose($pipes[2]);
        $return_code = proc_close($h);
        if ($error) {
          echo $error;
        } else {
          echo $output;
        }
      }
    }
  }
}
