<?php
require_once('common.php');
require_once('mssql.php');

function select_miner($name = '', $default = false)
{
	$category = '';
	$options = array();

	foreach(new QueryIterator("SELECT id, category, name FROM miners ORDER BY category, name") as $k => $v)
		$options[$v['category']][] = "<option value=$k".($k == $default ? ' selected' : '').">".htmlspecialchars($v['name'],ENT_QUOTES);

	$r = "<select name='$name'>";
	foreach($options as $k => $v)
		$r .= "<optgroup label='".htmlspecialchars($k,ENT_QUOTES)."'>".join('',$v)."</optgroup>\n";
	$r .= "</select>";

	return $r;
}

function select_miner_category($name = '', $default = false)
{
	$selected = false;

	$r = "<select name='$name' onChange='this.nextSibling.style.display=(this.options[this.selectedIndex].value==0)?\"inline-block\":\"none\";'>";
	$r .= "<option value='0'>Other (Specify)";
	foreach(new QueryIterator("SELECT DISTINCT category FROM miners") as $k => $v)
	{
		$r .= "<option value='".htmlspecialchars($k,ENT_QUOTES)."'";
		if($k == $default)
		{
			$r .= ' selected';
			$selected = true;
		}
		$r .= ">".htmlspecialchars($k);
	}
	$r .= "</select>";
	$r .= "<input type=text id='${name}_other' name='${name}_other' value='".htmlspecialchars($default,ENT_QUOTES)."'".($selected ? " style='display:none'" : '').">";
	return $r;
}

function select_server($name = '', $default = false) // hacked to make training room the default
{
	$r = "<select name='$name' id='$name'>";
	$r .= "<optgroup label='Groups'>";
	foreach(new QueryIterator("SELECT id, name FROM serverhandles WHERE single=0 AND hide=0 ORDER BY name") as $k => $v)
		$r .= "<option value=$k".($k == $default ? ' selected' : '').">".htmlspecialchars($v['name'],ENT_QUOTES);
	$r .= "</optgroup>";
	$r .= "<optgroup label='Individual'>";
	foreach(new QueryIterator("SELECT id, name FROM serverhandles WHERE single=1 AND hide=0 ORDER BY name") as $k => $v)
		$r .= "<option value=$k".(($k == $default || !$default && $v['name'] == 'Training Room') ? ' selected' : '').">".htmlspecialchars($v['name'],ENT_QUOTES);
	$r .= "</optgroup>";
	$r .= "</select>";
	return $r;
}

function select_timeframe($name = '', $default = COMMON_DEF_TIMEFRAME)
{
	global $common_timeframes;

	$r = "<select name='$name' id='$name'>";
	foreach($common_timeframes as $k => $v)
		$r .= "<option value=$k".($k == $default ? ' selected' : '').">$v[disp]";
	$r .= "</select>";
	return $r;
}

function select_frequency($name = '', $default = COMMON_DEF_FREQUENCY)
{
	global $common_frequencies;

	$selected = false;
	if(!is_int($default))
		$default = COMMON_DEF_FREQUENCY;

	$r = "<select name='$name' onChange='this.nextSibling.style.display=(this.options[this.selectedIndex].value==0)?\"inline-block\":\"none\";'>";
	$r .= "<option value=0>Other (In Hours)";
	foreach($common_frequencies as $k => $v)
	{
		if($k < 24)
			continue;
		$r .= "<option value=$k";
		if($k == $default)
		{
			$r .= ' selected';
			$selected = true;
		}
		$r .= ">$v";
	}
	$r .= "</select>";
	$r .= "<input type=text id='${name}_other' name='${name}_other' value='$default' size=4".($selected?" style='display:none'":'').">\n";
	return $r;
}

function select_schedule_start($name = '', $default = 0)
{
	$r = "<select name='$name' onChange='this.nextSibling.style.display=(this.options[this.selectedIndex].value==\"now\")?\"none\":\"inline-block\";'>";
	$r .= "<option value='now'>Start Now";
	$r .= "<option value='date'".($default ? ' selected' : '').">Specify a Date";
	$r .= "</select>";
	$r .= "<input type=text id='${name}_other' name='${name}_other'".($default ? (" value='".htmlspecialchars(reformat_date($default),ENT_QUOTES)."'") : " style='display:none'").">\n";
	return $r;
}

function select_schedule_finish($name = '', $default = 0)
{
	$count = (int)$default;
	$date = !$count && $default;

	$r = "<select name='$name' onChange='var v=this.options[this.selectedIndex].value;this.nextSibling.style.display=(v==\"never\"||v==\"now\")?\"none\":\"inline-block\";'>";
	$r .= "<option value='never'>Continue Forever";
	$r .= "<option value='count'".($count ? ' selected' : '').">Specify a Count";
	$r .= "<option value='date'>".($date ? ' selected' : '')."Specify a Date";
	$r .= "<option value='now'>Stop Now";
	$r .= "</select>";
	$r .= "<input type=text id='${name}_other' name='${name}_other'";
	if($count)
		$r .= " value='$default'";
	else if($date)
		$r .= " value='".htmlspecialchars(reformat_date($default),ENT_QUOTES)."'";
	else
		$r .= " style='display:none'";

	return $r;
}

$common_form_containers = false;

function form_mining_prolog()
{
	global $common_aggregation, $common_comparison, $common_form_containers;
	$r = '';

	if($common_form_containers)
		die("Error: form_mining_prolog() called twice");
	$common_form_containers = array();

	$templates = array();
	$templated = array();
	foreach(new QueryIterator("SELECT * FROM containers WHERE groupok=1 OR dataok=1 OR whereok=1 ORDER BY container") as $v)
	{
		if(substr($v['tablename'],0,7) == 'mineacc')
		{
			$v['container'] .= '.%'; // FIXME: make this naming convention pervasive?
			$tablename = ucfirst($v['container']);
		}
		else
		{
			$templated[$v['container']] = true;
			$tablename = substr($v['tablename'],0,strcspn($v['tablename'],'0123456789'));
		}
		if(@!$templates[$v['container']])
			$templates[$v['container']] = array();
		if(@!$templates[$v['container']][$tablename])
			$templates[$v['container']][$tablename] = array();
		$templates[$v['container']][$tablename][$v['name']] = $v;

		$common_form_containers[$v['container']] = true;
	}

	// sorts columns by name, with tablename## merged
	foreach($templates as $k => $v)
		foreach($v as $l => $w)
		{
			ksort($w);
			$templates[$k][$l] = $w;
		}

// JavaScript Functions
	$r .= "<script><!--\n";

	//  aggregator_changed(asel)
	$r .= "function aggregator_changed(asel) {\n";
	$r .= "	var elems = asel.parentNode.childNodes;\n";
	$r .= " var dsel;\n";
	$r .= "	for(var i = 0; i < elems.length; i++)\n";
	$r .= "		if(elems[i].name.substr(0,5) == 'data_') {\n";
	$r .= "			dsel = elems[i];\n";
	$r .= "			break;\n";
	$r .= "		}\n";
	$a = array();
	foreach($common_aggregation as $k => $v)
		if(!$v['data'])
			$a[] = "asel.options[asel.selectedIndex].value == '$k'";
	if($a)
	{
		$r .= "	if(".join(' || ',$a).")\n";
		$r .= "		dsel.style.display=\"none\";\n";
		$r .= "	else\n";
		$r .= "	";
	}
	$r .= "	dsel.style.display=\"inline\";\n";
	$r .= "}\n";
	$r .= "\n";

	// clear_lines(name,type)
	$r .= "function clearlines(name,type) {\n";
	$r .= "	var elems = document.getElementById(name+'.'+type).childNodes;\n";
	$r .= "	for(var i = 0; i < elems.length; i++)\n";
	$r .= "		if(elems[i].id == 'line') {\n";
	$r .= "			elems[i].parentNode.removeChild(elems[i]);\n";
	$r .= "			i--;\n";
	$r .= "		}\n";
	$r .= "}\n";
	$r .= "\n";

	// add_line(name,type)
	$r .= "function addline(name,type) {\n";
	$r .= "	var csel = document.getElementById(name+'.c');\n";
	$r .= "	var container = csel ? csel.options[csel.selectedIndex].value : '';\n";
	$r .= "	var line = document.getElementById(type+'line_'+container).cloneNode(true);\n";
	$r .= "	line.id = 'line';\n";
	$r .= "	line.style.display = 'block';\n";
	$r .= "	var elems = line.childNodes;\n";
	$r .= " for(var i = 0; i < elems.length; i++)\n";
	$r .= "		if(elems[i].name)\n";
	$r .= "			elems[i].name += name+'[]';\n";
	$r .= "	document.getElementById(name+'.'+type).appendChild(line);\n";
	$r .= "}\n";
	$r .= "\n";

	// container_changed(name)
	$r .= "function container_changed(name) {\n";
	$r .= "	var csel = document.getElementById(name+'.c');\n";
	$r .= "	var container = csel.options[csel.selectedIndex].value;\n";
//	$r .= "	document.getElementById(name+'.t').style.display = (container.substr(-2) == '.%') ?  'block' : 'none';\n";
	$r .= "	document.getElementById(name+'.t').style.display = (container.lastIndexOf('.%') == container.length-2) ?  'block' : 'none';\n";

	$r .= "	clearlines(name,'data');\n";
	$r .= "	clearlines(name,'group');\n";
	$r .= "	clearlines(name,'where');\n";
	$r .= "	addline(name,'data');\n";
//	$r .= "	document.getElementById(name+'.where').style.display = (container.substr(-2) == '.%') ?  'none' : 'block';\n"; // FIXME: constraints for mineaccs
	$r .= "	document.getElementById(name+'.where').style.display = (container.lastIndexOf('.%') == container.length-2) ?  'none' : 'block';\n"; // FIXME: constraints for mineaccs
	$r .= "}\n";
	$r .= "\n";

	$r .= "--></script>\n";

// Line Templates
	foreach($templates as $container => $v)
	{
		// Data Line
		$r .= "<div id='dataline_${container}' style='display:none'>";
		$r .= "<a href='javascript:' onClick='this.parentNode.parentNode.removeChild(this.parentNode);'><img src='img/listrem.png' title='Remove Data' class='listicon'></a>";

		$r .= "<select name='aggregation_' onChange='aggregator_changed(this);'></div>";
		foreach($common_aggregation as $aggk => $aggv)
			if(isset($aggv['localin'], $aggv['localout']) || substr($container,-2) != '.%')
				$r .= "<option value='$aggk'>$aggv[disp]";
		$r .= "</select>";

		$r .= "<select name='data_'>";
		foreach($v as $tablename => $w)
		{
			$r2 = '';
			foreach($w as $field)
				if($field['dataok'])
					$r2 .= "<option value='$field[id]' title='".htmlspecialchars($field['description'])."'>$field[name]";
			if($r2)
				$r .= "<optgroup label='$tablename'>$r2</optgroup>";
		}
		$r .= "</select>";

		$r .= "</div>\n";

		// Group Line
		$r .= "<div id='groupline_${container}' style='display:none'>";
		$r .= "<a href='javascript:' onClick='this.parentNode.parentNode.removeChild(this.parentNode);'><img src='img/listrem.png' title='Remove Grouping' class='listicon'></a>";

		$r .= "<select name='group_'>";
		foreach($v as $tablename => $w)
		{
			$r2 = '';
			foreach($w as $field)
				if($field['groupok'])
					$r2 .= "<option value='$field[id]'>".($field['name']=='.Level' ? 'Level' : $field['name']);
			if($r2)
				$r .= "<optgroup label='$tablename'>$r2</optgroup>";
		}
		$r .= "</select>";

		$r .= "</div>\n";

		// Constraint Line
		$r .= "<div id='whereline_${container}' style='display:none'>";
		$r .= "<a href='javascript:' onClick='this.parentNode.parentNode.removeChild(this.parentNode);'><img src='img/listrem.png' title='Remove Constraint' class='listicon'></a>";

		$r .= "<select name='where_'>";
		foreach($v as $tablename => $w)
		{
			$r2 = '';
			foreach($w as $field)
				if($field['whereok'])
					$r2 .= "<option value='$field[id]'>$field[name]";
			if($r2)
				$r .= "<optgroup label='$tablename'>$r2</optgroup>";
		}
		$r .= "</select>";

		$r .= "<select name='wherecmp_'></div>";
		foreach($common_comparison as $cmpk => $cmpv)
			$r .= "<option value='$cmpk'>$cmpv[disp]";
		$r .= "</select>";

		$r .= "<input type=text name='whereval_'>";
		$r .= "</div>\n";
	}

	// Column Name Line
	$r .= "<div id='columnline_' style='display:none'>";
	$r .= "<a href='javascript:' onClick='this.parentNode.parentNode.removeChild(this.parentNode);'><img src='img/listrem.png' title='Remove Constraint' class='listicon'></a>";
	$r .= "<input type=text name='column_'>";
	$r .= "</div>\n";

	return $r;
}

function form_mining($name, $default_container = '', $default_field = '')
{
	global $common_form_containers;
	$r = '';

	if(!$common_form_containers)
		die("Error: form_mining_prolog() must be called before form_mining()");

	$r .= "<div class='formrow'><div class='label'>Server(s)</div>".select_server("server_$name")."</div>\n";
	$r .= "<div class='formrow'><div class='label'>Container</div>\n";
	$r .= "<select name='container_$name' id='$name.c' onChange='container_changed(\"$name\");'>";
	foreach($common_form_containers as $k => $v)
		$r .= "<option value='$k'".($k == $default_container ? ' selected' : '').">".ucfirst(substr($k,-2) == '.%' ? substr($k,0,-2) : $k);
	$r .= "</select>\n";
	$r .= "</div>\n";

	$r .= "<div class='formrow' id='$name.t'><div class='label'>Time Frame</div>".select_timeframe("timeframe_$name")."</div>\n";
	$r .= "<div class='formrowlong' id='$name.data'><div class='label'><a href='javascript:' onClick='addline(\"$name\",\"data\");'><img src='img/listadd.png' title='Add Data' class='listicon'></a> Data <span class='description'>(columns)</span></div></div>\n";
	$r .= "<div class='formrowlong' id='$name.group'><div class='label'><a href='javascript:' onClick='addline(\"$name\",\"group\");'><img src='img/listadd.png' title='Add Grouping' class='listicon'></a> Groupings <span class='description'>(rows)</span></div></div>\n";
	$r .= "<div class='formrowlong' id='$name.where'><div class='label'><a href='javascript:' onClick='addline(\"$name\",\"where\");'><img src='img/listadd.png' title='Add Constraint' class='listicon'></a> Constraints</div></div>\n";

	$r .= "<script><!--\n";
	$r .= "container_changed('$name');\n";
	$r .= "--></script>\n";
	return $r;
}

?>