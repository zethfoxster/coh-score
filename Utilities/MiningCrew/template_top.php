<?php
require_once('config.php');
// disable browser caching
header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
header('Cache-Control: no-cache, must-revalidate');
header('Pragma: no-cache');
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<html lang='en-us'>

<head>
<title><?php echo $config_title.($template_title ? ": $template_title" : ''); ?></title>
<meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-5'>
<meta http-equiv='Content-Script-Type' content='text/javascript'>
<meta http-equiv='Content-Style-Type' content='text/css'>
<?php /* TODO: use this <link rel='Copyright' href='<?php echo $config_urlroot; ?>/copyright.php'> */ ?>
<?php /* TODO: use this <link rel='Shortcut Icon' href='<?php echo $config_urlroot; ?>/favicon.ico'> */ ?>
<style type='text/css'><!-- @import "<?php echo $config_urlroot; ?>/style.css"; --></style>
</head>

<body><div><table id='main_table'><tr><td id='main_menu'>
<table>
<tr><th class='rowt'>Main Menu
<tr><th class='rowh'>Mining Actions
<tr><td class='rowf'><a href='miner.php?a=n'><img src='img/new.png' class='icon'> Create a Miner</a>
<tr><td class='rowf'><a href='onetime.php'><img src='img/go.png' class='icon'> One Time Mine</a>
<tr><td class='rowf'><a href='miner.php'><img src='img/mine.png' class='icon'> Miner List</a>
<tr><td class='rowf'><a href='schedule.php'><img src='img/sched.png' class='icon'> Full Schedule</a>
<?php
if($template_admin_menu)
{
	echo "<tr><th class='rowh'>Administration\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php'><img src='img/server.png' class='icon'> Servers</a>\n";
	echo "<tr><td class='rowf'><a href='admin_query.php'><img src='img/query.png' class='icon'> Manual Query</a>\n";
	echo "<tr><td class='rowf'><a href='admin_containers.php'><img src='img/container.png' class='icon'> Containers</a>\n";
	echo "<tr><td class='rowf'><a href='admin_defs.php'><img src='img/def.png' class='icon'> Defs</a>\n";
	echo "<tr><td class='rowf'><a href='admin_pstrings.php'><img src='img/pstring.png' class='icon'> PStrings</a>\n";
	echo "<tr><td class='rowf'><a href='admin_ss2000.php'><img src='img/ss2000.png' class='icon'> SS2000</a>\n";
	echo "<tr><td class='rowf'><a href='admin_errors.php'><img src='img/errors.png' class='icon'> Errors</a>\n";
}
else
	echo "<tr><td class='rowf'><a href='admin.php'><img src='img/admin.png' class='icon'> Admin Menu</a>";
?>
</table>
<td id='main_body'><table id='body_table'><tr><td>
<?php

foreach($template_errorbox as $v)
	echo "<div class='errorbox'><div class='boxicon'><img src='img/errorbox.png' class='boxicon'></div>$v</div>\n";
foreach($template_warnbox as $v)
	echo "<div class='warnbox'><div class='boxicon'><img src='img/warnbox.png' class='boxicon'></div>$v</div>\n";
foreach($template_infobox as $v)
	echo "<div class='infobox'><div class='boxicon'><img src='img/infobox.png' class='boxicon'></div>$v</div>\n";
?>