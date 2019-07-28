<?php

function error_back_trace($shift = 1)
{
	$reporting = error_reporting(0);

	$r = '';
	foreach(array_slice(debug_backtrace(),$shift) as $v)
	{
		if($v['file'] || $v['line'])
			$r .= "$v[file]($v[line])\n";
		if($v['function'])
		{
			$args = array();
			if($v['args'])
				foreach($v['args'] as $w)
					$args[] = preg_replace('|\\s+|',' ',var_export($w,true));

			if($v['class'])
				$r .= "$v[class]$v[type]";
			$r .= "$v[function](".join(',',$args).")\n";
		}
		// FIXME: dump object if available
		$r .= "\n";
	}

	error_reporting($reporting);
	return $r;
}

?>