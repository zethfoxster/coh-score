<?php
require_once('config.php');
// disable browser caching
header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
header('Cache-Control: no-cache, must-revalidate');
header('Pragma: no-cache');
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<html lang='en-us'>

<head>
	<title><?php echo $config_title.($template_title ? ": $template_title" : ''); ?></title>
	<meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-5'>
	<meta http-equiv='Content-Script-Type' content='text/javascript'>
	<meta http-equiv='Content-Style-Type' content='text/css'>
	
<?php /* TODO: use this <link rel='Copyright' href='<?php echo $config_urlroot; ?>/copyright.php'> */ ?>
<?php /* TODO: use this <link rel='Shortcut Icon' href='<?php echo $config_urlroot; ?>/favicon.ico'> */ ?>

	<style type='text/css'><!-- @import "<?php echo $config_urlroot; ?>/style.css"; --></style>

	<script type="text/javascript">
		function toggle(table_name)
		{
			var theTable = (document.getElementById(table_name));
			var trs = theTable.getElementsByTagName('tr');
			for(var i = 0; i < trs.length; i++) 
			{
				var tr = trs[i];
				if( tr.className == 'hidden' )
				{
					if ((tr.style.display=="")||(tr.style.display== ""))
						tr.style.display="none";
					else
						tr.style.display="";
				}
			}
		}
	</script>
</head>



<body>
<table id='main_table'>
	<tr>
		<td id='main_menu' width=200>

			<table id="mainmenut_table" width="100%">
				<tr><th class='rowt'>Main Menu</th></tr>
			</table>

			<table id="common_table" >
				<tr><th class='rowh'  width="200"><a href="javascript:toggle('common_table');" id="expander">Common Queries</a></th></tr>
				<tr class='hidden' style="display:block;"><td class='rowf' width="200" ><a href='statisticaldata.php?a=first'><img src='img/new.png' class='icon'>Statistical Data</a></td></tr>
				<tr class='hidden' style="display:block;"><td class='rowf' width="200" ><a href='singletonqueries.php?a=first'><img src='img/go.png' class='icon'>Specific Elements</a></td></tr>
			</table>

			<table id="daily_table" >
				<tr><th class='rowh' width="200"><a href="javascript:toggle('daily_table');" id="expander">Daily Queries</a></th></tr>
				<tr class='hidden' style="display:block;"><td class='rowf' width="200"><a href='daily.php?a=first'><img src='img/new.png' class='icon'>Daily Info</a></td></tr>
				<tr class='hidden' style="display:block;"><td class='rowf' width="200"><a href='alerts.php?a=first'><img src='img/go.png' class='icon'>Alerts</a></td></tr>
			</table>

			<table id="mining_table">
				<tr><th class='rowh' width="200"><a href="javascript:toggle('mining_table');" id="expander">Miner Construction</a></th></tr>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='miner.php?a=n'><img src='img/new.png' class='icon'> Create a Miner</a></td></tr>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='onetime.php'><img src='img/go.png' class='icon'> One Time Mine</a></td></tr>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='miner.php'><img src='img/mine.png' class='icon'> Miner List</a></td></tr>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='schedule.php'><img src='img/sched.png' class='icon'> Full Schedule</a></td></tr>
			</table>

			<table id="settings_table">
				<tr><th class='rowh' width="200"><a href="javascript:toggle('settings_table');" id="expander">Settings</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_servers.php'><img src='img/server.png' class='icon'> Servers</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_query.php'><img src='img/query.png' class='icon'> Manual Query</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_containers.php'><img src='img/container.png' class='icon'> Containers</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_defs.php'><img src='img/def.png' class='icon'> Defs</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_pstrings.php'><img src='img/pstring.png' class='icon'> PStrings</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_attributes.php'><img src='img/pstring.png' class='icon'> Attributes</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_ss2000.php'><img src='img/ss2000.png' class='icon'> SS2000</a>
				<tr class='hidden' style="display:none;"><td class='rowf' width="200"><a href='admin_errors.php'><img src='img/errors.png' class='icon'> Errors</a>
			</table>
	</td>
	<td id='main_body' align=left>
		<table id='body_table' align=left>
			<tr align=left>
				<td width=1200 align=left>
	
					<?php
					
					foreach($template_errorbox as $v)
						echo "<div class='errorbox' align=left><div class='boxicon'><img src='img/errorbox.png' class='boxicon'></div>$v</div>\n";
					foreach($template_warnbox as $v)
						echo "<div class='warnbox'><div class='boxicon'><img src='img/warnbox.png' class='boxicon'></div>$v</div>\n";
					foreach($template_infobox as $v)
						echo "<div class='infobox'><div class='boxicon'><img src='img/infobox.png' class='boxicon'></div>$v</div>\n";
					?>
	
			 
			<!--  Content -->

		
		