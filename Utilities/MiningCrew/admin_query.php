<?php
// WARNING: this whole thing's basically a hack
require_once('common_forms.php');
require_once('utils_sql.php');

if(@$_POST['new'] && @$_POST['query'])
{
//	$category = $_POST['category'] == '0' ? @$_POST['category_other'] : $_POST['category'];
//	if(!$category)
//		$category = "Uncategorized";

	sql_query("INSERT INTO miners (name,category,qualifier,description,servers,dataheader,aggregation,grouplocally,groupheader,groupids,whereheader,query) VALUES ("
		.sql_string(@$_POST['name']?$_POST['name']:$_POST['query']).", "
		."'Manual Queries', " // .$category.", "
		.sql_string($_POST['query']).", "
		."'', "
		."$_POST[server], "
		.sql_string(join("\n",@$_POST['column_query'])).", "
		."'', "
		."0, "
		."'', "
		."'', "
		."'', "
		.sql_string($_POST['query']).")");
	list($minerid) = sql_fetch("SELECT SCOPE_IDENTITY()");

	// TODO: error checking
	goto_page("$config_urlroot/miner.php?i=$minerid");
}

$template_title = 'Admin: Manual Query';
$template_admin_menu = true;
include('template_top.php');

// TODO: allow modification of existing miners here
echo form_mining_prolog();
echo "<form class='newbox' method=post>\n";
echo "<div class='formrow'><div class='label'>Name</div><input type=text name='name' value=''></div>\n";
//echo "<div class='formrow'><div class='label'>Category:</div>".select_miner_category('category')."</div>\n";
echo "<div class='formrow'><div class='label'>Server(s)</div>".select_server('server')."</div>\n";
echo "<div class='formrowlong' id='query.column'><div class='label'><a href='javascript:' onClick='addline(\"query\",\"column\");'><img src='img/listadd.png' title='Add Column Headers' class='listicon'></a> Column Headers</div></div>\n";
echo "<div class='formrowlong'><div class='label'>Query</div><textarea name='query' cols=30 rows=3></textarea></div>\n";
echo "<div class='formrowlong'>"
	."<input type=submit class='butnew' name='new' value='Add'> "
	."<input type=reset  class='butcancel' value='Reset'> "
	."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
	."</div>\n";
echo "</form>\n";

include('template_bottom.php');
?>