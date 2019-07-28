<?php
require_once('config.php');

$template_title = 'Admin';
$template_admin_menu = true;
include('template_top.php');

echo "<table>\n";
echo "<tr><th class='rowt'>Admin Actions\n";
echo "<tr><td class='rowf'><a href='admin_servers.php'><img src='img/server.png' class='icon'> Go to Servers</a>\n";
echo "<tr><td class='rowf'><a href='admin_containers.php'><img src='img/container.png' class='icon'> Go to Containers</a>\n";
echo "<tr><td class='rowf'><a href='admin_defs.php'><img src='img/def.png' class='icon'> Go to Defs</a>\n";
echo "<tr><td class='rowf'><a href='admin_pstrings.php'><img src='img/pstring.png' class='icon'> Go to Message Store (PStrings)</a>\n";
echo "</table>\n";

include('template_bottom.php');
?>