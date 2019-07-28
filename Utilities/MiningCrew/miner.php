<?php
require_once('common_tables.php');
require_once('common_forms.php');
require_once('utils_sql.php');

if(@(int)$_GET['i'])
	$id = (int)$_GET['i'];
else if(@(int)$_POST['id'])
	$id = (int)$_POST['id'];
else
	$id = false;

if(@$_POST['delete'] && $id)
{
	sql_query("DELETE FROM data WHERE miner=$id");
	sql_query("DELETE FROM schedules WHERE miner=$id");
	sql_query("DELETE FROM miners WHERE id=$id");
	$template_infobox[] = "The Miner \"$_POST[name]\", its associated Schedules,<br>and its Collected Data were successfully deleted.";
	$id = false;
}
else if(@$_POST['edit'] && $id)
{
	$category = $_POST['category'] == '0' ? @$_POST['category_other'] : $_POST['category'];
	sql_query("UPDATE miners SET "
		."name=".sql_string($_POST['name']).", "
		."category=".sql_string($category)//.", "
//		."description=".sql_string($_POST['description'])
		." WHERE id=$id");
	$template_infobox[] = "The Miner \"$_POST[name]\" was successfully updated.";
}
else if(@$_POST['new'])
{
	// FIXME: validate the query
	// FIXME: feedback / confirmation
	$category = $_POST['category'] == '0' ? @$_POST['category_other'] : $_POST['category'];
	$id = create_miner(
		@$_POST['name'],
		$category,
		@$_POST['description'],
		$_POST['server_newminer'],
		$_POST['container_newminer'],
		@$_POST['timeframe_newminer'],
		@$_POST['aggregation_newminer'],
		@$_POST['data_newminer'],
		@$_POST['group_newminer'],
		@$_POST['where_newminer'],
		@$_POST['wherecmp_newminer'],
		@$_POST['whereval_newminer'],
		false
	);
}
else if(@$_POST['deletedata'])
{
	sql_query("DELETE FROM data WHERE id=".(int)$_POST['data']);
	$template_infobox[] = "Data from $_POST[date] successfully deleted.";
}

$miner = $id ? sql_fetch("SELECT * FROM miners WHERE id=$id") : false;
$template_title = $miner ? "Miner: $miner[name]" : 'Miners';

include('template_top.php');

if($miner && @$_GET['a'] == 'd')
{
	echo "<form class='warnbox' action='miner.php' method=post>\n";
	echo "<input type=hidden name='id' value='$miner[id]'>\n";
	echo "<input type=hidden name='name' value='".htmlspecialchars($miner['name'],ENT_QUOTES)."'>\n";
	echo "You are about to delete this Miner and all of its Collected Data.<br>\n";
	echo "Are you sure you want to do this?<br>\n";
	echo "<div class='formrowlong'>"
		."<input type=submit class='butdanger' name='delete' value='Delete Miner and Data'> "
		."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
		."</div>\n";
	echo "</form>\n";
}
else if(($miner && @$_GET['a'] == 'e') || @$_GET['a'] == 'n')
{
	$edit = $_GET['a'] == 'e';
	if($miner)
	{
		$name = htmlspecialchars($miner['name'],ENT_QUOTES);
		$category = $miner['category']; // select_miner_category() will apply htmlspecialchars as appropriate
		$description = htmlspecialchars($miner['description'],ENT_QUOTES);
		$template_warnbox[] = "Copying Miners is currently limited to copying Name, Category, and Description.";
	}
	else
		$name = $category = $description = '';

	echo form_mining_prolog();
	echo "<form class='".($edit ? 'editbox' : 'newbox')."' action='miner.php' method=post>\n";
	if($edit)
		echo "<input type=hidden name='id' value='$miner[id]'>\n";
	echo "<div class='formrow'><div class='label'>Name</div><input type=text name='name' value='$name'></div>\n";
	echo "<div class='formrow'><div class='label'>Category:</div>".select_miner_category('category',$category)."</div>\n";
	if(!$edit)
		echo form_mining('newminer','Ents')."\n";
//	echo "<div class='formrow'><div class='label'>Description:</div><textarea name='description' cols=30 rows=3>$description</textarea></div>\n";
	echo "<div class='formrowlong'>"
		.($edit
		?"<input type=submit class='butwarning' name='edit' value='Update Miner'> "
		:"<input type=submit class='butnew' name='new' value='Create Miner'> ")
		."<input type=reset  class='butcancel' value='Reset'> "
		."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
		."</div>\n";
	echo "</form>\n";
}
else if($miner && @$_GET['a'] == 'z')
{
	collect_data($miner['id']);
}
else if($miner && (int)@$_GET['d'])
{
	$data = (int)@$_GET['d'];
	list($date) = sql_fetch("SELECT date FROM data WHERE id=$data");
	$date = reformat_date($date);

	echo "<form class='warnbox' action='miner.php' method=post>\n";
	echo "<input type=hidden name='id' value='$miner[id]'>\n";
	echo "<input type=hidden name='data' value='$data'>\n";
	echo "<input type=hidden name='date' value='".htmlspecialchars($date,ENT_QUOTES)."'>\n";
	echo "You are about to delete the Data collected on $date.<br>\n";
	echo "Are you sure you want to do this?<br>\n";
	echo "<div class='formrowlong'>"
		."<input type=submit class='butdanger' name='deletedata' value='Delete Data'> "
		."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
		."</div>\n";
	echo "</form>\n";
}

if($miner)
{
	echo "<span class='title'>$miner[name]</span><br>\n";
	echo "<span class='description'>$miner[description]</span><br>\n";
	echo "<span class='description'>$miner[qualifier]</span><br>\n";
	echo "<span class='description'><br>Server(s):<br>";
	$server = sql_fetch("SELECT * FROM serverhandles WHERE serverhandles.id=$miner[servers]");
	echo $server["name"];
	echo "</span><br>\n";
	echo "<br>\n";
	table_schedule($miner['id']);
	echo "<br>\n";
	table_data($miner['id']);
	echo "<br>\n";
	echo "<table>\n";
	echo "<tr><th class='rowh'>Other Actions\n";
	list($count) = sql_fetch("SELECT COUNT(*) FROM data WHERE data.miner=$miner[id]");
	if($count)
		echo "<tr><td class='rowf'><a href='get.php?m=$miner[id]'><img src='img/get.png' class='icon'> Get All of this Miner's Data</a>\n";
	echo "<tr><td class='rowf'><a href='miner.php?i=$miner[id]&amp;a=e'><img src='img/edit.png' class='icon'> Edit this Miner</a>\n";
	echo "<tr><td class='rowf'><a href='miner.php?i=$miner[id]&amp;a=n'><img src='img/copy.png' class='icon'> Make a Copy of this Miner</a>\n";
	echo "<tr><td class='rowf'><a href='miner.php?i=$miner[id]&amp;a=d'><img src='img/drop.png' class='icon'> Delete this Miner</a>\n";
	echo "<tr><td class='rowf'><a href='miner.php'><img src='img/mine.png' class='icon'> Go to the Miner List</a>\n";
	echo "</table>\n";
}
else
	table_miners();

include('template_bottom.php');
?>