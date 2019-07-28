<?php
require_once('common_sql.php');
require_once('utils_sql.php');

function add_dir($directory)
{
	$dir = opendir($directory);
	while(($file = readdir($dir)) !== false)
	{
		if($file == '.' || $file == '..' || substr($file,0,1) == '_')
			continue;

		$file = "$directory/$file";
		if(is_dir($file))
			add_dir($file);
		else if(substr($file,-3) == '.ms')
		{
			echo add_ms_file($file)." PStrings added from $file<br>\n";
			ob_flush();
			flush();
		}
	}
}

echo "<html><body>\n";
add_dir('English');
echo "</body></html>";
?>