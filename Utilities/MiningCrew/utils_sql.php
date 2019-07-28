<?php
require_once('utils.php');
require_once('common.php');
require_once('common_sql.php');

function create_miner($name, $category, $description, $serverhandle, $container, $timeframe,
						$dataaggs, $dataids, $groupids, $whereids, $wherecmps, $wherevals, $forcecreate)
{
	global $common_aggregation, $common_timeframes, $common_comparison;
	$mineacc = substr($container,-2) == '.%';

	if(!is_array($dataids))
		$dataids = array();
	if(!is_array($groupids))
		$groupids = array();
	if(!is_array($whereids))
		$whereids = array();


	$data = array();
	for($i = 0; $i < count($dataids); $i++)
	{
		if($common_aggregation[$dataaggs[$i]]['data'])
		{
			$data[$i] = sql_fetch("SELECT * FROM containers WHERE id=$dataids[$i]");
			if(!$data[$i])
				die("Error: Aggregation needs a data column, but no column definition was found for data (#$dataids[$i]).");
			if($mineacc  != (substr($data[$i]['tablename'],0,7) == 'mineacc'))
				die("Error: Mixed container types.");
		}
		else
			$data[$i] = false;
	}

	$groups = array();
	foreach($groupids as $groupid)
	{
		$group = sql_fetch("SELECT * FROM containers WHERE id=$groupid");
		if(!$group)
			die("Error: No column definition was found for grouping (#$groupid).");
		if($mineacc != (substr($group['tablename'],0,7) == 'mineacc'))
			die("Error: Mixed container types.");
		$groups[] = $group;
	}

	$wheres = array();
	foreach($whereids as $whereid)
	{
		$wherecol = sql_fetch("SELECT * FROM containers WHERE id=$whereid");
		if(!$wherecol)
			die("Error: No column definition was found for constraint (#$whereid).");
		if($mineacc != (substr($wherecol['tablename'],0,7) == 'mineacc'))
			die("Error: Mixed container types.");
		$wheres[] = $wherecol;
	}

	$select = array();
	$from = array();
	$leftjoin = array();
	$where = array();
	$groupby = array();

	$dataheaders = array();
	$groupheaders = array();
	$whereheaders = array();

	$grouplocally = 0;
	$groupdefids = '';

	if($mineacc)
	{
		$select[] = 'MiningAccumulator.Name';
		$from[] = 'MiningAccumulator';
		$where[] = "MiningAccumulator.Name LIKE ".sql_string( $container);
		$md = 0;
		for($i = 0; $i < count($data); $i++)
		{
			$datum = $data[$i];
			if($datum)
			{
				$select[] = "md$md.".$common_timeframes[$timeframe]['col'];
				// TODO: make leftjoin more automatic
				$leftjoin[] = "LEFT OUTER JOIN MinedData as md$md ON md$md.ContainerId = MiningAccumulator.ContainerId AND md$md.Field = ".sql_string($datum['columnname']);
//				$from[] = "MinedData as md$md";
//				$where[] = "md$md.ContainerId = MiningAccumulator.ContainerId";
//				$where[] = "md$md.Field = ".sql_string($datum['columnname']);
				$md++;
			}

			$dataheaders[] = $common_aggregation[$dataaggs[$i]]['disp'].($datum ? " $datum[name]" : '');
		}
		$grouplocally = 1;
		foreach($groups as $group)
			$groupheaders[] = $group['name'];
		$groupdefids = join("\n",$groupids);
		// FIXME: constraints
	}
	else
	{
		$multitables = array();
		$from[] = $container; // CAVEAT: assumes there's a 1-to-1 table of the same name as container
		for($i = 0; $i < count($data); $i++)
		{
			$datum = $data[$i];
			if($datum)
			{
				if($datum['columntype'] == 'int')
					$data_cast = "CAST($datum[tablename].$datum[columnname] as bigint)";
				else // $datum['columntype'] == 'float'
					$data_cast = "$datum[tablename].$datum[columnname]";

				$select[] = str_replace('$',$data_cast,$common_aggregation[$dataaggs[$i]]['func']);

				if($datum['tablemapping'] > 1)
					$multitables[$datum['tablename']][] = $datum['columnname'];
				else
					$from[] = "$datum[tablename]";

				if($datum['tablename'] != $container)
					$where[] = "$container.ContainerId = $datum[tablename].ContainerId";
			}
			else
				$select[] = $common_aggregation[$dataaggs[$i]]['func'];

			$dataheaders[] = $common_aggregation[$dataaggs[$i]]['disp'].($datum ? " $datum[name]" : '');
		}

		$attr = 0;
		foreach($groups as $group)
		{
			if($group['tablemapping'] > 1)
				$multitables[$group['tablename']][] = $group['columnname'];
			else
				$from[] = "$group[tablename]";

			if($group['tablename'] != $container)
				$where[] = "$group[tablename].ContainerId = $container.ContainerId";

			if($group['columntype'] == 'attribute')
			{
				$from[] = "Attributes as attr$attr";
				$where[] = "attr$attr.Id=$group[tablename].$group[columnname]";
				$groupby[] = "attr$attr.Name";
				$attr++;

				// HACK: total hack to automatically hide certain powers
				if($container == 'Ents' && $group['columnname'] == 'PowerName')
				{
					if($group['tablemapping'] > 1)
						$multitables[$group['tablename']][] = 'CategoryName';

					$where[] = 'Powers.CategoryName <> 30'; // Inherent
					$where[] = 'Powers.CategoryName <> 33'; // Temporary_Powers
					$where[] = 'Powers.CategoryName <> 11682'; // Items_Of_Power
				}
			}
			else
				$groupby[] = "$group[tablename].$group[columnname]";

			$groupheaders[] = $group['name'];
		}

		for($i = 0; $i < count($wheres); $i++)
		{
			$wherecol = $wheres[$i];

			if($wherecol['tablemapping'] > 1)
				$multitables[$wherecol['tablename']][] = $wherecol['columnname'];
			else
				$from[] = "$wherecol[tablename]";
			if($wherecol['tablename'] != $container)
				$where[] = "$wherecol[tablename].ContainerId = $container.ContainerId";

			// FIXME: validate comparison function values
			if($wherecol['columntype'] == 'attribute')
			{
				$from[] = "Attributes as attr$attr";
				$where[] = "attr$attr.Id=$wherecol[tablename].$wherecol[columnname]";
				$where[] = str_replace(array('$0','$1'),
										array("attr$attr.Name",sql_string($wherevals[$i])),
										$common_comparison[$wherecmps[$i]]['func']);
				$attr++;
			}
			else if($wherecol['columntype'] == 'date')
			{
				$where[] = str_replace(array('$0','$1'),
										array("$wherecol[tablename].$wherecol[columnname]",sql_date($wherevals[$i])),
										$common_comparison[$wherecmps[$i]]['func']);
			}
			else
				$where[] = str_replace(array('$0','$1'),
										array("ISNULL($wherecol[tablename].$wherecol[columnname],0)",sql_string($wherevals[$i])),
										$common_comparison[$wherecmps[$i]]['func']);

			$whereheaders[] = str_replace(array('$0','$1'),
											array("$wherecol[tablename].$wherecol[columnname]",$wherevals[$i]),
											$common_comparison[$wherecmps[$i]]['human']);
		}

		foreach($multitables as $k => $v)
		{
			$v = array_unique($v);
			$from[] = "(SELECT DISTINCT ContainerId, ".join(', ',$v)." FROM $k) as $k";
			$where[] = "$k.ContainerId = $container.ContainerId";
		}
	}

	$groupby = array_unique($groupby);
	$select = array_unique(array_merge($groupby,$select));
	$from = array_unique($from);
	$where = array_unique($where);

	$query = "SELECT ".join(', ',$select)
		.(count($from) ? ("\nFROM ".join(', ',$from)) : '')
		.(count($leftjoin) ? ("\n\t".join("\n\t",$leftjoin)) : '')
		.(count($where) ? ("\nWHERE ".join("\n\tAND ",$where)) : '')
		.(count($groupby) ? ("\nGROUP BY ".join(', ',$groupby)) : '')
		.(count($groupby) ? ("\nORDER BY ".join(', ',$groupby)) : '');

	$dataheader = sql_string(join("\n",$dataheaders));
	$groupheader = sql_string(join("\n",$groupheaders));
	$whereheader = sql_string(join("\n",$whereheaders));

	if(!$forcecreate)
	{
		$id = sql_fetch("SELECT id FROM miners WHERE servers=$serverhandle AND query LIKE ".sql_string($query)." AND dataheader LIKE $dataheader AND groupheader LIKE $groupheader"); // FIXME: better checking? =?
		if($id)
			return $id['id'];
	}

	list($qualifier) = sql_fetch("SELECT name FROM serverhandles WHERE id=$serverhandle");
	$qualifier = "$qualifier, "
		.(substr($container,-2) == '.%' ? substr($container,0,-2) : $container)
		." - (".join_humanreadable($dataheaders).")"
		.(count($groupheaders) ? (" by (".join_humanreadable($groupheaders).")") : '')
		.(count($whereheaders) ? (" where (".join_humanreadable($whereheaders).")") : '')
		.($mineacc ? (' '.$common_timeframes[$timeframe]['disp']) : '');

	if(!$name)
		$name = $qualifier;

	if(!$category)
		$category = "Uncategorized";

	sql_query("INSERT INTO miners (name,category,qualifier,description,servers,dataheader,aggregation,grouplocally,groupheader,groupids,whereheader,query) VALUES ("
		.sql_string($name).", "
		.sql_string($category).", "
		.sql_string($qualifier).", "
		.sql_string($description).", "
		."$serverhandle, "
		."$dataheader, "
		.sql_string(join("\n",$dataaggs)).", "
		."$grouplocally, "
		."$groupheader, "
		.sql_string($groupdefids).", "
		."$whereheader, "
		.sql_string($query).")");
	
	list($id) = sql_fetch("SELECT SCOPE_IDENTITY()");
	return $id;
}

// FIXME: how fast is this function with a lot of data?
function collect_data($minerid, $schedule = false)
{
	global $common_aggregation, $config_compressionlevel;

	$miner = sql_fetch("SELECT * FROM miners WHERE id=$minerid");
	$groups = unjoin("\n",trim($miner['groupheader']));
	foreach(unjoin("\n",trim($miner['aggregation'])) as $v)
		$aggregation[] = $common_aggregation[$v];

	$groupdefs = array();
	foreach(unjoin("\n",trim($miner['groupids'])) as $v)
		$groupdefs[] = sql_fetch("SELECT * FROM containers WHERE id=".(int)$v);


	$data = '';
	foreach(unjoin("\n",trim($miner['whereheader'])) as $v)
		$data .= format_csv(array($v));
	if(strlen($data))
		$data .= format_csv(array());

	foreach(new QueryIterator("SELECT servers.* FROM servers, serverlookup WHERE serverlookup.handle=$miner[servers] AND servers.id=serverlookup.server") as $server)
	{
		$data .= format_csv(array($server['name']));
		if(trim($miner['groupheader']) || trim($miner['dataheader']))
			$data .= format_csv(array_merge(unjoin("\n",trim($miner['groupheader'])),unjoin("\n",trim($miner['dataheader']))));
		if($miner['grouplocally'])
		{
			$table = array();
			foreach(new QueryIterator($miner['query'],sql_server_connect($server),MSSQL_NUM) as $v)
			{
				unset($name,$level);
				@list(,$name,$level) = explode('.',array_shift($v)); // TODO : genericize this

				$idxs = array();
				foreach($groupdefs as $groupdef)
				{
					if($groupdef['name'] == 'Name' && $groupdef['tablename'] == 'mineacc')
						$idxs[] = isset($name) ? array($name) : array();
					else if($groupdef['name'] == '.Level' && $groupdef['tablename'] == 'mineacc')
						$idxs[] = isset($level) ? array($level) : array();
					else
					{
						$a = array();
						foreach(new QueryIterator("SELECT defdata.* FROM defs, defdata WHERE defs.type='".substr($groupdef['tablename'],7)."' AND defs.name=".sql_string($name)." AND defdata.def=defs.id AND defdata.field=".sql_string($groupdef['columnname'])) as $def)
							$a[] = $def[$def['datatype'].'data'];
						$idxs[] = $a;
					}
				}

				$keys = array('');
				foreach($idxs as $idx)
				{
					$newkeys = array();
					if(!count($idx))
						$idx = array('UNDEFINED');
					foreach($keys as $key)
						foreach($idx as $idxval)
							$newkeys[] = "$key\n$idxval";
					$keys = $newkeys;
				}

				foreach($keys as $key)
					for($i = 0; $i < count($aggregation); $i++)
						$aggregation[$i]['localin']($table[$key][$i],@$v[$i]); // FIXME: data gaps not handled
			}

			ksort($table);

			foreach($table as $k => $v)
			{
				for($i = 0; $i < count($aggregation); $i++)
				{
					if(!isset($v[$i]))
						$v[$i] = '';
					else
						$v[$i] = $aggregation[$i]['localout']($v[$i]);
				}
				$data .= format_csv(array_merge(unjoin("\n",trim($k)),$v));
			}
		}
		else
			foreach(new QueryIterator($miner['query'],sql_server_connect($server,true),MSSQL_NUM) as $v)
				$data .= format_csv($v);
		$data .= format_csv(array());
	}
	$data = gzcompress($data,$config_compressionlevel);
	$data = unpack("H*hex",$data);

	sql_query("INSERT INTO data (miner,schedule,date,data) VALUES ("
		."$minerid, "
		.($schedule ? "$schedule, " : 'NULL, ')
		."GETDATE(), "
		."0x$data[hex])");
	list($id) = sql_fetch("SELECT SCOPE_IDENTITY()");

	if($schedule)
		sql_query("UPDATE schedules SET collected = collected+1 WHERE id=$schedule");

	return $id;
}

function add_ms_file($filename)
{
	$file = file_get_contents($filename);
	if(substr($file,0,3) == pack("CCC",0xef,0xbb,0xbf)) // UTF-8 BOM
		$file = utf8_decode(substr($file,3));

	// This should mimic our code very well
	preg_match_all('/(?:\\r|\\n)"([^"\\r\\n]*)".*?(?:<<(.*?)>>|"([^\\n\\r]*)")/m',$file,$m,PREG_SET_ORDER);
	foreach($m as $v)
	{
		if(strlen($v[1]) > 250)
			die("PString Identifier Overflow in $filename<br>$v[1]");
		sql_query("DELETE FROM pstrings WHERE id=".sql_string($v[1]));
		sql_query("INSERT INTO pstrings (id,string) VALUES (".sql_string($v[1]).",".sql_string(@$v['2'].@$v['3']).")");
	}

	return count($m);
}

// This is smart enough to parse .salvage, .recipe and .powers (mostly) files
function add_def_file($filename)
{
	$count = 0;
	$sub = 0;
	$name = false;
	$type = false;
	$fields = array();

	foreach(file($filename) as $line)
	{
		$line = trim($line);
		if($line == '' || substr($line,0,2) == '//')
			continue;

		if($sub > 1) // currently, only grab top level information
		{
			if($line == '}')
				$sub--;
		}
		else if($line == '{')
		{
			$sub++;
		}
		else if($line == '}')
		{
			// sanity check
			if(!$name)
				die('Error: No Name defined.');
			if(!$type)
				die('Error: No Type defined.');
			if(!$sub)
				die('Error: Got confused by braces {}.');

			// delete any old data in there
			$def = sql_fetch("SELECT id FROM defs WHERE name=".sql_string($name)." AND type=".sql_string($type));
			if($def)
			{
				sql_query("DELETE FROM defdata WHERE def=$def[id]");
				sql_query("DELETE FROM defs WHERE id=$def[id]");
			}

			// insert the new stuff
			sql_query("INSERT INTO defs (type,name) VALUES (".sql_string($type).",".sql_string($name).")");
			list($id) = sql_fetch("SELECT SCOPE_IDENTITY()");
			foreach($fields as $field => $v)
				foreach($v as $subid => $data)
				{
					if(preg_match('|^\\d+$|',$data))
					{
						$datatype = 'int';
						$data = (int)$data;
					}
					else if(preg_match('|^\\d*\\.\\d+$|',$data))
					{
						$datatype = 'float';
						$data = (float)$data;
					}
					else
					{
						$datatype = 'string';
						$data = sql_string($data);
					}

					sql_query("INSERT INTO defdata (def,field,subid,datatype,${datatype}data) VALUES ($id,".sql_string($field).",$subid,'$datatype',$data)");
				}
			
			// get ready for the net chunk
			$count++;
			$sub--;
			$name = false;
			$type = false;
			$fields = array();
		}
		else
		{
			if(!preg_match('|^(\\S+)(.*)$|',$line,$m))
				die("Error: I could not parse this line:<br><code>$line</code>");
			$lhs = $m[1];
			$rhs = trim($m[2]);

			if(!$type)
			{
				if($lhs == 'Power')
				{
					$m = explode('.',$rhs);
					$name = $m[2];
					$type = $m[0];
					$fields['powerset'][] = $m[1];
				}
				else
				{
					$name = $rhs;
					$type = $lhs;
				}
			}
			else
			{
				if(substr($rhs,0,1) == '"' && substr($rhs,-1) == '"')
				{
					if($lhs == 'Name')
						$name = substr($rhs,1,-1);
					else
						$fields[$lhs][] = substr($rhs,1,-1);
				}
				else if($lhs == 'Name')
					$name = $rhs;
				else if($rhs != '')
				{
					// this is wrong, but should be suitable
					$fields[$lhs][] = $rhs;
					foreach(preg_split('|,?\\s+|',$rhs) as $v)
						$fields[$lhs][] = $v;
				}
			}
		}
	}

	return $count;
}

?>