<?php
require_once('common_sql.php');
require_once('utils_sql.php');

$directory = 'defs';
echo "<html><body>\n";
$dir = opendir($directory);
while(($file = readdir($dir)) !== false)
{
	if($file == '.' || $file == '..' || substr($file,0,1) == '_')
		continue;

	$file = "$directory/$file";
	echo add_def_file($file)." defs added from $file<br>\n";
	ob_flush();
	flush();
}
echo "</body></html>";
?>