Directory Contents
------------------
(1) Runespy
	- This Project is the base C# application that monitors user levels of 
	the MMO Runescape at predetermined intervals (that can be defined on the 
	command line). The default interval is 15 minutes and values are written 
	to file at every interval. The file is rolled every day (thus values are
	from 12AM to 11:45PM).
(2) RunespyService
	- This project wraps the Runespy project (above) into a service that can be
	run with no user logged in.
(3) MMOSpy.pl
	- This is the latest incarnation. This Perl script runs within a cacti
	framework (http://cacti.net) which is a web-based graphing front-end built
	on RRDtool (http://oss.oetiker.ch/rrdtool/). Currently, this script is
	running on the Core Cacti installation run by Greg Peterson in Ops. Greg
	must grant you access before you can view the output logs at 
	http://ncagamereports01/cacti/.
	