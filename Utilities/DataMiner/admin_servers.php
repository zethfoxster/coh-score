<?php
require_once('common_sql.php');

function check_server($server)
{
	if(is_array($server))
		$server = $server['id'];
	list($count) = sql_fetch("SELECT COUNT(*) FROM miners, servers WHERE miners.servers=servers.handle AND servers.id=$server");
	return !$count;
}

function check_group($group)
{
	if(is_array($group))
		$group = $group['id'];
	list($count) = sql_fetch("SELECT COUNT(*) FROM miners, serverhandles WHERE miners.servers=serverhandles.id AND serverhandles.id=$group");
	return !$count;
}

function table_servers($handle = false, $id = false, $check_links = false)
{
	$row = 0;

	echo "<table>\n";
	if(!$id && !$handle)
		echo "<tr><th class='rowt' colspan=2>Servers\n";
	if(!$id)
	{
		echo "<tr><th class='rowh'".($check_links ? '' : ' colspan=2').">".($handle ? 'Included Servers' : 'Individual Servers').($check_links ? "<th class='rowh'>Status" : '')."\n";
		if($handle)
			$query = new QueryIterator("SELECT servers.* FROM servers, serverlookup, serverhandles WHERE serverhandles.id=$handle AND serverlookup.handle=serverhandles.id AND servers.id=serverlookup.server ORDER BY servers.name");
		else
			$query = new QueryIterator("SELECT * FROM servers ORDER BY name");
		foreach($query as $v)
		{
			echo "<tr>";
			echo "<td class='row$row'".($check_links ? '' : ' colspan=2').">"
				.(check_server($v) ? "<a href='admin_servers.php?i=$v[id]&amp;a=d'><img src='img/drop.png' class='icon'></a>" : "<img src='img/blank.png' class='icon'>")
				."<a href='admin_servers.php?i=$v[id]&amp;a=n'><img src='img/copy.png' class='icon'></a>"
				."<a href='admin_servers.php?i=$v[id]&amp;a=e'><img src='img/edit.png' class='icon'></a>"
				." <a href='admin_servers.php?i=$v[id]'>$v[name]</a>";
// hack to see all the ip addresses at once
//			echo "<td class='row$row'>$v[loginaddr]";
			if($check_links)
			{
				flush(); ob_flush();
				echo "<td class='row$row'><img src='img/link".(sql_server_connect($v) ? 'good' : 'bad').".png' class='icon'>";
			}
			echo "\n";
			$row = $row ? 0 : 1;
		}
		if(!$query->num_rows())
			echo "<tr><td colspan=2 class='row$row nodata'>No servers".($handle ? '' : 'in this group')."\n";
	}

	if(!$handle)
	{
		echo "<tr><th class='rowh'>".($id ? 'Member of' : 'Server Groups')."<th class='rowh'>#\n";
		if($id)
			$query = new QueryIterator("SELECT serverhandles.* FROM serverhandles, serverlookup WHERE serverlookup.server=$id AND serverhandles.id=serverlookup.handle AND serverhandles.single=0 ORDER BY serverhandles.name");
		else
			$query = new QueryIterator("SELECT * FROM serverhandles WHERE single=0 ORDER BY name");
		foreach($query as $v)
		{
			list($count) = sql_fetch("SELECT COUNT(*) FROM serverlookup WHERE handle=$v[id]");
			echo "<tr>";
			echo "<td class='row$row'>"
				.(check_group($v) ? "<a href='admin_servers.php?h=$v[id]&amp;a=d'><img src='img/drop.png' class='icon'></a>" : "<img src='img/blank.png' class='icon'>")
				."<a href='admin_servers.php?h=$v[id]&amp;a=h'><img src='img/copy.png' class='icon'></a>"
				."<a href='admin_servers.php?h=$v[id]&amp;a=e'><img src='img/edit.png' class='icon'></a>"
				." <a href='admin_servers.php?h=$v[id]'>$v[name]</a>";
			echo "<td class='row$row'>$count";
			echo "\n";
			$row = $row ? 0 : 1;
		}
		if(!$query->num_rows())
			echo "<tr><td colspan=2 class='row$row nodata'>".($id ? 'Not in any' : 'No')." server groups\n";
	}
	echo "</table>\n";
}

if(@(int)$_GET['i'])
	$id = (int)$_GET['i'];
else if(@(int)$_POST['id'])
	$id = (int)$_POST['id'];
else
	$id = false;

if($id)
	list($handle) = sql_fetch("SELECT handle FROM servers WHERE id=$id");
else if(@(int)$_GET['h'])
	$handle = (int)$_GET['h'];
else if(@(int)$_POST['handle'])
	$handle = (int)$_POST['handle'];
else
	$handle = false;

if(@$_POST['delete'])
{
	if($id && !check_server($id) || $handle && !check_group($handle))
		$template_errorbox[] = "This server ".($server ? '' : 'group ')."still has Miners associated with it.<br>They must be deleted first.";
	else if($id)
	{
		sql_query("DELETE serverhandles FROM serverhandles, servers WHERE serverhandles.id=servers.handle AND servers.id=$id;\n"
				. "DELETE serverlookup FROM serverlookup, servers WHERE serverlookup.handle=servers.handle AND servers.id=$id;\n"
				. "DELETE FROM servers WHERE id=$id;\n");
		$template_infobox[] = "The server \"$_POST[name]\" was successfully deleted.<br>";
		$id = false;
	}
	else if($handle)
	{
		sql_query("DELETE FROM serverhandles WHERE id=$handle;\n"
				. "DELETE FROM serverlookup WHERE handle=$handle;\n");
		$template_infobox[] = "The server group \"$_POST[name]\" was successfully deleted.<br>";
		$handle = false;
	}
}
else if(@$_POST['editserver'] && $id)
{
	if(!@$_POST['name'])
		$_POST['name'] = $_POST['address'];
	sql_query("UPDATE servers SET "
		."name=".sql_string($_POST['name']).", "
		."loginaddr=".sql_string($_POST['address']).", "
		."loginuser=".sql_string($_POST['user']).", "
		."loginpass=".sql_string($_POST['password']).", "
		."serverdb=".sql_string($_POST['database'])
		." WHERE id=$id");
	sql_query("UPDATE serverhandles SET name=".sql_string($_POST['name']).", hide=".(@$_POST['hide'] ? 1 : 0)." WHERE id=$handle");
	$template_infobox[] = "The server \"$_POST[name]\" was successfully updated.";
}
else if(@$_POST['editgroup'] && $handle)
{
	sql_query("UPDATE serverhandles SET "
		."name=".sql_string($_POST['name']).", "
		."hide=".(@$_POST['hide'] ? 1 : 0)
		." WHERE id=$handle");
	$template_infobox[] = "The server group \"$_POST[name]\" was successfully updated.";
}
else if(@$_POST['newserver'])
{
	if(sql_db_connect($_POST['address'],$_POST['user'],$_POST['password'],$_POST['database']))
	{
		$name = sql_string($_POST['name']);
		$query = "DECLARE @handle AS int;\n";
		$query .= "INSERT INTO serverhandles (name,single,hide) VALUES ($name,1,0);\n";
		$query .= "SET @handle=SCOPE_IDENTITY();\n";
		$query .= "INSERT INTO servers (handle,name,loginaddr,loginuser,loginpass,serverdb) VALUES ("
			."@handle, "
			."$name, "
			.sql_string($_POST['address']).", "
			.sql_string($_POST['user']).", "
			.sql_string($_POST['password']).", "
			.sql_string($_POST['database']).");\n";
		$query .= "INSERT INTO serverlookup (handle,server) VALUES (@handle,SCOPE_IDENTITY());\n";
		sql_query($query);
		$template_infobox[] = "The server \"$_POST[name]\" was successfully added.";
	}
	else
		$template_errorbox[] = "Could not connect to the server! Not adding.<br><span class='description'>$_POST[user]:$_POST[password]@$_POST[address]/$_POST[database]</span>";
}
else if(@$_POST['newgroup'])
{
	if(@count($_POST['servers']) < 2)
		$template_errorbox[] = "You must specify at least two servers for a server group.";
	else
	{
		$name = sql_string($_POST['name']);
		sql_query("INSERT INTO serverhandles (name,single,hide) VALUES (".sql_string($_POST['name']).",0,0)");
		list($handle) = sql_fetch("SELECT SCOPE_IDENTITY()");
		foreach($_POST['servers'] as $v)
			sql_query("INSERT INTO serverlookup (handle,server) VALUES ($handle,".(int)$v.")");
		$template_infobox[] = "The server group \"$_POST[name]\" was successfully added.";
		$handle = false;
	}
}


$server = $id ? sql_fetch("SELECT * FROM servers WHERE id=$id") : false;
$group = $handle ? sql_fetch("SELECT * FROM serverhandles WHERE id=$handle") : false;

$template_title = 'Admin: '.($server ? "Server: $server[name]" : ($group ? "Server Group: $group[name]" : 'Servers'));
$template_admin_menu = true;
include('template_top.php');

if(($server || $group) && @$_GET['a'] == 'd')
{
	if($server)
	{
		$good = check_server($server);
		$tmp = $server;
	}
	else
	{
		$good = check_group($group);
		$tmp = $group;
	}

	if($good)
	{
		echo "<form class='warnbox' action='admin_servers.php' method=post>\n";
		echo "<input type=hidden name='".($server ? 'id' : 'handle')."' value='$tmp[id]'>\n";
		echo "<input type=hidden name='name' value='".htmlspecialchars($tmp['name'],ENT_QUOTES)."'>\n";
		echo "You are about to delete this server".($server ? ', which will alter further data collection on its groups' : 'server group').".<br>\n";
		echo "Are you sure you want to do this?<br>\n";
		echo "<div class='formrowlong'>"
			."<input type=submit class='butdanger' name='delete' value='Delete Server".($server ? ', Alter Groups' : ' Group')."'> "
			."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
			."</div>\n";
		echo "</form>\n";
	}
	else
		$template_errorbox[] = "This server ".($server ? '' : 'group ')."still has Miners associated with it.<br>They must be deleted first.";
}
else if(($server && @$_GET['a'] == 'e') || @$_GET['a'] == 'n')
{
	$edit = @$_GET['a'] == 'e';
	if($server)
	{
		$name = htmlspecialchars($server['name'],ENT_QUOTES);
		$address = htmlspecialchars($server['loginaddr'],ENT_QUOTES);
		$user = htmlspecialchars($server['loginuser'],ENT_QUOTES);
		$password = htmlspecialchars($server['loginpass'],ENT_QUOTES);
		$database = htmlspecialchars($server['serverdb'],ENT_QUOTES);
		$hide = $group['hide'];
	}
	else
	{
		$name = $address = $user = $password = $database = '';
	}

	echo "<form class='".($edit ? 'editbox' : 'newbox')."' action='admin_servers.php' method=post>\n";
	if($edit)
		echo "<input type=hidden name='id' value='$server[id]'>\n";
	echo "<div class='formrow'><div class='label'>Address</div><input type=text name='address' value='$address'></div>\n";
	echo "<div class='formrow'><div class='label'>User</div><input type=text name='user' value='$user'></div>\n";
	echo "<div class='formrow'><div class='label'>Password</div><input type=text name='password' value='$password'></div>\n";
	echo "<div class='formrow'><div class='label'>Database</div><input type=text name='database' value='$database'></div>\n";
	echo "<div class='formrow'><div class='label'>Name <span class='description'>(default: address)</span></div><input type=text name='name' value='$name'></div>\n";
	if($edit)
		echo "<div class='formrow label'>Hidden <input type=checkbox name='hide' ".($hide ? ' checked' : '')."></div>\n";
	echo "<div class='formrowlong'>"
		.($edit
		?"<input type=submit class='butwarning' name='editserver' value='Update Server'> "
		:"<input type=submit class='butnew' name='newserver' value='Add Server'> ")
		."<input type=reset  class='butcancel' value='Reset'> "
		."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
		."</div>\n";
	echo "</form>\n";
}
else if(($handle && @$_GET['a'] == 'e') || @$_GET['a'] == 'h')
{
	$edit = @$_GET['a'] == 'e';
	if($handle)
	{
		$name = htmlspecialchars($group['name'],ENT_QUOTES);
		$hide = $group['hide'];
	}
	else
	{
		$name = '';
	}

	echo "<form class='".($edit ? 'editbox' : 'newbox')."' action='admin_servers.php' method=post>\n";
	if($edit)
		echo "<input type=hidden name='handle' value='$group[id]'>\n";
	echo "<div class='formrow'><div class='label'>Name</div><input type=text name='name' value='$name'></div>\n";
	if(!$edit)
	{
		$selected = array();
		if($handle)
			foreach(new QueryIterator("SELECT * FROM serverlookup WHERE handle=$handle") as $v)
				$selected[] = $v['server'];

		$servers = array();
		foreach(new QueryIterator("SELECT * FROM servers ORDER BY name") as $k => $v)
			$servers[$k] = $v['name'];

		echo "<div class='formrow'><div class='label'>Servers <span class='description'>(Select at least 2)</span></div><select name='servers[]' multiple='yes' size='".count($servers)."'>";
		foreach($servers as $k => $v)
			echo "<option value=$k".(in_array($k,$selected) ? ' selected' : '').">$v";
		echo "</select></div>";
	}
	if($edit)
		echo "<div class='formrow label'>Hidden <input type=checkbox name='hide' ".($hide ? ' checked' : '')."></div>\n";
	echo "<div class='formrowlong'>"
		.($edit
		?"<input type=submit class='butwarning' name='editgroup' value='Update Server Group'> "
		:"<input type=submit class='butnew' name='newgroup' value='Add Server Group'> ")
		."<input type=reset  class='butcancel' value='Reset'> "
		."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
		."</div>\n";
	echo "</form>\n";
}

if($server)
{
	echo "<span class='title'>$server[name]</span>".($server['name']!=$server['loginaddr'] ? " <span class='description'>($server[loginaddr])</span>" : '')."<br>\n";
	echo "<br>\n";
	echo "<br>\n";
	table_servers(false,$server['id']);
	echo "<br>\n";
	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?i=$server[id]&amp;a=e'><img src='img/edit.png' class='icon'> Edit this Server</a>\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?i=$server[id]&amp;a=n'><img src='img/copy.png' class='icon'> Copy this Server</a>\n";
	if(check_server($server))
		echo "<tr><td class='rowf'><a href='admin_servers.php?i=$server[id]&amp;a=d'><img src='img/drop.png' class='icon'> Delete this Server</a>\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php'><img src='img/server.png' class='icon'> Go to the Server List</a>\n";
	echo "</table>\n";
}
else if($group)
{
	echo "<span class='title'>$group[name]</span><br>\n";
	if($group['hide'])
		echo "<span class='description'>hidden</span>";
	echo "<br>\n";
	echo "<br>\n";
	table_servers($group['id']);
	echo "<br>\n";
	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?h=$group[id]&amp;a=e'><img src='img/edit.png' class='icon'> Edit this Server Group</a>\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?h=$group[id]&amp;a=h'><img src='img/copy.png' class='icon'> Copy this Server Group</a>\n";
	if(check_group($group))
		echo "<tr><td class='rowf'><a href='admin_servers.php?h=$group[id]&amp;a=d'><img src='img/drop.png' class='icon'> Delete this Server Group</a>\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php'><img src='img/server.png' class='icon'> Go to the Server List</a>\n";
	echo "</table>\n";
}
else
{
	table_servers(false,false,(@$_GET['t']?$_GET['t']:false));
	echo "<br>\n";
	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?t=1'><img src='img/server.png' class='icon'> Show Link Status</a>\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?a=n'><img src='img/new.png' class='icon'> Add a new Server</a>\n";
	echo "<tr><td class='rowf'><a href='admin_servers.php?a=h'><img src='img/new.png' class='icon'> Add a new Server Group</a>\n";
	echo "</table>\n";
}

include('template_bottom.php');
?>