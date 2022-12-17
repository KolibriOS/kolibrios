<?php

$your_local_page_address = '/proxy.php'; // if in /index.php: '/' or '/index.php' (depends on request url)
$your_useragent = 'Mozilla/5.0 (X11; Linux x86_64; rv:108.0) Gecko/20100101 Firefox/108.0';

$site = substr($_SERVER['REQUEST_URI'], strlen($your_local_page_address . '?site='));
$site = str_replace("\\", "\\\\", $site);
$site = str_replace("`", "\\`", $site);
$site = str_replace("$", "\\$", $site);
$site = str_replace("\"", "\\\"", $site);
echo passthru('curl -L -A "' . $your_useragent . '" "' . $site . '"');

?>
