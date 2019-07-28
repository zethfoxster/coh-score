<html><body>
<?php
require_once('common_sql.php');

if((int)@$_GET['server'])
{
	$server = sql_fetch("SELECT * FROM servers WHERE id=".(int)$_GET['server']);
	$db = sql_server_connect($server);
	echo "<table border=1><tr><th colspan=5>$server[name]\n";
	echo "<tr><th>name<th>field<th>ever<th>this week<th>last week\n";
	foreach(new QueryIterator("SELECT name, field, ever, thisweek, lastweek FROM MiningAccumulator, MinedData WHERE MiningAccumulator.ContainerId=MinedData.ContainerId ORDER BY name, field",$db) as $v)
		echo "<tr><td>$v[0]<td>$v[1]<td>$v[2]<td>$v[3]<td>$v[4]\n";
	echo "</table>\n";
	echo "<hr>\n";
}
?>
<form method=get action='script_mineacc_dump.php'>
Server: <select name='server'>
<?php
foreach(new QueryIterator("SELECT * FROM servers") as $v)
	echo "<option value=$v[id]".((int)$_GET['server'] == $v['id'] ? ' selected' : '').">$v[name]\n";
?>
</select>
<input type=submit value='Dump MineAcc Data'>
</body></html>