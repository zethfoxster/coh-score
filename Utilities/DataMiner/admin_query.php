<?php
// WARNING: this whole thing's basically a hack
require_once('common_forms.php');
require_once('utils_sql.php');

if(@$_POST['new'] && @$_POST['query'])
{
	$category = $_POST['category'] == '0' ? @$_POST['category_other'] : $_POST['category'];
	if($category)
	{	
		$server = sql_fetch("SELECT handle FROM servers WHERE name='All Servers'");
		
		sql_query(
			"INSERT INTO miners (name,category,servers,description,emailnames,emailaddr,customheaders,customquery) VALUES ("
			.sql_string(@$_POST['name']).", "
			.sql_string($category).", "
			.sql_string($server['handle']).", "
			.sql_string($_POST['description']).", "
			.sql_string(join("\n",@$_POST['emailval_query'])).", "
			.sql_string(join("\n",@$_POST['emailaddr_query'])).", "
			.sql_string(join("\n",@$_POST['column_query'])).", "
			.sql_string($_POST['query']).")"
			);
		list($minerid) = sql_fetch("SELECT SCOPE_IDENTITY()");
		
		// if start is some time in the past, assume now
		if( @$_POST['frequency_'] )
		{
			$start = time();
			$desired = 0;
	
			sql_query("INSERT INTO schedules (miner,frequency,start,desired) VALUES ("
			."$minerid,"
			.@$_POST['frequency_'].", "
			."GETDATE(), "
			."$desired)" );
			list($schedule) = sql_fetch("SELECT SCOPE_IDENTITY()");
		}	

		// TODO: error checking
		goto_page("$config_urlroot/miner.php?i=$minerid");
	}
}

$template_title = 'Admin: Manual Query';
$template_admin_menu = true;


include('template_top.php');

echo "<form action=admin_query.php class='newbox' method=post>\n";


echo form_mining_prolog();

echo "<div class='formrow'><div class='label'>Name</div><input type=text name='name' value=''></div>\n";
echo "<div class='formrow'><div class='label'>Description:</div><textarea name='description' cols=100 rows=3>$description</textarea></div>\n";
echo "<div class='formrow'><div class='label'>Category:</div>".select_miner_category('category',$category)."</div>\n";
echo "<div class='formrow'><div class='label'>Frequency</div>".select_frequency("")."</div>\n";
echo "<div class='formrowlong' id='query.email'><div class='label'><a href='javascript:' onClick='addline(\"query\",\"email\");'><img src='img/listadd.png' title='Add Email' class='listicon'></a> Email Notification</div></div>\n";
echo "<div class='formrowlong' id='query.column'><div class='label'><a href='javascript:' onClick='addline(\"query\",\"column\");'><img src='img/listadd.png' title='Add Column Headers' class='listicon'></a> Column Headers</div></div>\n";
echo "<div class='formrowlong'><div class='label'>Query</div><textarea name='query' cols=100 rows=3></textarea></div>\n";
echo "<div class='formrowlong'>"
."<input type=submit class='butnew' name='new' value='Add'> "
."<input type=reset  class='butcancel' value='Reset'> "
."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
."</div>\n";
echo "</form>\n";

include('template_bottom.php');
?>