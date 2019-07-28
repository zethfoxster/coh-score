

<?php
require_once('common_tables.php');
require_once('common_forms.php');
require_once('utils_sql.php');

if(@$_GET['a'] == 'first')
{
	$template_infobox[] = "This page allows you to look up detailed information on specific players, accounts and supergroups.";
}

include('template_top.php');
?>

<div id='singleton_table'>
	<form action='singletonqueries.php' method=post>
	<table align=left width=100%>
		<tr>
			<td width=300></td><td width=100></td>
		<tr>
			<th class='rowh' colspan=2>
			<div align=left>Find all characters by auth name</div>
			</th>
		</tr>
		<tr>
			<td class='rowf' width=400>
				<div class='formrow'>
				<div class='label'>Auth Name:</div>
				<input type=text name='auth' value=''></div>
			</td>
			<td width=100 class='rowf'><input type=submit class='butnew' name='authname' value='Search'></td>
		</tr>
		<tr>
			<th class='rowh' colspan=2>Find all characters by player name</th>
		</tr>
		<tr>
			<td class='rowf'>
			<div class='formrow'>
			<div class='label'>Player Name:</div>
			<input type=text name='player' value=''></div>
			</td>
			<td class='rowf'><input type=submit class='butnew' name='playername'
				value='Search'></td>
		</tr>
		<tr>
			<th class='rowh' colspan=2>Find all supergroups by name</th>
		</tr>
		<tr>
			<td class='rowf'>
			<div class='formrow'>
			<div class='label'>Supergroup Name:</div>
			<input type=text name='super' value=''></div>
			</td>
			<td class='rowf'><input type=submit class='butnew'
				name='supergroupname' value='Search'></td>
		</tr>
	</table>
	</form>
</div>

<?php 
if( @$_GET['container'] && @$_GET['name'] && @$_GET['server'] )
{
	//$template_infobox[] = "Detailed Information on a Single Character";
	$name = $_GET['name'];
	$container = $_GET['container'];
	$servername = $_GET['server'];
	$view = $_GET['viewing'];
	
		
	echo("<table align=left  style='width:1200px'>");
	echo( "<tr><td style='width:200px'></td><td style='width:1000px'></td></tr>");
	echo( "<tr><th class='rowt' colspan=2>Data for ".$name."</th></tr>");	
	echo( "<tr><td class='rowh'>Table</td><td class='rowh'>Results</td></tr>");
	
	$server = sql_fetch("SELECT * FROM servers WHERE name='All Servers'");
	$db = sql_server_connect($server, true);
	$result = sql_fetch("SELECT * FROM .COXSHARDVIEW.dbo.$container WHERE name='$name' AND ShardName='$servername'", $db ); 
	$shardid = $result['ShardId'];
	$id = $result['ContainerId'];
		
	echo( "<tr><td valign=top style='width:10%'>" );
	foreach(new QueryIterator("SELECT DISTINCT tablename FROM containers WHERE container='$container' ORDER BY tablename") as $containers)
	{
		$tablename = $containers['tablename'];
		if( $tablename == "InvRecipeInvention" || $tablename == "AttribMods"  )
			continue;
			
		echo( "<div align=left><a href='singletonqueries.php?name=".$name."&server=".$servername."&container=".$container."&table=".$tablename."'>$tablename</a></div>");	
	}		
	echo( "</td><td valign=top style='width:90%'>");
		
	if( @$_GET['table'] )
	{
		$tablename = $_GET['table'];
		
		echo( "<table align=left width=1000 id='".$tablename."'>");
		echo( "<tr><th style='width:200px'></th><th style='width:400px'></th><th style='width:400px'></th></tr>");
		echo( "<tr><th class='rowt' colspan=3>".$tablename."</th></tr>");
		$query = new QueryIterator("SELECT * FROM .COXSHARDVIEW.dbo.$tablename WHERE ContainerId='$id' AND ShardId='$shardid'", $db);
		$i = 0;
		$count = 0;
	
		foreach($query as $result )
		{
			if($i == 0 && $query->num_rows() > 0 )
			{
				echo( "<tr><th class='rowh' style='width:200px'>Name</th><th class='rowh' style='width:400px'>Value</th><th class='rowh' style='width:400px'>Description</th></tr>");
			}
			if( $query->num_rows() > 1 )
				echo( "<tr><td class='rowh' width=1000 colspan=3>".++$i."</th></tr>" );
			
			foreach(new QueryIterator("SELECT * FROM containers WHERE container='$container' AND tablename=".sql_string("$tablename")." ORDER BY tablename") as $v) // Form handling should match this query
			{
				$displayname = $v['name'];
				$colname = $v['columnname'];
				$type = $v['columntype'];
				$description = $v['description'];
				$value = $result[$colname];
				
				if( $type == "attribute" )
				{
					$attribute = sql_fetch("SELECT DISTINCT Name FROM .COXSHARDVIEW.dbo.Attributes WHERE Id='$value'", $db ); 
					$value = $attribute['Name'];
				}

				if( $value )
				{
					$count++;
					if($v['isDbid'] > 0 )
					{
						$newplayer = sql_fetch("SELECT Name FROM .COXSHARDVIEW.dbo.Ents WHERE ContainerId='$value' AND ShardId='$shardid'", $db );
						if( $newplayer )
						{
							$newname = $newplayer['Name'];
							echo( "<tr><td class='rowf' width=200>".$displayname."</td><td class='rowf' width=400><a href='singletonqueries.php?name=".$newname."&server=".$servername."&container=Ents'>".$newname."</a></td><td class='rowf' width=400>".$description."</td></tr>");	
						}
						else 
							echo( "<tr><td class='rowf' width=200>".$displayname."</a></td><td class='rowf' width=400>Offlined Player:".$value."</td><td class='rowf' width=400>".$description."</td></tr>");
					}
					else
						echo( "<tr><td class='rowf' width=200>".$displayname."</td><td class='rowf' width=400>".$value."</td><td class='rowf' width=400>".$description."</td></tr>");	
					
				}
			}
		}
		
		if( $count == 0 )
			echo( "<tr><td class='rowf' colspan=3><div align=left>No Entries</div></td></tr>");
		echo("</table>");		
	}
	echo("</table>");
}

if(@$_POST['auth'] )
{
	$server = sql_fetch("SELECT * FROM servers WHERE name='All Servers'");
	$db = sql_server_connect($server, true);
	
	$query = new QueryIterator("Select name, ShardName FROM .COXSHARDVIEW.dbo.Ents WHERE authname='$_POST[auth]'", $db );
	
	echo("<table align=left>");
	echo( "<tr><th class='rowt' colspan=2>Characters with the Authname ".$_POST[auth]."</th>");
	echo( "<tr><th class='rowh'>Name</th><th class='rowh'>Server</th></tr>");
	foreach($query as $row)
	{
		echo( "<tr><td class='rowf'><a href='singletonqueries.php?name=".$row['name']."&server=".$row['ShardName']."&container=Ents'>".$row['name']."</a></td><td class='rowf'>".$row['ShardName']."</td></tr>");	
	}
	echo("</table>");
}

if(@$_POST['player'])
{
	$server = sql_fetch("SELECT * FROM servers WHERE name='All Servers'");
	$db = sql_server_connect($server, true);
	
	$query = new QueryIterator("Select AuthName, ShardName FROM .COXSHARDVIEW.dbo.Ents WHERE name='$_POST[player]'", $db );
	
	echo("<table align=left>");
	echo( "<tr><th class='rowt' colspan=3>Characters with the Player Name ".$_POST[player]."</th>");
	echo( "<tr><th class='rowh'>Auth Name</th><th class='rowh'>Server</th></tr>");
	foreach($query as $row)
	{
		echo( "<tr><td class='rowf'><a href='singletonqueries.php?name=".$_POST[player]."&server=".$row['ShardName']."&container=Ents'>".$row['AuthName']."</a></td><td class='rowf'>".$row['ShardName']."</td></tr>");	
	}
	echo("</table>");
}

if(@$_POST['super'])
{
	$server = sql_fetch("SELECT * FROM servers WHERE name='All Servers'");
	$db = sql_server_connect($server, true);
	
	$query = new QueryIterator("Select name, ShardName FROM .COXSHARDVIEW.dbo.Supergroups WHERE Name='$_POST[super]'", $db );
	
	echo("<table align=left>");
	echo( "<tr><th class='rowt' colspan=2>Supergroups with the name: ".$_POST[super]."</th>");
	echo( "<tr><th class='rowh'>Name</th><th class='rowh'>Server</th></tr>");
	foreach($query as $row)
	{
		echo( "<tr><td class='rowf'><a href='singletonqueries.php?name=".$row['name']."&server=".$row['ShardName']."&container=Supergroups'>".$row['name']."</a></td><td class='rowf'>".$row['ShardName']."</td></tr>");	
	}
	echo("</table>");
}

	include('template_bottom.php');
	
?>
