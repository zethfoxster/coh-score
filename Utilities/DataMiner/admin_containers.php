<?php
require_once('common_sql.php');
require_once('utils.php');

function select_defs($name = '', $selected = '')
{
	$r = "<select name='$name'>";
	$r .= "<option value='0'>None";
	foreach(new QueryIterator("SELECT DISTINCT type FROM defs ORDER BY type") as $v)
		$r .= "<option value=\"".htmlspecialchars($v['type'])."\"".($v['type'] == $selected ? ' selected' : '').">$v[type]";
	$r .= "</select>";
	return $r;
}

$container = $table = false;

if(@$_GET['c'])
	$container = $_GET['c'];
else if(@$_POST['container'])
	$container = $_POST['container'];

if($container)
{
	if(@$_GET['t'])
		$table = $_GET['t'];
	else if(@$_POST['table'])
		$table = $_POST['table'];
}


if(@$_POST['edit'] && $table)
{
	if(@$_FILES['attributefile']['name'])
	{
		$attributefile = $_FILES['attributefile'];
		$attributes = array();

		foreach(file($attributefile['tmp_name']) as $v)
			if(preg_match('|^(\\d+) "(.*)"|',$v,$m)) // TODO: this may need to be unescaped
				$attributes[(int)$m[1]] = $m[2];

		unlink($attributefile['tmp_name']);
	}
	else
		$attributes = false;

	foreach(new QueryIterator("SELECT * FROM containers WHERE container=".sql_string($container)." AND tablename LIKE ".sql_string("$table%")) as $k => $v)
	{
		if($attributes && preg_match('|(\\d+)$|',$v['columnname'],$m) && @$attributes[(int)$m[1]]) // TODO: actual column may depend on the table number
			$name = sql_string($attributes[(int)$m[1]]);
		else if(@$_POST['name'][$k] && @$_POST['name'][$k] != 'mineacc')
			$name = sql_string($_POST['name'][$k]);
		else
			$name = 'columnname';

		if($v['tablename'] == 'mineacc' && $v['columnname'] == '.Level')
			$groupok = @$_POST['parselevel'] ? 1 : 0;
		else
			$groupok = @$_POST['groupok'][$k] ? 1 : 0;
		$dataok = (@$_POST['dataok'][$k]) ? 1 : 0;
		$whereok = @$_POST['whereok'][$k] ? 1 : 0;
		$isDbid = @$_POST['isDbid'][$k] ? 1 : 0;
		$isColor = @$_POST['isColor'][$k] ? 1 : 0;

		sql_query("UPDATE containers SET name=$name, groupok=$groupok, dataok=$dataok, whereok=$whereok, isDbid=$isDbid, isColor=$isColor WHERE id=$k");
	}

	$template_infobox[] = "The $container Container was successfully adjusted.";
}
else if($container && @$_POST['type'] == 'template')
{
	if(@$_FILES['containerfile']['name'])
	{
		$containerfile = $_FILES['containerfile'];

		if(strtolower($container).'.template' != strtolower($containerfile['name']))
			$template_errorbox[] = ".template file <span class='description'>$containerfile[name]</span> has an invalid file name for container $container!";
		else
		{
			// get a list of existing information, to see if we need to delete any columns
			// don't just delete everything and start over, because we want to keep user options
			$oldcontainer = array();
			foreach(new QueryIterator("SELECT * FROM containers WHERE container=".sql_string($container)) as $v)
				$oldcontainer["$v[tablename].$v[columnname]"] = $v;

			// parse the incoming template
			foreach(file($containerfile['tmp_name']) as $v)
			{
				// tablename[mapping].columnname storage
				$v = trim($v);

				if(preg_match('|^(\\w+)\\[(\\d+)\\]\\.(.*)$|',$v,$m))
				{
					$tablename = $m[1];
					$mapping = $m[2];
					if(!$mapping) // Hack for an old template error in Ents
						$mapping = 1;
					$v = $m[3];
				}
				else
				{
					$tablename = $container;
					$mapping = 1;
				}

				if(preg_match('|^(\\w+) (.*?)(?: Indexed)?$|',$v,$m))
				{
					$column = $m[1];

					if($m[2] == '"Attribute"')
						$type = 'attribute';
					else if($m[2] == '"DateTime"')
						$type = 'date';
					else if(strchr($m[2],'"') !== false)
						$type = 'string';
					else if(preg_match('|^\\d+$|',$m[2]))
						$type = 'int';
					else if(preg_match('|^\\d+\\.\\d+$|',$m[2]))
						$type = 'float';
					else
						die("Unknown type: $m[2]");
				}
				else
					die("Unable to parse line:<br>\n$v");
				
				if($type)
				{
					if(@$oldcontainer["$tablename.$column"])
					{
						sql_query("UPDATE containers SET tablemapping=$mapping, columntype='$type' WHERE id=".$oldcontainer["$tablename.$column"]['id']);
						unset($oldcontainer["$tablename.$column"]); // handled this column, don't delete it
					}
					else // new column
						sql_query("INSERT INTO containers (name,container,tablename,tablemapping,columnname,columntype) VALUES ('$column',".sql_string($container).",'$tablename',$mapping,'$column','$type')");
				}
			}
			unlink($containerfile['tmp_name']);

			// delete any existing columns that weren't handled
			$deleted = '';
			foreach($oldcontainer as $k => $v)
			{
				$deleted .= "$k<br>";
				sql_query("DELETE FROM containers WHERE id=$v[id]");
			}

			$template_infobox[] = "$container successfully updated or created with $container.template.".($deleted ? "<div class='description'>Deleted Columns:<br>$deleted</div>" : '');
		}
	}
	if(@$_FILES['containerschema']['name'])
	{
		$containerschema = $_FILES['containerschema'];

		if(strtolower($container).'.schema.html' != strtolower($containerschema['name']))
			$template_errorbox[] = ".schema.html file <span class='description'>$containerschema[name]</span> has an invalid file name for container $container!";
		else
		{
			// parse the incoming schema
			foreach(file($containerschema['tmp_name']) as $v)
			{
				if(preg_match("|<tr bgcolor='#eeeeee'><td valign=top>(.*?)</td>.*(.*)|", $v, $m))
				{
					$tablename = sql_string($m[1]);
					$v = $m[2];
				}
				if(preg_match('|<tr><td>(.+?)</td><td>.*?</td><td>(.*?)</td>|',$v,$m))
				{
					$columnname = sql_string($m[1]);
					$description = sql_string($m[2]);
					sql_query("UPDATE containers SET description=$description WHERE tablename=$tablename AND columnname=$columnname");
				}
			}
			unlink($containerschema['tmp_name']);
			$template_infobox[] = "$container successfully updated with $container.schema.html.";
		}
	}
}
else if($container && @$_POST['type'] == 'mineacc')
{
	// get a list of existing information
	// unused fields will have 'dataok' cleared

	$oldcontainer = array();
	foreach(new QueryIterator("SELECT * FROM containers WHERE tablename='mineacc' AND container=".sql_string($container)) as $v)
		$oldcontainer[$v['columnname']] = $v;

	// get what's currently on all the servers
	$newcontainer = array();
	foreach(new QueryIterator("SELECT servers.* FROM serverhandles, serverlookup, servers WHERE hide=0 AND serverlookup.handle=serverhandles.id AND serverlookup.server=servers.id") as $server)
		foreach(new QueryIterator("SELECT DISTINCT MinedData.Field FROM MiningAccumulator, MinedData WHERE MiningAccumulator.Name LIKE ".sql_string("$container.%")." AND MiningAccumulator.".sql_string($config_container_id)." = MinedData.".sql_string($config_container_id)." ORDER BY MinedData.Field",sql_server_connect($server,true)) as $v)
			$newcontainer[$v['Field']] = true;

	if(!count($oldcontainer))
	{
		sql_query("INSERT INTO containers (name,groupok,container,tablename,tablemapping,columnname,columntype) VALUES ('Name',1,".sql_string($container).",'mineacc',1,'Name','string')");
		sql_query("INSERT INTO containers (name,container,tablename,tablemapping,columnname,columntype) VALUES ('Level',".sql_string($container).",'mineacc',1,'.Level','string')");
	}

	foreach($oldcontainer as $k => $v)
		if($k != 'Name' && $k != '.Level' && !@$newcontainer[$k])
			sql_query("UPDATE containers SET dataok=0, groupok=0 WHERE tablename='mineacc' AND id=$v[id]");

	foreach($newcontainer as $k => $v)
		if(!@$oldcontainer[$k])
			sql_query("INSERT INTO containers (name,dataok,container,tablename,tablemapping,columnname,columntype) VALUES (".sql_string($k).",1,".sql_string($container).",'mineacc',1,".sql_string($k).",'int')");

	// update/change local data association
	$olddef = @$_POST['olddef'] ? $_POST['olddef'] : 0;
	$def = $_POST['def'];

	if($olddef != $def)
		sql_query("DELETE FROM containers WHERE tablename<>'mineacc' AND container=".sql_string($container));

	$oldcontainer = array();
	foreach(new QueryIterator("SELECT * FROM containers WHERE tablename<>'mineacc' AND container=".sql_string($container)) as $v)
		$oldcontainer[$v['columnname']] = $v;

	$newcontainer = array();
	foreach(new QueryIterator("SELECT defdata.field FROM defs, defdata WHERE defs.id=defdata.def AND defs.type=".sql_string($def)) as $v)
		$newcontainer[$v['field']] = true;

	foreach($oldcontainer as $k => $v)
		if(!@$newcontainer[$k])
			sql_query("UPDATE containers SET dataok=0, groupok=0 WHERE id=$v[id]");

	foreach($newcontainer as $k => $v)
		if(!@$oldcontainer[$k])
			sql_query("INSERT INTO containers (name,container,tablename,tablemapping,columnname,columntype) VALUES (".sql_string($k).",".sql_string($container).",".sql_string("mineacc$def").",9999,".sql_string($k).",'int')");

	$template_infobox[] = "$container successfully updated or created with all registered servers.";
	$table = 'mineacc';
}

$template_title = 'Admin: Container'.($container ? ": $container" : 's');
$template_admin_menu = true;
include('template_top.php');

if($table)
{
	echo "<form method=post enctype='multipart/form-data'>\n";
	echo "<input type=hidden name='MAX_FILE_SIZE' value='".bytes_shorthand_to_int(ini_get('upload_max_filesize'))."'>\n";
	echo "<input type=hidden name='container' value='$container'>\n";
	if($table != 'mineacc')
		echo "<input type=hidden name='table' value='$table'>\n";
	else
		echo "<input type=hidden name='type' value='mineacc'>\n";

	$tablename = '';
	$row = 0;
	$def = 0;
	$parselevel = false;
	echo "<table>\n";
	echo "<tr><th class='rowt' colspan=8>$container\n";
	foreach(new QueryIterator("SELECT * FROM containers WHERE container=".sql_string($container)." AND tablename LIKE ".sql_string("$table%")." ORDER BY tablename") as $v) // Form handling should match this query
	{
		if($tablename == 'mineacc' && $v['columnname'] == '.Level')
		{
			$parselevel = $v['groupok'];
			continue;
		}
		if($v['tablename'] != $tablename)
		{
			$tablename = $v['tablename'];
			if($table != 'mineacc')
				echo "<tr><th class='rowh' colspan=8>$tablename\n";
			else if(substr($tablename,7))
				$def = substr($tablename,7);
			echo "<tr><th class='rowh'>Display Name<th class='rowh'>Column Name<th class='rowh'>Type<th class='rowh'>Data<th class='rowh'>Group<th class='rowh'>Where<th class='rowh'>Is DBID<th class='rowh'>Is Color\n";
		}
		echo "<tr>";
		echo "<td class='row$row'><input type=text name='name[$v[id]]' value='".htmlspecialchars($v['name'],ENT_QUOTES)."'>";
		echo "<td class='row$row'>$v[columnname]";
		echo "<td class='row$row description'>$v[columntype]";
		if( $v['columntype'] == 'string' || ($table == 'mineacc' && $tablename != 'mineacc'))
			echo "<td class='row$row'>&nbsp;";
		else
			echo "<td class='row$row'><input type=checkbox name='dataok[$v[id]]'".($v['dataok']?' checked':'')."><span class='description'></span>";
		if($tablename == 'mineacc' && $v['columnname'] != 'Name')
			echo "<td class='row$row'>&nbsp;";
		else
			echo "<td class='row$row'><input type=checkbox name='groupok[$v[id]]'".($v['groupok']?' checked':'')."><span class='description'></span>";
		if($table == 'mineacc') // FIXME: constraints for mineaccs
			echo "<td class='row$row'>&nbsp;";
		else
			echo "<td class='row$row'><input type=checkbox name='whereok[$v[id]]'".($v['whereok']?' checked':'')."><span class='description'></span>";
			
		if($table == 'mineacc') // FIXME: constraints for mineaccs
			echo "<td class='row$row'>&nbsp;";
		else
			echo "<td class='row$row'><input type=checkbox name='isDbid[$v[id]]'".($v['isDbid']?' checked':'')."><span class='description'></span>";

		if($table == 'mineacc') // FIXME: constraints for mineaccs
			echo "<td class='row$row'>&nbsp;";
		else
			echo "<td class='row$row'><input type=checkbox name='isColor[$v[id]]'".($v['isColor']?' checked':'')."><span class='description'></span>";

		echo "\n";
		if(trim($v['description']))
			echo "<tr><td colspan=8 class='row$row description' style='font-style:italic'>$v[description]\n";
		$row = $row ? 0 : 1;
	}
	echo "<tr><td class='rowf' colspan=8>\n";
	echo "<div class='formrow' align=center><input type=submit class='butwarning' name='edit' value='Save Changes'></div>\n";
	
	if($table == 'mineacc')
		echo "    <div class='formrow label'><input type=checkbox name='parselevel'".($parselevel ? ' checked' : '').">Parse Level From Name</div>\n";
	else
		echo "    <div class='formrow'><div class='label'>Name Columns with .attribute File</div><input type=file name='attributefile'></div>\n";
	
	if($table == 'mineacc')
	{
		echo "<tr><td class='rowf' colspan=8><input type=hidden name='olddef' value=\"".htmlspecialchars($def)."\">\n";
		echo "    <div class='formrow'><div class='label'>Local Data Association</div>".select_defs('def',$def)."</div>\n";
		echo "    <div class='formrow'><input type=submit class='butwarning' value='Update Container Fields From All Servers And Local Data'></div>\n";
	}
	echo "</table>\n";
	echo "<br>\n";
	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	if($table != 'mineacc')
		echo "<tr><td class='rowf'><a href='admin_containers.php?c=$container'><img src='img/back.png' class='icon'> Go Back to $container</a>\n";
	echo "<tr><td class='rowf'><a href='admin_containers.php'><img src='img/container.png' class='icon'> Go to the Container List</a>\n";
	echo "</table>\n";

	echo "</form>\n";
}
else if($container)
{
	$tables = array();
	foreach(new QueryIterator("SELECT * FROM containers WHERE container=".sql_string($container)) as $v)
	{
		$tablename = substr($v['tablename'],0,strcspn($v['tablename'],'0123456789'));
		if(!@$tables[$tablename])
		{
			$tables[$tablename] = array(
				'tables' => array(),
				'columns' => 0,
				'tablemapping' => $v['tablemapping'],
			);
		}
		
		if($tables[$tablename]['tablemapping'] == $v['tablemapping'])
		{
			$tables[$tablename]['tables'][$v['tablename']] = 1;
			$tables[$tablename]['columns']++;
		}
		else
			die("Tablename/Mapping Mismatch: ".$tablename."[".$tables[$tablename]['tablemapping']."] and ".$v['tablename']."[".$v['tablemapping']."]\n");
	}

	echo "<form method=post enctype='multipart/form-data'>\n";
	echo "<input type=hidden name='MAX_FILE_SIZE' value='".bytes_shorthand_to_int(ini_get('upload_max_filesize'))."'>\n";
	echo "<input type=hidden name='container' value='$container'>\n";
	echo "<input type=hidden name='type' value='template'>\n";

	$row = 0;
	echo "<table>\n";
	echo "<tr><th class='rowt' colspan=4>$container\n";
	echo "<tr><th class='rowh'>Name<th class='rowh'>Mapping<th class='rowh'>Tables<th class='rowh'>Columns\n";
	foreach($tables as $k => $v)
	{
		echo "<tr>";
		echo "<td class='row$row'><a href='?c=$container&amp;t=$k'>$k";
		echo "<td class='row$row description' style='text-align:right'>".($v['tablemapping']==9999 ? 'n' : $v['tablemapping'])."-to-1";
		echo "<td class='row$row description'>".count($v['tables']);
		echo "<td class='row$row description'>$v[columns]";
		echo "\n";
		$row = $row ? 0 : 1;
	}
	echo "<tr><td class='rowf' colspan=4>\n";
	echo "    <div class='formrow'><div class='label'>$container.template File</div><input type=file name='containerfile'></div>\n";
	echo "    <div class='formrow'><div class='label'>$container.schema.html File</div><input type=file name='containerschema'></div>\n";
	echo "    <div class='formrow'><input type=submit class='butwarning' name='edit' value='Upload New $container.template/schema.html File(s)'></div>\n";
	echo "</table>\n";

	echo "</form>\n";

	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	echo "<tr><td class='rowf'><a href='admin_containers.php'><img src='img/container.png' class='icon'> Go to the Container List</a>\n";
	echo "</table>\n";

}
else
{
	echo "<form method=post enctype='multipart/form-data'>\n";
	echo "<input type=hidden name='MAX_FILE_SIZE' value='".bytes_shorthand_to_int(ini_get('upload_max_filesize'))."'>\n";

	$row = 0;
	echo "<table>\n";
	echo "<tr><th class='rowt'>Containers\n";
	echo "<tr><th class='rowh'>Templated\n";
	foreach(new QueryIterator("SELECT DISTINCT container FROM containers WHERE tablename NOT LIKE 'mineacc%'") as $v)
	{
		echo "<tr><td class='row$row'><a href='admin_containers.php?c=$v[0]'><img src='img/edit.png' class='icon'> $v[0]";
		$row = $row ? 0 : 1;
	}
	echo "<tr><th class='rowh'>Mining Accumulators\n";
	foreach(new QueryIterator("SELECT DISTINCT container FROM containers WHERE tablename='mineacc'") as $v)
	{
		echo "<tr><td class='row$row'><a href='admin_containers.php?c=$v[0]&amp;t=mineacc'><img src='img/edit.png' class='icon'> $v[0]";
		$row = $row ? 0 : 1;
	}
	echo "<tr><td class='rowf' colspan=4>\n";
	echo "    <div class='formrow'><div class='label'>Container Name</div><input type=text name='container'></div>\n";
	echo "    <div class='formrow'><div class='label'>Container Type</div><select name='type' onChange='var template=this.options[this.selectedIndex].value==\"template\";getElementById(\"templateline\").style.display=template?\"block\":\"none\";getElementById(\"mineaccline\").style.display=template?\"none\":\"block\";'><option value='template'>Templated<option value='mineacc'>Mining Accumulator</select></div>\n";
	echo "    <div class='formrow' id='templateline'><div class='label'>.template File</div><input type=file name='containerfile'></div>\n";
	echo "    <div class='formrow' id='mineaccline'><div class='label'>Local Data Association</div>".select_defs('def')."</div>\n";
	echo "    <div class='formrow'><input type=submit class='butnew' value='Create New Container'></div>\n";
	echo "</table>\n";

	echo "</form>\n";
}

include('template_bottom.php');
?>