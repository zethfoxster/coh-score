<?php
// External Interface ////////////////////////////////////////////////////////

function sql_db_connect($addr, $user, $pass, $db, $fail = false)
// returns a reference to be passed to other functions for the parameter $db
{
	global $mssql_dbs;

	//$link = odbc_connect("Driver={SQL Server};Server={$addr};Database={$db}",$user,$pass);
	$link = @mssql_connect($addr,$user,$pass);
	$mssql_db = array('addr' => $addr, 'link' => $link, 'db' => $db);
	if($link === false)
	{
		if($fail)
			sql_fatal_error("Could not connect to server", $mssql_db);
		else
			return false;
	}
	if(!sql_selectdb($link, $fail))
		return false;

	$mssql_dbs[] = $mssql_db;
	return count($mssql_dbs)-1;
}

function sql_query($query, $db = 0)
// returns a result that can be used with php's builtin mssql_* functions
// this function is mostly for queries that don't return anything
// in general you should use sql_fetch if you want something back
{
	global $mssql_dbs;
// nice for debugging, all queries go through here
//include_once('error.php');
//echo "<div class='mssql_error'>SQL Query<br>".($query ? "<span class='mssql_error_query'>".nl2br($query)."</span><br>" : "")."<div class='mssql_error_trace'>\n\n".error_back_trace(2)."</div></div>";

	if($db===false)
		sql_fatal_error("Invalid database.");
	if(!sql_selectdb($mssql_dbs[$db]))
		sql_fatal_error("Could not connect to server", $mssql_db[$db]);

	$result = @mssql_query($query,$mssql_dbs[$db]['link']);
	if($result === false)
		sql_fatal_error("Database query failed.", $mssql_dbs[$db], $query);
	return $result;
}

function sql_fetch($query, $db = 0, $result_type = MSSQL_BOTH)
// returns an array for the first result, only
{
	return mssql_fetch_array(sql_query($query,$db),$result_type);
}

function sql_fetch_all($query, $db = 0, $result_type = MSSQL_BOTH)
// returns an array of arrays, with an entry for each result row
// in general you should use a QueryIterator instead
{
	$all = array();
	$result = sql_query($query,$db);
	while(($row = mssql_fetch_array($result,$result_type)) !== false)
		$all[] = $row;
	return $all;
}

function sql_string($string)
// returns the given string escaped and quoted for entry in a query
{
	return "'".str_replace("'","''",$string)."'";
}

function sql_date($string = '')
// returns the given date formatted and quoted for entry in a query
// defaults to the current date according to the server
{
	if($string)
		return date("'Y-m-d H:i:s.000'",strtotime($string));
	else
		return 'GETDATE()';
}

class QueryIterator implements Iterator
// To be used in a foreach loop
// The query is performed once per loop, generating one result array per iteration
// qi->num_rows() can be used to determine the number of results in a set after the loop has been started
// e.g. foreach(new QueryIterator("SELECT * FROM `mytable`") as $key => $value) ...
// Keys are simply the first column in a result
{
	function __construct($query, $db = 0, $result_type = MSSQL_BOTH)
	{
		$this->query = $query;
		$this->result = false;
		$this->value = false;
		$this->db = $db;
		$this->type = $result_type;
	}

	function rewind()
	{
		$this->result = sql_query($this->query,$this->db);
		$this->next();
	}

	function current()	{ return $this->value; }
	function key()		{ return reset($this->value); }
	function next()		{ return $this->value = mssql_fetch_array($this->result,$this->type); }
//	function next()		{ return $this->value = $this->result === true ? false : mssql_fetch_array($this->result,$this->type); }
	function valid()	{ return $this->value !== false; }

	// this function returns false until the object has been rewound
	function num_rows()	{ return $this->result === false ? false : mssql_num_rows($this->result); }

	private $query;
	private $result;
	private $value;
	private $db;
	private $type;
}

function sql_fatal_error_call_if_no_output($function)
{
	global $mssql_error_user_func;
	if(function_exists($function))
		$mssql_error_user_func = $function;
}

// Internal Stuff ////////////////////////////////////////////////////////////

$mssql_dbs = array();
$mssql_error_user_func = false;

function sql_selectdb($db, $fail = true)
{
	if(	!$db ||
		($db['db'] && !@mssql_select_db($db['db'],$db['link'])) )
	{
		if($fail)
			sql_fatal_error("Could not select database", $db);
		else
			return false;
	}
	return true;
}

function sql_fatal_error($message,$db=false,$query='')
{
	global $mssql_error_user_func;
	include_once('error.php');

//	flush();
//	if($mssql_error_user_func && !headers_sent())
	if($mssql_error_user_func && !ob_get_length() && !headers_sent())
		$mssql_error_user_func();

	$last = mssql_get_last_message();
	
	    $sMssql_get_last_message = mssql_get_last_message();
        $sQuery_added  = "BEGIN TRY\n";
        $sQuery_added .= "\t".$query."\n";
        $sQuery_added .= "END TRY\n";
        $sQuery_added .= "BEGIN CATCH\n";
        $sQuery_added .= "\tSELECT 'Error: '  + ERROR_MESSAGE()\n";
        $sQuery_added .= "END CATCH";
        $rRun2= mssql_query($query, $db);
        $aReturn = mssql_fetch_assoc($rRun2);
        if(empty($aReturn))
        {
            echo $sError.'. MSSQL returned1: '.$sMssql_get_last_message.'.<br>Executed query: '.nl2br($query);
        }
        elseif(isset($aReturn['computed']))
        {
            echo $sError.'. MSSQL returned2: '.$aReturn['computed'].'.<br>Executed query: '.nl2br($query);
        }

	$err =	"<div class='mssql_error'>".
			"Error: <span class='mssql_error_error'>$message</span><br>".
			($db ? "Database: <span class='mssql_error_db'>$db[addr] $db[db]</span><br>" : "").
			($last ? "Last Reply: <span class='mssql_error_message'>$last</span><br>" : "").
			($query ? "Query:<br><span class='mssql_error_query'>$query</span><br>" : "").
			"<div class='mssql_error_trace'>\n\n".error_back_trace(2)."</div>".
			"</div>";

	@file_put_contents("errors/".time().".html", $err);
	die($err);
}

?>