<?php
require_once('common.php');

// mktime(0, 0, 0, 1, 1, 2000) == 946713600
if(@$_POST['lookup'])
{
	if(isset($_POST['timestamp']))
		$template_infobox[] = (int)$_POST['timestamp'].": <b>".date($config_datetimeformat,$_POST['timestamp']+946713600)."</b>";
	else
		$template_errorbox[] = "Please enter a SecondsSince2000 timestamp.";
}

$template_title = 'Admin: SecondsSince2000';
$template_admin_menu = true;
include('template_top.php');

echo "<form method=post>\n";
echo "<table>\n";
echo "<tr><th class='rowh'>Actions\n";
echo "<tr><td class='row0'><div class='label'>Timestamp</div><input type=text name='timestamp'> <input class='butsearch' type=submit name='lookup' value='Get Date'>\n";
echo "<tr><td class='rowf'><a href='admin.php'><img src='img/admin.png' class='icon'> Go to the Admin Menu</a>\n";
echo "</table>\n";
echo "</form>\n";

include('template_bottom.php');
?>