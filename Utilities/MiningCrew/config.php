<?php

$config_title = 'CoX Mining Crew';
$config_urlroot = 'http://ncncxptools.ncnc.ncus';
$config_docroot = 'C:/wwwroot';
$config_dateformat = 'M j, Y'; // Since we only get mirrors daily (was 'M j, y g:ia')
$config_datetimeformat = 'M j, Y g:ia';
$config_compressionlevel = 9;

// Connection info for the server holding mined data and configuration
$config_mssql_server = 'ncncsql01';
$config_mssql_user = 'ncncsa';
$config_mssql_pass = 'nuDNA752dR';
$config_mssql_db = 'coh_datamine';

// These can be overridden by individual pages to alter the page template
$template_title = '';
$template_admin_menu = false;

// These can be added to by individual pages, to give feedback
$template_infobox = array();
$template_warnbox = array();
$template_errorbox = array();

?>