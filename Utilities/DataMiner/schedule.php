<?php
require_once('common_sql.php');
require_once('common_forms.php');
require_once('common_tables.php');
require_once('utils_sql.php');

$infobox = false;

if(@(int)$_GET['i'])
	$id = (int)$_GET['i'];
else if(@(int)$_POST['id'])
	$id = (int)$_POST['id'];
else
	$id = false;

if(@$_POST['delete'] && $id)
{
	sql_query("DELETE FROM data WHERE schedule=$id");
	sql_query("DELETE FROM schedules WHERE id=$id");
	goto_page("$config_urlroot/miner.php?i=$_POST[miner]");
}
else if(@$_POST['move'] && $id)
{
	sql_query("UPDATE data SET schedule=NULL WHERE schedule=$id");
	sql_query("DELETE FROM schedules WHERE id=$id");
	goto_page("$config_urlroot/miner.php?i=$_POST[miner]");
}
else if(@$_POST['edit'] && $id)
{
	$schedule = sql_fetch("SELECT * FROM schedules WHERE id=$id");
	$now = false;
	$start = strtotime($schedule['start']);
	$miner = $schedule['miner'];
	$frequency = $schedule['frequency_schedule'];

	$update = array();
	if($schedule['collected'] <= 1)
	{
		$frequency = (int)@$_POST['frequency_schedule'] ? (int)$_POST['frequency_schedule'] : (int)@$_POST['frequency_other'];
		$update[] = "frequency=$frequency";
	}
	if(!$schedule['collected'])
	{
		$miner = (int)$_POST['miner'];
		$update[] = "miner=$miner";

		$now = $_POST['start'] == 'now' || strtotime($_POST['start_other']) < time();
		$start = $now ? time() : strtotime($_POST['start_other']);
		$update[] = "start=".($now ? 'GETDATE()' : sql_date($_POST['start_other']));
	}
	if(!$schedule['desired'] || $schedule['collected'] < $schedule['desired'])
	{
		if($_POST['finish'] == 'never')
			$desired = 0;
		else if($_POST['finish'] == 'date') // calculate difference, include start, minimum of 1
			$desired = max(1, (1 + ( strtotime($_POST['finish_other']) - $start ) / (3600*$frequency)) );
		else if($_POST['finish'] == 'count')
			$desired = (int)$_POST['finish_other'];
		else // end now... this will get set to 'collected'
			$desired = 1;
		if($desired) // don't allow 'desired' to be set less than 'collected'
			$desired = max($desired,$schedule['collected']);
		$update[] = "desired=$desired";
	}
	if(count($update))
		sql_query("UPDATE schedules SET ".join(',',$update)." WHERE id=$id");
	if($now)
		collect_data($miner,$id);

	$template_infobox[] = "This Schedule was successfully updated.";
}
else if(@$_POST['new'])
{
	// FIXME: check for schedule duplication?

	// if start is some time in the past, assume now
	$now = $_POST['start'] == 'now' || strtotime($_POST['start_other']) < time();
	$start = $now ? time() : strtotime($_POST['start_other']);
	$miner = (int)$_POST['miner'];
	$frequency = (int)@$_POST['frequency_schedule'] ? (int)$_POST['frequency_schedule'] : (int)@$_POST['frequency_other'];
	
	if($_POST['finish'] == 'never')
		$desired = 0;
	else if($_POST['finish'] == 'date') // calculate difference, include start, minimum of 1
		$desired = max(1, (1 + (strtotime($_POST['finish_other']) - $start ) / (3600*$frequency)) );
	else if($_POST['finish'] == 'count')
		$desired = (int)$_POST['finish_other'];
	else // end now... i.e. collect once at start
		$desired = 1;

	sql_query("INSERT INTO schedules (miner,frequency,start,desired) VALUES ("
				."$miner,"
				."$frequency,"
				.($now ? 'GETDATE()' : sql_date($_POST['start_other'])).","
				."$desired)");
	list($id) = sql_fetch("SELECT SCOPE_IDENTITY()");

	if($now)
		collect_data($miner,$id);
}

$schedule = $id ? sql_fetch("SELECT * FROM schedules WHERE id=$id") : false;
$template_title = 'Schedule'; // FIXME: better title

include('template_top.php');

if($schedule && @$_GET['a'] == 'd')
{
	echo "<form class='warnbox' action='schedule.php' method=post>\n";
	echo "<input type=hidden name='id' value='$schedule[id]'>\n";
	echo "<input type=hidden name='miner' value='$schedule[miner]'>\n";
	echo "You are about to delete this Schedule.<br>\n";
	if($schedule['collected'])
	{
		echo "Do you also want to delete its Collected Data,<br>\n";
		echo "Or move the data to the Miner's unscheduled Collected Data?<br>\n";
	}
	echo "<div class='formrowlong'>";
	if($schedule['collected'])
	{
		echo "<input type=submit class='butdanger' name='delete' value='Delete Schedule and Data'> ";
		echo "<input type=submit class='butdanger' name='move' value='Delete Schedule, but Move Data'> ";
	}
	else
		echo "<input type=submit class='butdanger' name='delete' value='Delete Schedule'> ";
	echo "<input type=submit class='butcancel' name='cancel' value='Cancel'>";
	echo "</div>\n";
	echo "</form>\n";
}
else if( ( $schedule && @$_GET['a'] == 'e' && (!$schedule['desired'] || $schedule['collected'] < $schedule['desired']) ) || 
		(@$_GET['a'] == 'n' && ($schedule || @(int)$_GET['m'])) )
{
	$edit = $_GET['a'] == 'e';

	if($schedule)
	{
		$miner = $schedule['miner'];
		$frequency = $schedule['frequency'];
		$start = htmlspecialchars($schedule['start'],ENT_QUOTES);
		$desired = $schedule['desired'];
		$collected = $edit ? $schedule['collected'] : 0;
	}
	else
	{
		$miner = (int)$_GET['m'];
		$frequency = $start = '';
		$desired = $collected = 0;
		// FIXME: more feedback on selected miner
	}

	echo "<form class='".($edit ? 'editbox' : 'newbox')."' action='schedule.php' method=post>\n";
	if($edit)
		echo "<input type=hidden name='id' value='$schedule[id]'>\n";
	if($collected)
		echo "<input type=hidden name='miner' value='$miner'>\n";
	else
		echo "<div class='formrow'><div class='label'>Miner</div>".select_miner('miner',$miner)."</div>\n";
	if($collected <= 1)
		echo "<div class='formrow'><div class='label'>Frequency</div>".select_frequency('schedule',$frequency)."</div>\n";
	if(!$collected)
		echo "<div class='formrow'><div class='label'>Start</div>".select_schedule_start('start',$start)."</div>\n";
	echo "<div class='formrow'><div class='label'>Finish</div>".select_schedule_finish('finish',$desired)."</div>\n";
	echo "<div class='formrowlong'>"
		.($edit
		?"<input type=submit class='butwarning' name='edit' value='Update Schedule'> "
		:"<input type=submit class='butnew' name='new' value='Create Schedule'> ")
		."<input type=reset  class='butcancel' value='Reset'> "
		."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
		."</div>\n";
	echo "</form>\n";
}

if($schedule)
{
	echo "<span class='title'>".readable_frequency($schedule['frequency'])."</span><br>\n";
	echo "From <span class='title'>".reformat_date($schedule['start'])."</span><br>\n";
	echo ($schedule['desired'] ? ("Until <span class='title'>".reformat_date_plus($schedule['start'],$schedule['frequency'],$schedule['desired'])) : "<span class='title'>Forever")."</span><br>\n";
	echo "<br>\n";
	list($minername) = sql_fetch("SELECT name FROM miners WHERE id=$schedule[miner]");
	echo "Miner:<br><a href='miner.php?i=$schedule[miner]'><span class='title'>$minername</span></a><br>\n";
	echo "<br>\n";
	$data = table_data(false,$schedule['id']);
	echo "<br>\n";
	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	if($data)
		echo "<tr><td class='rowf'><a href='get.php?s=$schedule[id]'><img src='img/get.png' class='icon'> Get All of this Schedule's Data</a>\n";
	if(!$schedule['desired'] || $schedule['collected'] < $schedule['desired']) 
		echo "<tr><td class='rowf'><a href='schedule.php?i=$schedule[id]&amp;a=e'><img src='img/edit.png' class='icon'> Edit this Schedule</a>\n";
	echo "<tr><td class='rowf'><a href='schedule.php?i=$schedule[id]&amp;a=n'><img src='img/copy.png' class='icon'> Make a Copy of this Schedule</a>\n";
	echo "<tr><td class='rowf'><a href='schedule.php?i=$schedule[id]&amp;a=d'><img src='img/drop.png' class='icon'> Delete this Schedule</a>\n";
	echo "<tr><td class='rowf'><a href='miner.php?i=$schedule[miner]'><img src='img/mine.png' class='icon'> Go to this Schedule's Miner</a>\n";
	echo "<tr><td class='rowf'><a href='schedule.php'><img src='img/sched.png' class='icon'> Go to the Full Schedule</a>\n";
	echo "</table>\n";
}
else
	table_schedule();


include('template_bottom.php');
?>