<?php
require_once('common_sql.php');
require_once('utils_sql.php');

if(@$_POST['lookup'])
{
	if(@$_POST['id'])
	{
		$pstring = sql_fetch("SELECT * FROM pstrings WHERE id=".sql_string($_POST['id']));
		if($pstring)
			$template_infobox[] = "<div class='label'>$_POST[id]</div>".nl2br(htmlspecialchars($pstring['string']));
		else
			$template_warnbox[] = "<div class='label'>$_POST[id]</div><div class='description'>No matching PString.</div>";
	}
	else
	{
		$template_errorbox[] = "Please enter a PString identifier.";
	}
}
else if(@$_POST['revlookup'])
{
	if(@$_POST['str'])
	{
		$query = new QueryIterator("SELECT * FROM pstrings WHERE string LIKE ".sql_string("%$_POST[str]%"));
		$info = "<table style='border-spacing:0'>";
		foreach($query as $v)
			$info .= "<tr><td style='border-top:white 1px solid;vertical-align:top'>$v[id]<td style='border-top:white 1px solid'>".nl2br(htmlspecialchars($v['string']));
		if(!$query->num_rows())
			$info .= "<tr><td class='description' colspan=2>No matching strings";
		$info .= "</table>";
		$template_infobox[] = $info;
	}
	else
	{
		$template_errorbox[] = "Please enter a translated substring.";
	}
}
else if(@$_POST['upload'])
{
	if(@$_FILES['file']['tmp_name'])
	{
		$count = add_ms_file($_FILES['file']['tmp_name']);
		unlink($_FILES['file']['tmp_name']);
		$template_infobox[] = "$count PStrings added/updated.";
	}
	else
	{
		$template_errorbox[] = "Please choose a PString file to upload.";
	}
}

$template_title = 'Admin: PStrings';
$template_admin_menu = true;
include('template_top.php');

echo "<form method=post enctype='multipart/form-data'>\n";
echo "<input type=hidden name='MAX_FILE_SIZE' value='".bytes_shorthand_to_int(ini_get('upload_max_filesize'))."'>\n";
echo "<table>\n";
echo "<tr><th class='rowh'>Actions\n";
echo "<tr><td class='row0'><div class='label'>Identifier</div><input type=text name='id'> <input class='butsearch' type=submit name='lookup' value='Lookup PString'>\n";
echo "<tr><td class='row1'><div class='label'>Substring</div><input type=text name='str'> <input class='butsearch' type=submit name='revlookup' value='Lookup Translated Substring'>\n";
echo "<tr><td class='row0'><div class='label'>PString File</div><input type=file name='file'>\n";
echo "		<div class='formrow'><input class='butnew' type=submit name='upload' value='Add PStrings'></div>\n";
echo "<tr><td class='rowf'><a href='admin.php'><img src='img/admin.png' class='icon'> Go to the Admin Menu</a>\n";
echo "</table>\n";
echo "</form>\n";

include('template_bottom.php');
?>