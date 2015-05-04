<?php

$params = array();
parse_str(implode('&', array_splice($argv, 1)), $params);
current($params);
$content = file('raw/' . key($params) . '.txt');
$json = array();
foreach ($content as $line => $string) {
    list($id, $lang) = explode(",", $string, 2);
    $json = array_merge($json, array(strtolower($id) => trim($lang)));
}
$fp = fopen(key($params) . '.json', 'w+');
fwrite($fp, json_encode($json, JSON_UNESCAPED_SLASHES | JSON_PRETTY_PRINT));
fclose($fp);
