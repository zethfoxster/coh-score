

<?php
require_once('config.php');

$common_timeframes = array(
	array(
		'disp' => 'Ever',
		'col' => 'Ever',
	),
	array(
		'disp' => 'Since Monday',
		'col' => 'ThisWeek',
	),
	array(
		'disp' => 'Last Week',
		'col' => 'LastWeek',
	),
);
define('COMMON_DEF_TIMEFRAME',0);

$common_aggregation = array(
/*
	array(
		'disp' => 'Mean',
		'func' => 'SUM($)/COUNT(*)', // AVG() doesn't work, since we use NULL for 0
		'data' => true,
	),
	array(
		'disp' => 'Mean (No 0s)',
		'func' => 'AVG($)', // intentionally ignore NULLs, may be useful
		'data' => true,
	),
*/
	'average' => array(
		'disp' => 'Average',
		'func' => 'AVG(CAST(ISNULL($,0) as float))',
		'data' => true,
		'localin' => 'common_localagg_averagein',
		'localout' => 'common_localagg_averageout',
	),
	'sum' => array(
		'disp' => 'Total',
		'func' => 'SUM($)', // doesn't matter if it includes NULLs
		'data' => true,
		'localin' => 'common_localagg_sumin',
		'localout' => 'common_localagg_sumout',
	),
	'count' => array(
		'disp' => 'Count',
		'func' => 'COUNT(*)', // will include NULLs
		'data' => false,
		'localin' => 'common_localagg_countin',
		'localout' => 'common_localagg_countout',
	),
	'min' => array(
		'disp' => 'Minimum',
		'func' => 'MIN(ISNULL($,0))',
		'data' => true,
		'localin' => 'common_localagg_minin',
		'localout' => 'common_localagg_minout',
	),
	'max' => array(
		'disp' => 'Maximum',
		'func' => 'MAX(ISNULL($,0))',
		'data' => true,
		'localin' => 'common_localagg_maxin',
		'localout' => 'common_localagg_maxout',
	),
	'stdev' => array(
		'disp' => 'StdDev',
		'func' => 'STDEV(CAST(ISNULL($,0) as float))',
		'data' => true,
	),

//	Median requires a more complex query
//	Both of these are affected by our use of NULL for 0
//
//	array(
//		'disp' => 'Median',
//		'func' => 'HAHA_YEAH_RIGHT($)',
//	),
// TODO: add some sort of partitioning, perhaps as a grouping option.  e.g. '% of [group] where [data] > 0'
);

$common_comparison = array(
	'eq' => array(
		'disp' => '=',
		'func' => '$0 = $1',
		'human' => '$0 equals $1',
	),
	'ne' => array(
		'disp' => '&ne;',
		'func' => '$0 <> $1',
		'human' => '$0 does not equal $1',
	),
	'lt' => array(
		'disp' => '&lt;',
		'func' => '$0 < $1',
		'human' => '$0 is less than $1',
	),
	'le' => array(
		'disp' => '&le;',
		'func' => '$0 <= $1',
		'human' => '$0 is at most $1',
	),
	'gt' => array(
		'disp' => '&gt;',
		'func' => '$0 > $1',
		'human' => '$0 is greater than $1',
	),
	'ge' => array(
		'disp' => '&ge;',
		'func' => '$0 >= $1',
		'human' => '$0 is at least $1',
	),
);

function common_localagg_averagein(&$element,$input)
{
	@$element[0] += $input;
	@$element[1]++;
}
function common_localagg_averageout($element) { return $element ? $element[0]/$element[1] : false; }

function common_localagg_sumin(&$element,$input)	{ @$element += $input; }
function common_localagg_sumout($element)			{ return $element; }

function common_localagg_countin(&$element,$input)	{ @$element++; }
function common_localagg_countout($element)			{ return $element; }

function common_localagg_maxin(&$element,$input)	{ if(!isset($element) || $element < $input) $element = $input; }
function common_localagg_maxout($element)			{ return $element; }

function common_localagg_minin(&$element,$input)	{ if(!isset($element) || $element > $input) $element = $input; }
function common_localagg_minout($element)			{ return $element; }


$common_frequencies = array(
	0 => 'Once',
	24	=> 'Daily',
	168	=> 'Weekly',
	672 => 'Monthly',
);
define('COMMON_DEF_FREQUENCY',0);

function readable_frequency($frequency)
{
	global $common_frequencies;
	if(@$common_frequencies[$frequency])
		return $common_frequencies[$frequency];

	if($frequency <= 0)
		return 'Never';

	$r = array();
	if($frequency > 672)
	{
		$big = (int)($frequency/672);
		$r[] = "$big month".($big == 1 ? '' : 's');
		$frequency %= 672;
	}
	if($frequency > 168)
	{
		$big = (int)($frequency/168);
		$r[] = "$big week".($big == 1 ? '' : 's');
		$frequency %= 168;
	}
	if($frequency > 24)
	{
		$big = (int)($frequency/24);
		$r[] = "$big day".($big == 1 ? '' : 's');
		$frequency %= 24;
	}
	if($frequency > 0)
		$r .= "$frequency hour".($frequency == 1 ? '' : 's');

	return 'Every '.join(', ',$r);
}

function reformat_date($str)
{
	global $config_dateformat;
	return date($config_dateformat,strtotime($str));
}

function reformat_datetime($str)
{
	global $config_datetimeformat;
	return date($config_datetimeformat,strtotime($str));
}

function reformat_date_plus($str,$frequency,$total)
{
	global $config_dateformat;
	return date($config_dateformat,strtotime($str) + 3600*$frequency*max($total-1,0));
}

?>
