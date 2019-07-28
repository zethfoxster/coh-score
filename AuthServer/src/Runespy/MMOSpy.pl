#!/usr/bin/perl -w

# Modules ######################################################################

use strict;
use LWP::Simple;
use LWP::Simple qw($ua);

# Set STDOUT not to cache
$| = 1;

# Read in parameters
my ($game) = @ARGV;  # command line options

die "No game specified!\n" unless $game;

# HTTP Client object
$ua->timeout(15);  # 15 second timeout for the http request
my $content;
my $parseString;
if ($game eq "Runescape")
{
    $content = get("http://www.runescape.com/lang/en/aff/runescape/title.ws");
    $parseString = "There are currently (\\d+) people playing!";
}
elsif ($game eq "HH")
{
    $content = get("http://www.habbo.com/");
#    $parseString = "(?:(\\d{1,3}),(\\d{3,3}),(\\d{3,3})|(\\d{1,3}),(\\d{3,3})|(\\d{1,3})) members online";
#    $parseString = "(?:(\\d{1,3}),(\\d{3,3}),(\\d{3,3})|(\\d{1,3}),(\\d{3,3})|(\\d{1,3}))</span> Habbos online";
    $parseString = "(?:(\\d+))</span> Habbos online";
}
elsif ($game eq "SL")
{
    $content = get("http://secondlife.com/");
    $parseString = "Online (?:.+?)<strong>(?:(\\d{1,3}),(\\d{3,3}),(\\d{3,3})|(\\d{1,3}),(\\d{3,3})|(\\d{1,3}))";
}
elsif ($game eq "Tibia")
{
    $content = get("http://www.tibia.com/");
    $parseString = ">(\\d+)<br />Players";
}

# make sure we got something back.
if (! defined $content)
{
    print -1; # something wrong with getting the web page! investigate immediately!
    exit;
}

if (! defined $parseString)
{
    print -1; # parse string not initialized correctly
    exit;
}

# Parse the webpage content in $content with the formatted parse string in $parseString
if ($content =~ /$parseString/s)
{
    # print current data point to stdout for cacti (RRDtool) to pick up
    if ($1)
    {
        if (! defined $2)
        {
            print $1;
        }
        else
        {
            print $1 . $2 . $3;
        }
    }
    if ($4)
    {
        print $4 . $5;
    }
    if ($6)
    {
        print $6;
    }
}
else
{
    print -3; # content not parsed correctly!!!! debug and investigate!
}
