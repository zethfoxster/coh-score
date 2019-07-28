<?php
require_once('utils.php');
require_once('common.php');
require_once('common_sql.php');

function create_miner($id, $name, $category, $description, $serverhandle, $container, $frequency, $timeframe,
$dataaggs, $dataids, $groupids, $whereids, $wherecmps, $wherevals, $emailnames, $emailaddr, $forcecreate)
{
	global $common_aggregation, $common_timeframes, $common_comparison;
	$mineacc = substr($container,-2) == '.%';

	// fixup
	if(!is_array($dataaggs))
	$dataaggs = array();
	if(!is_array($dataids))
	$dataids = array();
	if(!is_array($groupids))
	$groupids = array();
	if(!is_array($whereids))
	$whereids = array();
	if(!is_array($wherecmps))
	$wherecmps = array();
	if(!is_array($wherevals))
	$wherevals = array();
	if(!is_array($emailnames))
	$emailnames = array();
	if(!is_array($emailaddr))
	$emailaddr = array();

	$dataheaders = array();
	$groupheaders = array();
	$whereheaders = array();

	// error checking, qualifier generation
	if($mineacc)
	die("Not implemented!");

	for($i = 0; $i < count($dataids); $i++)
	{
		if($common_aggregation[$dataaggs[$i]]['data'])
		{
			$row = sql_fetch("SELECT * FROM containers WHERE id=$dataids[$i]");
			if(!$row)
			die("Error: Aggregation needs a data column, but no column definition was found for data (#$dataids[$i]).");
			if($mineacc  != (substr($row['tablename'],0,7) == 'mineacc'))
			die("Error: Mixed container types.");
		}
		else
		{
			$row = false;
		}
		$dataheaders[] = $common_aggregation[$dataaggs[$i]]['disp'].($row ? " $row[name]" : '');
	}

	foreach($groupids as $groupid)
	{
		if($groupid)
		{
			$row = sql_fetch("SELECT * FROM containers WHERE id=$groupid");
			if(!$row)
			die("Error: No column definition was found for grouping (#$groupid).");
			if($mineacc != (substr($row['tablename'],0,7) == 'mineacc'))
			die("Error: Mixed container types.");
			$groupheaders[] = $row['name'];

		}
		else
		{
			$groupheaders[] = "Shard";
		}
	}

	for($i = 0; $i < count($whereids); $i++)
	{
		// donotcheckin : more checking
		$row = sql_fetch("SELECT * FROM containers WHERE id=$whereids[$i]");
		if(!$row)
		die("Error: No column definition was found for constraint (#$whereid).");
		if($mineacc != (substr($row['tablename'],0,7) == 'mineacc'))
		die("Error: Mixed container types.");
		$whereheaders[] = str_replace(	array('$0', '$1'),
		array("$row[tablename].$row[columnname]", $wherevals[$i]),
		$common_comparison[$wherecmps[$i]]['human'] );
	}

	$servers = array();
	foreach(new QueryIterator("SELECT servers.* FROM servers, serverlookup WHERE serverlookup.handle=$serverhandle AND servers.id=serverlookup.server") as $server)
	{
		if(count($servers))
		if($server['loginaddr'] != $servers[0]['loginaddr'] || $server['loginuser'] != $servers[0]['loginuser'] || $server['loginpass'] != $servers[0]['loginpass'])
		die("Error: Servers have mixed access");
		$servers[] = $server;
	}
	if(!count($servers))
	die("Error: Server handle has no servers");

	// etc
	$dataaggs = join("\n", $dataaggs);
	$dataids = join("\n", $dataids);
	$groupids = join("\n", $groupids);
	$whereids = join("\n", $whereids);
	$wherecmps = join("\n", $wherecmps);
	$wherevals = join("\n", $wherevals); // FIXME: unsafe?
	$emailnames = join("\n", $emailnames);
	$emailaddr = join("\n", $emailaddr);

	// this seems odd to pull out some other data miner that might not match exactly and use it instead, I'm going to disable for now
	// I don't think the data miner list is so egregiously large that this is needed
	//if(!$id && !$forcecreate)
	//{
	//	$id = sql_fetch("SELECT id FROM miners WHERE servers=$serverhandle".
	//											" AND container LIKE ".sql_string($container).
	//											" AND dataaggs LIKE ".sql_string($dataaggs).
	//											" AND dataids LIKE ".sql_string($dataids).
	//											" AND groupids LIKE ".sql_string($groupids).
	//											" AND whereids LIKE ".sql_string($whereids).
	//											" AND wherecmps LIKE ".sql_string($wherecmps).
	//											" AND wherevals LIKE ".sql_string($wherevals));
	//	if($id)
	//		return $id['id'];
	//}

	list($qualifier) = sql_fetch("SELECT name FROM serverhandles WHERE id=$serverhandle");
	$qualifier = "$qualifier, "
	.($mineacc ? substr($container,0,-2) : $container)
	." - "
	.(count($dataheaders) ? ("(".join_humanreadable($dataheaders).")") : '')
	.(count($dataheaders) && count($groupheaders) ? " by " : '')
	.(count($groupheaders) ? ("(".join_humanreadable($groupheaders).")") : '')
	.(count($whereheaders) ? (" where (".join_humanreadable($whereheaders).")") : '')
	.($mineacc ? (' '.$common_timeframes[$timeframe]['disp']) : '');

	if(!$name)
	$name = $qualifier;

	if(!$category)
	$category = "Uncategorized";

	if(!$description)
	$description = "No description";

	if(!$id)
	{
		sql_query("INSERT INTO miners (name,category, description, qualifier,servers,container,dataaggs,dataids,groupids,whereids,wherecmps,wherevals,emailnames,emailaddr) VALUES ("
		.sql_string($name).", "
		.sql_string($category).", "
		.sql_string($description).", "
		.sql_string($qualifier).", "
		.(int)$serverhandle.", "
		.sql_string($container).", "
		.sql_string($dataaggs).", "
		.sql_string($dataids).", "
		.sql_string($groupids).", "
		.sql_string($whereids).", "
		.sql_string($wherecmps).", "
		.sql_string($wherevals).","
		.sql_string($emailnames).", "
		.sql_string($emailaddr).")");
		list($id) = sql_fetch("SELECT SCOPE_IDENTITY()");
	}
	else
	{
		sql_query(	"UPDATE miners SET ".
					"name=".sql_string($name).", ".
					"category=".sql_string($category).", ".
					"description=".sql_string($description).", ".
					"qualifier=".sql_string($qualifier).", ".
					"servers=".(int)$serverhandle.", ".
					"container=".sql_string($container).", ".
					"dataaggs=".sql_string($dataaggs).", ".
					"dataids=".sql_string($dataids).", ".
					"groupids=".sql_string($groupids).", ".
					"whereids=".sql_string($whereids).", ".
					"wherecmps=".sql_string($wherecmps).", ".
					"wherevals=".sql_string($wherevals).", ".
					"emailnames=".sql_string($emailnames).", ".
					"emailaddr=".sql_string($emailaddr).
					"WHERE id=$id" );
	}

	// if start is some time in the past, assume now
	if( $frequency )
	{
		$start = time();
		$desired = 0;

		sql_query("INSERT INTO schedules (miner,frequency,start,desired) VALUES ("
		."$id,"
		."$frequency,"
		."GETDATE(), "
		."$desired)" );
		list($schedule) = sql_fetch("SELECT SCOPE_IDENTITY()");
	}

	return $id;
}

function generate_query($miner)
{
	global $common_aggregation, $config_compressionlevel, $common_comparison, $config_container_id;

	$serverhandle = $miner['servers'];
	$headers = array();

	$servers = array();
	foreach(new QueryIterator("SELECT servers.* FROM servers, serverlookup WHERE serverlookup.handle=$serverhandle AND servers.id=serverlookup.server") as $server)
	{
		if(count($servers))
		{
			if($server['loginaddr'] != $servers[0]['loginaddr'] || $server['loginuser'] != $servers[0]['loginuser'] || $server['loginpass'] != $servers[0]['loginpass'])
				die("Error: Servers have mixed access");
		}
		$servers[] = $server;
	}
	if(!count($servers))
		die("Error: Server handle has no servers");

	if(trim($miner['customquery']))
	{
		$union = array();
		foreach($servers as $server)
			$union[] = str_replace(	array('$0', '$1'),
		array("$server[centerdb].$server[serverdb].dbo", sql_string($server['name'])),
		$miner['customquery'] );

		$query = join("\nUNION ALL ", $union)."\n";
		$headers[] = unjoin("\n", trim($miner['customheaders']));
		
		$emailnames = unjoin("\n", trim($miner['emailnames']));
		$emailaddr = unjoin("\n", trim($miner['emailaddr']));

		$emails = array();
		$count = count($emailnames);
		for($i = 0; $i < count($emailnames); $i++)
		{
			$emails[$i] = "$emailnames[$i]$emailaddr[$i]";
		}
	}
	else
	{
		$container = $miner['container'];
		$mineacc = substr($container,-2) == '.%';
		$dataaggs = unjoin("\n", trim($miner['dataaggs']));
		$dataids = unjoin("\n", trim($miner['dataids']));
		$groupids = unjoin("\n", trim($miner['groupids']));
		$whereids = unjoin("\n", trim($miner['whereids']));
		$wherecmps = unjoin("\n", trim($miner['wherecmps']));
		$wherevals = unjoin("\n", trim($miner['wherevals']));
		$emailnames = unjoin("\n", trim($miner['emailnames']));
		$emailaddr = unjoin("\n", trim($miner['emailaddr']));

		$emails = array();
		$count = count($emailnames);
		for($i = 0; $i < count($emailnames); $i++)
		{
			$emails[$i] = "$emailnames[$i]$emailaddr[$i]";
		}

		$data = array();
		for($i = 0; $i < count($dataids); $i++)
		{
			if($common_aggregation[$dataaggs[$i]]['data'])
			{
				$data[$i] = sql_fetch("SELECT * FROM containers WHERE id=$dataids[$i]");
				if(!$data[$i])
				die("Error: Aggregation needs a data column, but no column definition was found for data (#$dataids[$i]).");
			}
			else
			{
				$data[$i] = false;
			}
		}

		$groups = array();
		foreach($groupids as $groupid)
		{
			if($groupid)
			{
				$group = sql_fetch("SELECT * FROM containers WHERE id=$groupid");
				if(!$group)
					die("Error: No column definition was found for grouping (#$groupid).");
				if($mineacc != (substr($group['tablename'],0,7) == 'mineacc'))
					die("Error: Mixed container types.");
				$groups[] = $group;
			}
			else
			{
				$groups[] = false;
			}
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
		$where = array();
		$groupby = array();

		$dataheaders = array();
		$groupheaders = array();
		$whereheaders = array();

		$multitables = array();
		$cols = array();
		$from[] = "$0.$container as $container"; // CAVEAT: assumes there's a 1-to-1 table of the same name as container
		$col = 1;
		$attr = 0;

		foreach($groups as $group)
		{
			if($group)
			{
				if($group['tablemapping'] > 1)
					$multitables[$group['tablename']][] = $group['columnname'];
				else
					$from[] = "$0.$group[tablename] as $group[tablename]";

				if($group['tablename'] != $container)
					$where[] = "$group[tablename].$config_container_id = $container.$config_container_id";

				$colname = "col${col}_$group[columnname]";

				if($group['columntype'] == 'attribute')
				{
					$from[] = "$0.Attributes as attr$attr";
					$where[] = "attr$attr.Id=$group[tablename].$group[columnname]";
					$cols[] = "attr$attr.Name as $colname";
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
				else if($group['columntype'] == 'binary')
				{
					$cols[] = "CONVERT(VARCHAR(MAX),sys.fn_sqlvarbasetostr($group[tablename].$group[columnname])) as $colname";
				}
				else
				{
					$cols[] = "$group[tablename].$group[columnname] as $colname";
				}

				$groupheaders[] = $group['name'];
		}
		else
		{
			$colname = "col${col}_Shard";
			$cols[] = "$1 as $colname";
			$groupheaders[] = "Shard";
		}

		$groupby[] = $colname;
		$col++;
	}

	for($i = 0; $i < count($data); $i++)
	{
		$datum = $data[$i];
		if($datum)
		{
			$colname = "col${col}_$datum[columnname]";
			if($datum['columntype'] == 'int')
				$data_cast = "CAST($colname as bigint)";
			else // $datum['columntype'] == 'float'
				$data_cast = "$colname";
	
			$select[] = str_replace('$',$data_cast,$common_aggregation[$dataaggs[$i]]['func']);
	
			if($datum['tablemapping'] > 1)
				$multitables[$datum['tablename']][] = $datum['columnname'];
			else
				$from[] = "$0.$datum[tablename] as $datum[tablename]";
	
			if($datum['tablename'] != $container)
				$where[] = "$container.$config_container_id = $datum[tablename].$config_container_id";
	
			$cols[] = "$datum[tablename].$datum[columnname] as $colname";
		}
		else
		{
			$select[] = $common_aggregation[$dataaggs[$i]]['func'];
		}

		$dataheaders[] = $common_aggregation[$dataaggs[$i]]['disp'].($datum ? " $datum[name]" : '');
		$col++;
	}

	for($i = 0; $i < count($wheres); $i++)
	{
		$wherecol = $wheres[$i];

		if($wherecol['tablemapping'] > 1)
			$multitables[$wherecol['tablename']][] = $wherecol['columnname'];
		else
			$from[] = "$0.$wherecol[tablename] as $wherecol[tablename]";
		if($wherecol['tablename'] != $container)
			$where[] = "$wherecol[tablename].$config_container_id = $container.$config_container_id";

		// FIXME: validate comparison function values
		if($wherecol['columntype'] == 'attribute')
		{
			$from[] = "$0.Attributes as attr$attr";
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
		{
			$where[] = str_replace(	array('$0','$1'),
			array(	(@$common_comparison[$wherecmps[$i]]['nullsafe'] || 1)
			? "$wherecol[tablename].$wherecol[columnname]"
			: "ISNULL($wherecol[tablename].$wherecol[columnname],0)",
			sql_string(@$common_comparison[$wherecmps[$i]]['nodata']
			? ''
			: $wherevals[$i])
			),
			$common_comparison[$wherecmps[$i]]['func']);
		}

		$whereheaders[] = str_replace(	array('$0','$1'),
		array(	"$wherecol[tablename].$wherecol[columnname]",
		@$common_comparison[$wherecmps[$i]]['nodata']
		? ''
		: $wherevals[$i]
		),
		$common_comparison[$wherecmps[$i]]['human']);
	}

	foreach($multitables as $k => $v)
	{
		$v = array_unique($v);
		$from[] = "(SELECT DISTINCT $config_container_id, ".join(', ',$v)." FROM $0.$k) as $k";
		$where[] = "$k.$config_container_id = $container.$config_container_id";
	}

	// donotcheckin : clean $cols
	$from = array_unique($from);
	$where = array_unique($where);

	$query = "  SELECT ".(count($cols) ? join(",\n         ", $cols) : "$container.$config_container_id")
	.(count($from) ? ("\n\n  FROM ".join(",\n       ", $from)) : '')
	.(count($where) ? ("\n\n  WHERE ".join("\n    AND ", $where)) : '');

	$union = array();
	foreach($servers as $server)
	{
		$union[] = str_replace(	array('$0', '$1'),
		array("$server[centerdb].$server[serverdb].dbo", sql_string($server['name'])),
		$query );
		if(count($union) == 1)
			$query = preg_replace('/\\s+/', ' ', $query);
		else if(count($union) == 2)
			$union[0] .= "\n\n  -- as above, for other servers";
	}


	$select = array_unique(array_merge($groupby,$select));
	$query = "SELECT ".join(",\n       ", $select)."\n\n"
	."FROM (\n".join("\n  UNION ALL", $union)."\n) as rows"
	.(count($groupby) ? ("\n\nGROUP BY ".join(', ', $groupby)."\nORDER BY ".join(', ', $groupby)) : '')
	."\n";

	foreach($whereheaders as $v)
		$headers[] = array($v);
	if(count($groupheaders) || count($dataheaders))
		$headers[] = array_merge($groupheaders,$dataheaders);

	/*
	 list($qualifier) = sql_fetch("SELECT name FROM serverhandles WHERE id=$serverhandle");
	 $qualifier = "$qualifier, "
		.($mineacc ? substr($container,0,-2) : $container)
		." - "
		.(count($dataheaders) ? ("(".join_humanreadable($dataheaders).")") : '')
		.(count($dataheaders) && count($groupheaders) ? " by " : '');
		.(count($groupheaders) ? ("(".join_humanreadable($groupheaders).")") : '')
		.(count($whereheaders) ? (" where (".join_humanreadable($whereheaders).")") : '')
		.($mineacc ? (' '.$common_timeframes[$timeframe]['disp']) : '');
		if($qualifier != $miner['qualifier'])
		sql_query("UPDATE miners SET qualifier=".sql_string($qualifier)." WHERE id=$miner[id]");
		*/
	}
	return array($query, $headers, $servers, $emails);
}


function mail_attachment($content, $mailto, $from_mail, $from_name, $replyto, $subject, $message) {
	//$content = base64_encode($content);
	$uid = md5(uniqid(time()));

	$header = "From: ".$from_name." <".$from_mail.">\r\n";
	$header .= "Reply-To: ".$replyto."\r\n";
	$header .= "MIME-Version: 1.0\r\n";
	$header .= "Content-Type: multipart/mixed; boundary=\"".$uid."\"\r\n\r\n";

	$body = "This is a multi-part message in MIME format.\n\n";
	$body .= "--".$uid."\r\n";
	$body .= "Content-type:text/plain; charset=iso-8859-1\r\n";
	$body .= "Content-Transfer-Encoding: 7bit\r\n\r\n";
	$body .= $message."\r\n\r\n";
	$body .= "--".$uid."\r\n";

	$body .= "Content-Type: application/csv; name=\"attachment.csv\"\r\n"; // use different content types here
	$body .= "Content-Length: ".strlen($content);
	$body .= "Content-Transfer-Encoding: 7bit\r\n";
	$body .= "Content-Disposition: attachment\r\n\r\n";
	$body .= $content."\r\n\r\n";
	$body .= "--".$uid;

	if(!mail($mailto, $subject, $body, $header))
	{
		die( "Error sending mail" );
	}
}


// FIXME: how fast is this function with a lot of data?
function collect_data($minerid, $schedule = false)
{
	global $config_compressionlevel;

	$miner = sql_fetch("SELECT * FROM miners WHERE id=$minerid");
	list($query, $headers, $servers, $emails) = generate_query($miner);

	set_time_limit(9999999999);
	// build csv
	$server = sql_server_connect($servers[0], true);
	sql_query("SET ANSI_NULLS ON", $server);
	sql_query("SET ANSI_WARNINGS ON", $server);
	$csv = '';
	foreach($headers as $v)
	$csv .= format_csv($v);
	foreach(new QueryIterator($query, $server, MSSQL_NUM) as $row)
	{
		$csv .= format_csv($row);
	}

	$name = $miner['name'];
	$subject = "Miner Report: $name";
	mail_attachment($csv, "cbennett@ncsoft.com", "cbennett@ncsoft.com", "Dataminer", "cbennett@ncsoft.com", $subject, $miner['description']  );
	foreach($emails as $mail)
	{
		mail_attachment($csv, $mail, "cbennett@ncsoft.com", "Dataminer", "cbennett@ncsoft.com", $subject, $miner['description']  );
	}

	// pack csv
	$origsize = strlen($csv);
	$csv = gzcompress($csv,$config_compressionlevel);
	$csv = unpack("H*hex",$csv);
		
	sql_query("INSERT INTO data (miner,schedule,date,data) VALUES ("
	."$minerid, "
	.($schedule ? "$schedule, " : 'NULL, ')
	."GETDATE(), "
	."0x$csv[hex])");
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
		die("PString Identifier Overflow in $filename--><br>$v[1]");
		sql_query("DELETE FROM pstrings WHERE id=".sql_string($v[1]));
		sql_query("INSERT INTO pstrings (id,string) VALUES (".sql_string($v[1]).",".sql_string(@$v['2'].@$v['3']).")");
	}

	return count($m);
}

function add_attributes_file($filename)
{
	$file = file_get_contents($filename);
	if(substr($file,0,3) == pack("CCC",0xef,0xbb,0xbf)) // UTF-8 BOM
	$file = utf8_decode(substr($file,3));

	// This should mimic our code very well
	$tokens = preg_split("/[\n]/",$file,0,PREG_SPLIT_NO_EMPTY);
	$len = count($tokens);
	for( $i = 16000; $i < $len; $i++ )
	{
		$id = strtok( $tokens[$i], " ");
		$remainder = strtok( "\n" );
		$remainder = str_replace("\"", "", $remainder);
		sql_query( "DELETE FROM attributes WHERE id=".sql_string($id) );
		sql_query("INSERT INTO attributes (id,string) VALUES (".sql_string($id).",".sql_string($remainder).")");
	}


	return $len;
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