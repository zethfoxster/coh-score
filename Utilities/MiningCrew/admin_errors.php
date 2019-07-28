<?php
require_once('common_sql.php');

$template_title = 'Admin: Errors';
$template_admin_menu = true;
include('template_top.php');

echo "<table>\n";
$files = scandir('errors');
rsort($files);
foreach($files as $v)
	if(substr($v,-5) == '.html')
		echo "<tr><td><span class='label'>".date($config_datetimeformat, substr($v,0,-5))."</span><br>\n".file_get_contents("errors/$v")."\n";
echo "</table>";

include('template_bottom.php');
?>