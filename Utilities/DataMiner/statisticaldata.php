<?php
require_once('common_tables.php');
require_once('common_forms.php');
require_once('utils_sql.php');

if(@$_GET['a'] == 'first')
{
	$template_infobox[] = "This page contains common statistical and aggregate queries.";
}

include('template_top.php');



?>