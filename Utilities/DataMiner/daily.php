<?php
require_once('common_tables.php');
require_once('common_forms.php');
require_once('utils_sql.php');

if(@$_GET['a'] == 'first')
{
	$template_infobox[] = "Queries on this page are run daily.";
}

include('template_top.php');



?>