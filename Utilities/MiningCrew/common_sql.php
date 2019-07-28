<?php
require_once('mssql.php');
require_once('config.php');

function sql_server_connect($server,$fail=false) // takes a row from the servers database
{
	return sql_db_connect($server['loginaddr'],$server['loginuser'],$server['loginpass'],$server['logindb'],$fail);
}

function common_sql_error_include()
{
	extract($GLOBALS);
	$filename = 'template_top.php';
	if(file_exists($filename))
		include($filename);
}

sql_fatal_error_call_if_no_output('common_sql_error_include');

// db identifier doesn't need to be recorded, it should always be 0, which is the default for all of the functions
if(sql_db_connect($config_mssql_server,$config_mssql_user,$config_mssql_pass,$config_mssql_db) === false)
	sql_fatal_error("Could not connect to the primary SQL server or select the datamining database.");

?>