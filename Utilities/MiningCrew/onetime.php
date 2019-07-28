<?php
require_once('common_forms.php');
require_once('utils_sql.php');

if(@$_POST['new'])
{
	$minerid = create_miner(
		false,
		'One Time Mines',
		'',
		$_POST['server_onetime'],
		$_POST['container_onetime'],
		@$_POST['timeframe_onetime'],
		@$_POST['aggregation_onetime'],
		@$_POST['data_onetime'],
		@$_POST['group_onetime'],
		@$_POST['where_onetime'],
		@$_POST['wherecmp_onetime'],
		@$_POST['whereval_onetime'],
		false
	);

	$dataid = collect_data($minerid);

	// TODO: error checking
	goto_page("$config_urlroot/get.php?d=$dataid");
}

$template_infobox[] = "The data you retrieve will automatically be named and saved.";

include('template_top.php');

echo form_mining_prolog();
echo "<form class='newbox' method=post>\n";
echo form_mining('onetime','Ents');
echo "<div class='formrowlong'>"
	."<input type=submit class='butnew' name='new' value='Collect'> "
	."<input type=reset  class='butcancel' value='Reset'> "
	."<input type=submit class='butcancel' name='cancel' value='Cancel'>"
	."</div>\n";
echo "</form>\n";

include('template_bottom.php');
?>