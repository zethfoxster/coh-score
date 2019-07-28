<?php
require_once('utils_sql.php');

// This script should be running hourly, so get data within an half an hour of the correct time
foreach(new QueryIterator("SELECT * FROM schedules WHERE desired=0 OR collected<desired") as $v)
{
	echo( "<br>" );
	if(strtotime($v['start']) + 3600*$v['frequency']*$v['collected'] < time() + 1800)
	{
		echo "Collecting $v[miner]:$v[id]...\n";
		collect_data($v['miner'],$v['id']);
	}
	echo( "<br>" );
}
echo "Done.\n";
?>