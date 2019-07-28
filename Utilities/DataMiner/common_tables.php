
<?php
require_once('config.php');
require_once('mssql.php');
require_once('common.php');

function table_miners()
{
	$query = new QueryIterator("SELECT id, category, name, description FROM miners ORDER BY category, name");
	$category = '';
	$catid = 0;
	$row = 0;

	echo "<table id='minertable'>\n";
	echo "<tr><th class='rowt'>Data Miners\n";
	foreach($query as $v)
	{
		if($category != $v['category'])
		{
			$category = $v['category'];
			$catid++;
			echo "<tr><th class='rowh' onClick='"
				."var rows=document.getElementById(\"minertable\").rows;"
				."for(var i=0;i<rows.length;i++)"
				.	"if(rows[i].id==\"cat$catid\")"
				.		"rows[i].style.display=(rows[i].style.display&&rows[i].style.display==\"none\"?\"\":\"none\");"
				."' onMouseOver='this.className=\"rowhh\"' onMouseOut='this.className=\"rowh\"'>$category\n";
		}
		list($count) = sql_fetch("SELECT COUNT(*) FROM data WHERE data.miner=$v[id]");
		echo "<tr style='display:none' id='cat$catid'><td class='row$row'>"
			."<a href='miner.php?i=$v[id]&amp;a=d'><img src='img/drop.png' title='Delete' class='icon'></a>"
			."<a href='miner.php?i=$v[id]&amp;a=n'><img src='img/copy.png' title='Copy' class='icon'></a>"
			."<a href='miner.php?i=$v[id]&amp;a=e'><img src='img/edit.png' title='Edit' class='icon'></a>"
			.($count
				? "<a href='get.php?m=$v[id]'><img src='img/get.png' title='Download' class='icon'></a>"
				: "<img src='img/blank.png' class='icon'>")
			." <a href='miner.php?i=$v[id]'>$v[name]</a><br>$v[description]\n";
		$row = $row ? 0 : 1;
	}
	if(!$query->num_rows())
		echo "<tr><td class='row$row nodata'>No data miners";
	echo "<tr><td class='rowf'><a href='miner.php?a=n'><img src='img/new.png' class='icon'>Create a new Miner</a><br>\n";
	echo "</table>\n";

	return $query->num_rows();
}

function table_schedule($miner = false)
{
	if($miner)
	{
		$query = new QueryIterator("SELECT * FROM schedules WHERE miner=$miner");
		$cols = 4;
	}
	else
	{
		$query = new QueryIterator("SELECT schedules.*, miners.name as minername FROM schedules, miners WHERE schedules.miner=miners.id ORDER BY minername");
		$cols = 5;
	}
	$row = 0;

	echo "<table>\n";
	echo "<tr><th colspan=$cols class='rowt'>Collection Schedule\n";
	echo "<tr><th class='rowh'>Frequency<th class='rowh'>Start<th class='rowh'>Finish<th class='rowh'>Data".(!$miner ? "<th class='rowh'>Miner" : '')."\n";
	foreach($query as $v)
	{
		$url = "schedule.php?i=$v[id]";
		echo "<tr>";
		list($count) = sql_fetch("SELECT COUNT(*) FROM data WHERE data.schedule=$v[id]");
		echo "<td class='row$row'>"
			."<a href='$url&amp;a=d'><img src='img/drop.png' title='Delete' class='icon'></a>"
			."<a href='$url&amp;a=n'><img src='img/copy.png' title='Copy' class='icon'></a>"
			.(!$v['desired'] || $v['collected'] < $v['desired']
				? "<a href='$url&amp;a=e'><img src='img/edit.png' title='Edit' class='icon'></a>"
				: "<img src='img/blank.png' class='icon'>")
			.($v['collected']
				? "<a href='get.php?s=$v[id]'><img src='img/get.png' title='Download' class='icon'></a>"
				: "<img src='img/blank.png' class='icon'>")
			." <a href='$url'>".readable_frequency($v['frequency'])."</a>";
		echo "<td class='row$row'><a href='$url'>".reformat_date($v['start'])."</a>";
		echo "<td class='row$row'><a href='$url'>".($v['desired'] ? reformat_date_plus($v['start'],$v['frequency'],$v['desired']) : 'Never')."</a>";
		echo "<td class='row$row'><a href='$url'>".($v['desired'] ? ((int)(100 * $v['collected']/$v['desired'])."%") : $v['collected'])."</a>";
		if(!$miner)
			echo "<td class='row$row'>$v[minername]";
		echo "\n";
		$row = $row ? 0 : 1;
	}
	if(!$query->num_rows())
		echo "<tr><td colspan=$cols class='row$row nodata'>No schedules\n";
	if($miner)
		echo "<tr><td colspan=$cols class='rowf'><a href='schedule.php?m=$miner&amp;a=n'><img src='img/new.png' class='icon'> Add a new schedule for this Miner</a>\n";
	echo "</table>\n";

	return $query->num_rows();
}

function table_data($miner = false, $schedule = false) // TODO: paging
{
	if($miner)
		$where = "WHERE miner=$miner AND schedule IS NULL";
	else if($schedule)
		$where = "WHERE schedule=$schedule";
	else
		$where = '';
		
	$query = new QueryIterator("SELECT id, date FROM data $where ORDER BY date DESC");
	$row = 0;

	echo "<table>\n";
	if($miner)
		echo "<tr><th class='rowh'>Unscheduled Data\n";
	else if($schedule)
		echo "<tr><th class='rowh'>Collected Data\n";
	foreach($query as $v)
	{
		echo "<tr><td class='row$row'>"
			.($schedule
			?''
			:"<a href='miner.php?i=$miner&amp;d=$v[id]'><img src='img/drop.png' title='Delete' class='icon'></a>")
			."<a href='get.php?d=$v[id]'><img src='img/get.png' title='Download' class='icon'>".reformat_datetime($v['date'])."</a>\n";
		$row = $row ? 0 : 1;
	}
	if(!$query->num_rows())
		echo "<tr><td class='row$row nodata'>No data collected\n";
	if($miner)
		echo "<tr><td class='rowf'><a href='?i=$miner&amp;a=z'><img src='img/go.png' class='icon'> Collect Data Now</a>\n";
	echo "</table>\n";

	return $query->num_rows();
}

?>
