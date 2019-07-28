<?php
require_once('common_sql.php');

// TODO: add xls support

// FIXME: better filenames
function filename_csv($miner,$data)
{
	return substr(preg_replace('|\\W|','',ucwords($miner['name'])),0,100).'-'.date('y-m-d-Hi',strtotime($data['date'])).'.csv';
}

function filename_zip($miner)
{
	return substr(preg_replace('|\\W|','',ucwords($miner['name'])),0,100).'.zip';
}

if(@(int)$_GET['d']) // data
{
	$data = sql_fetch("SELECT * FROM data WHERE id=".(int)$_GET['d']);	
	$miner = sql_fetch("SELECT * FROM miners WHERE id=$data[miner]");
	$filename = filename_csv($miner,$data);
		
	//die( "Start ".strlen($data['data']) );
	
	$dump = gzuncompress($data['data']);
	if(!$dump)
		die( "Could not uncompress file" );
		
	//die( "Uncomp ".strlen($dump) );	
    header("Content-Type: text/csv");
	header("Content-Length: ".strlen($dump));
	header("Content-Disposition: attachment; filename=$filename");
    echo ($dump);
	exit();
}
else if(@(int)$_GET['s'] || @(int)$_GET['m']) // schedule or miner
{
	if(@(int)$_GET['s'])
	{
		$schedule = sql_fetch("SELECT * FROM schedules WHERE id=".(int)$_GET['s']);
		$miner = sql_fetch("SELECT * FROM miners WHERE id=$schedule[miner]");
		$where = 'schedule='.(int)$_GET['s'];
	}
	else // @(int)$_GET['m']
	{
		$miner = sql_fetch("SELECT * FROM miners WHERE id=".(int)$_GET['m']);
		$where = 'miner='.(int)$_GET['m'];
	}

	$temp = sys_get_temp_dir().$_SERVER['REMOTE_ADDR'].'-'.uniqid().'.zip';
	$zip = new ZipArchive();
	if(!$zip->open($temp,ZIPARCHIVE::CREATE))
	{
		$template_errorbox[] = "Could not open temporary file for writing!";
		include('template_top.php');
		include('template_bottom.php');
		exit();
	}

	foreach(new QueryIterator("SELECT * FROM data WHERE $where ORDER BY date") as $data)
		$zip->addFromString(filename_csv($miner,$data),gzuncompress($data['data']));
	$zip->close();

    header("Content-Type: application/zip");
	header("Content-Length: ".filesize($temp));
	header("Content-Disposition: attachment; filename=".filename_zip($miner));
    readfile($temp);

	unlink($temp);
}

?>