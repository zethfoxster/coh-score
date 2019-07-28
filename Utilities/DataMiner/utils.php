<?php

// Useful functions that don't require anything special //////////////////////

function goto_page($url)
{
	header("Location: $url");
	exit();
}

function bytes_shorthand_to_int($val)
{
	$r = (int)$val;
	switch(strtolower(substr(trim($val),-1)))
	{
		case 'g':
			$r *= 1024;
		case 'm':
			$r *= 1024;
		case 'k':
			$r *= 1024;
	}
	return $r;
}

function format_csv($array)
{
	$first = false;
	$r = '';
	foreach($array as $v)
	{
		if($first)
			$r .= ',';
		if(strcspn($v,",\"\n") != strlen($v) || trim($v) != $v)
			$r .= '"'.str_replace('"','""',$v).'"';
		else
			$r .= $v;
		$first = true;
	}
	if($r === '' && count($array))
		$r = '""';
	$r .= "\n";
	return $r;
}

function join_humanreadable($array)
{
	$r = '';
	switch(count($array))
	{
		default:
			while(count($array) > 2)
				$r .= array_shift($array).', ';
		case 2:
			$r .= array_shift($array).' and ';
		case 1:
			$r .= array_shift($array);
		case 0:
	}
	return $r;
}

function unjoin($delim,$str)
{
	return $str == '' ? array() : explode($delim,$str);
}

?>
