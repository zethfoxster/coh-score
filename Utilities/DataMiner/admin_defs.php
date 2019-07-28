<?php
require_once('common_sql.php');
require_once('utils.php');

$template_title = 'Admin: Defs';
$template_admin_menu = true;
include('template_top.php');

echo "<table>\n";
echo "<tr><th class='rowh'>Def Types\n";
$row = 0;
foreach(new QueryIterator("SELECT DISTINCT type FROM defs") as $v)
	echo "<tr><td class='row$row'>$v[type]\n";
echo "</table>\n";

include('template_bottom.php');
?>