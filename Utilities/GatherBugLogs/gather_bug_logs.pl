#!/usr/bin/perl -w

## Version History:
##   1. 8/25/2009	: Initial deployment.
##   2. 3/24/2011	: Updated for new log file format.

## Setup, to run this file:
## - Install ActiveState Perl, version 5.10.1.
## - Copy standard.pm, which is an NCSoft Perl Module, into your local Perl installation.
##		- We put it in C:\Perl\lib\NCSoft\TCC, but it should probably live in C:\Perl\site\lib\NCSoft\TCC.
## - Install several perl modules from the CPAN, e.g., using ActiveState's Perl Package Manager.
##		- Try running this file, and see which modules are missing.
##		- After installing ActiveState Perl, you can run the Perl Package Manager by typing "ppm" at the command prompt.


################################################################################
# Modules
################################################################################
use strict;
use NCSoft::TCC::standard;
use File::Copy;
use IO::File;
use IO::Dir;
use DBI;
use DBD::mysql;



################################################################################
# Proto-types
################################################################################
sub close_file($);
sub copy_file($$);
sub database_connect($$$$$);
sub database_disconnect($);
sub delete_file($);
sub ftp_cryptic_file($$);
sub move_file($$);
sub move_file_if_possible($$);
sub open_file_reading($);
sub open_file_writing($$);
sub queue_bugs();



################################################################################
# Variables / Configuration
################################################################################

    # Set STDOUT to cache
    $| = 1;

    # Grab script configuration file
    my ($config_file_path) = @ARGV;
    $config_file_path = 'bug_logs.conf' if (!defined($config_file_path));
    my %general_config = read_config_file($config_file_path);

    # Variable Configuration - Globals are use to remove allocation overheard
    my ($bug_datetime);             # Server: mm-dd-yyyy hh:mm:ss
    my ($bug_processID);            # Server: mapserver process ID
    my ($bug_bugID);                # Server; Bug ID
    my ($bug_type);                 # Server: what type of bug.
    my ($bug_category);             # User: what bug category
    my ($bug_summary);              # User: summary
    my ($bug_description);          # User: description
    my ($bug_clnsrv);               # Server: Build number game is on
    my ($bug_position);             # Server: Position data of client
    my ($bug_plyrname);             # Server: Player's character name
    my ($bug_gblhandle);            # Server: Player's global chat handle
    my ($bug_plyrlogin);            # Server: Player's game login id
    my ($bug_plyrdata);             # Server: Player character's game data
    my ($bug_plyrsetup);            # Server: current tray setup & enchancements
    my ($bug_plyrteam);             # Server: Player's team information
    my ($bug_plyrsainfo);           # Server: Player's story arc information
    my ($bug_clntBeaconinfo);       # Server: Beacon data
    my ($bug_clntHardware);         # Server: Client hardware information

    # Set Working directory
    
    # Build Report Variables
    
    # pre-compiled reg-ex
	# JWG: It looks like $normal_record is obsolete and out of date.  Remove it?
    my $normal_record = qr/^
    (\d{6}\s\d+:\d+:\d+)\s+         # Date and Time
    (\d+).*?\n                      # Process id(?)
    \((.+)\)\n                      # Bug Type
    Category:\s*(\d+)\s*\n          # Category
    Summary:\s(.+)\n+               # Summary
    Description:\s(.+)\n+           # Description
    Client\/Server\sVer:\s(.+)\n+   # Client \ Server Version
    Position:(.*)                   # Player positional information
    Player:\s*(\S+?.+)\n            # Player character's name
    Global\sHandle:\s*(\S+?.+)\n    # Player's global chat handle
    Login:\s*(\S+?.+)\n             # Player's authorization handle
    (Origin:.+)                     # Player's game data
    (Current\sTray:.+)              # Player setup
    Team\sInfo:(.+)                 # Player Team information
    (Story\sInfo:.+)                # Player's storyarc information
    (Beacon\sVersion:.+)            # Beacon data
    (CPU.+)                         # Player hardware information
    \@\@END                         # End of record
    /smx;                           # Allow Comments & Whitespace; multiline mode
    
    my $identify_record = qr/^
    (\d{6}\s\d+:\d+:\d+)\s+         # Date and Time
    (\d+).*?\n                      # Process id(?)
    \((.+)\)\n                      # Bug Type
    /mx;                            # Allow Comments & Whitespace; multiline mode
    
    # Create Paths (in case one doesn't exist)
    create_path($general_config{directories}->{buglog_dir}->{destination});
    create_path($general_config{directories}->{buglog_dir}->{source});
    create_path($general_config{directories}->{tmp_path});
    create_path($general_config{directories}->{buglog_dir}->{archive_combined});
    create_path($general_config{directories}->{buglog_dir}->{archive_cryptic});
    create_path($general_config{directories}->{buglog_dir}->{archive_ncsoft});
    create_path("$general_config{directories}->{buglog_dir}->{archive_ncsoft}queued");
    create_path($general_config{general}->{logging}->{base_path});
    create_path($general_config{directories}->{nccsCohBugReader}->{queue}->{base_path});
    
    # mysql parameters
    my $mysql_server = $general_config{mysql}->{server};
    my $mysql_port = $general_config{mysql}->{port};
    my $mysql_db = $general_config{mysql}->{db};
    my $mysql_user = $general_config{mysql}->{user};
    my $mysql_pass = $general_config{mysql}->{pass};
    
    # logging
    my $LOG = open_file_writing($general_config{general}->{logging}->{base_path} . $general_config{general}->{logging}->{log_name}, 1);


################################################################################
# Main Body
################################################################################

# Logging
print $LOG (get_current_datetime() . "\tSTARTED: gather_bug_logs.exe\n");

# Move the current bug.log files into a single file, out of the CoV logserver's working directory so we can do work
# Find all bug log files
my $source_filedir = IO::Dir->new($general_config{directories}->{buglog_dir}->{source});
my @source_filelist;
while (defined(my $file = $source_filedir->read))
{
    push(@source_filelist, $file) if ($file =~ /bug.*\.log/);
};

# Logging
my $number_of_files = @source_filelist;
print $LOG (get_current_datetime() . "\tFound $number_of_files files: @source_filelist\n");

## Create destination file names
my $current_timestamp = get_current_datetime();
my $destination_filename = $current_timestamp;
$destination_filename =~ s/://g;
$destination_filename =~ s/\s/_/g;
$destination_filename =~ s/-//g;
my $destination_filename_combined = $general_config{directories}->{buglog_dir}->{destination} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . 'bugs.log';
my $destination_filename_ncsoft = $general_config{directories}->{buglog_dir}->{destination} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . 'nbugs.log';
my $destination_filename_cryptic = $general_config{directories}->{buglog_dir}->{destination} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . 'cbugs.log';

# Logging
print $LOG (get_current_datetime() . "\tMoving Files: from logserver into bugserver directory.\n") if (@source_filelist);

# Move the source bug logs out of the working directory
my @dest_filelist;
while (my $file = shift(@source_filelist)) {
    print "Processing $file\n";
    if(!move_file_if_possible($general_config{directories}->{buglog_dir}->{source} . $file,
        $general_config{directories}->{buglog_dir}->{destination} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . $file))
    {
       print $LOG (get_current_datetime() . "Skipping file $file\n");
    } else {
       push(@dest_filelist, $file);
    }
}
@source_filelist = @dest_filelist;

# Logging
print $LOG (get_current_datetime() . "\tProcessing Files: consolidating and filtering files.\n") if (@source_filelist);

# Process each bug log
my ($inBug, $fullRecord) = (0, '');
my %number_bugs = ('Total' => 0);
if (@source_filelist) {
    open (OUT_NCSOFT, ">$destination_filename_ncsoft") or die;
    open (OUT_CRYPTIC, ">$destination_filename_cryptic") or die;
    open (OUT_COMBINED, ">$destination_filename_combined") or die;
}

foreach my $file (@source_filelist) {

    $file = $general_config{directories}->{buglog_dir}->{destination} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . $file;
    
    open(IN, "< $file");
    while (<IN>) {
    
        my $line = $_;
    
        # Locate the begining of a record
        if ($line =~ /^(\d{6}\s\d+:\d+:\d+)\s+(\d+)/) {
            $inBug++;
        }
    
        # Capture bug record data (when inside bug record)
        if ($inBug == 1) {
            $fullRecord = $fullRecord . $line;
        }
    
        # Locate the end of a record
        if ($line =~ /\@\@END/) {
            
            $inBug--;
            ($bug_datetime, $bug_processID, $bug_type) = ($fullRecord =~ $identify_record);

            # Track number of bugs of each type
            $number_bugs{Total}++;
            if (defined($number_bugs{$bug_type})) {
                $number_bugs{$bug_type}++;
            } else {
                $number_bugs{$bug_type} = 1;
            }
            
            # Add processed bug to the combined log of all bugs
            print OUT_COMBINED $fullRecord;
        
            # For User submitted bugs...
            if ($bug_type =~ /internal/i) {
            
                # Print out Cryptic's record
                print OUT_CRYPTIC $fullRecord;
            
            } elsif ($bug_type =~ /normal/i) {
                
                # Print out NCSoft's record
                print OUT_NCSOFT $fullRecord;
                
            }
        
        }
    
        # clean fullRecord var after work with a bug record is done
        if ($inBug == 0) {
        
            $fullRecord = '';
        
        }
   
    } 
        close IN;
    
}

if (@source_filelist) {
    close OUT_COMBINED;
    close OUT_CRYPTIC;
    close OUT_NCSOFT;
    
    # Logging
    print $LOG (get_current_datetime() . "\tDeleting files: removing source files.\n");

    # Delete source files, keeping the combined and filtered logs
    foreach my $file (@source_filelist) {
        delete_file($file);
    }
    
    # Logging
    print $LOG (get_current_datetime() . "\tCompressing files: compressing consolidated and filtered files.\n");
    
    # Compress combined and filtered files
    my $gzip = $general_config{applications}->{gzip}->{full_path};
    system($gzip, $destination_filename_ncsoft)==0 or die print "Failed to run $gzip $destination_filename_ncsoft: $!\n";
    system($gzip, $destination_filename_cryptic)==0 or die print "Failed to run $gzip $destination_filename_cryptic: $!\n";
    system($gzip, $destination_filename_combined)==0 or die print "Failed to run $gzip $destination_filename_combined: $!\n";
    $destination_filename_ncsoft =~ s/\.log/\.log\.gz/;
    $destination_filename_cryptic =~ s/\.log/\.log\.gz/;
    $destination_filename_combined =~ s/\.log/\.log\.gz/;
    die print "Failed to gzip $destination_filename_ncsoft: $!\n" unless (-e $destination_filename_ncsoft);
    die print "Failed to gzip $destination_filename_cryptic: $!\n" unless (-e $destination_filename_cryptic);
    die print "Failed to gzip $destination_filename_combined: $!\n" unless (-e $destination_filename_combined);
    
    if (defined($number_bugs{Normal})) {
        
        # Logging
        print $LOG (get_current_datetime() . "\tPlayer submitted bugs found.\n");

    }
    
    if (defined($number_bugs{Internal})) {
    
        # Logging
        print $LOG (get_current_datetime() . "\tFTP: transferring (Internal) bugs to Cryptic.\n");
    
        # FTP file to Cryptic
        ftp_cryptic_file($destination_filename_cryptic, $general_config{ftp}->{cryptic}->{upload_dir});
        
    }
    
    # Logging
    print $LOG (get_current_datetime() . "\tMoving Files: archiving file(s).\n");
    
    # Move files into archive folders
    my $archive_path_ncsoft = $general_config{directories}->{buglog_dir}->{archive_ncsoft} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . 'nbugs.log.gz';
    my $archive_path_cryptic = $general_config{directories}->{buglog_dir}->{archive_cryptic} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . 'cbugs.log.gz';
    my $archive_path_combined = $general_config{directories}->{buglog_dir}->{archive_combined} . $general_config{general}->{shardid} . '_' . $destination_filename . '_' . 'bugs.log.gz';
    
    # If normal player submitted bugs exist move them; else delete the empty file.
    if (defined($number_bugs{Normal})) {
        move_file($destination_filename_ncsoft, $archive_path_ncsoft);
        print $LOG (get_current_datetime() . "\tQUEUE: Attempting to queue bug logs.\n");
        queue_bugs();
    } else {
        delete_file($destination_filename_ncsoft);
    }
    
    # If Cryptic submitted bugs exist move them; else delete the empty file.
    if (defined($number_bugs{Internal})) {
        move_file($destination_filename_cryptic, $archive_path_cryptic);
    } else {
        delete_file($destination_filename_cryptic);
    }
    
    # The combined file always exists else workflow would not be entered.
    move_file($destination_filename_combined, $archive_path_combined);

    # Report Information to MySQL db
    # Logging
    print $LOG (get_current_datetime() . "\tMySQL: connecting to database\n");
    
    # open db handle
    my $dbh = database_connect($mysql_server, $mysql_port, $mysql_db, $mysql_user, $mysql_pass);
    
    # Logging
    print $LOG (get_current_datetime() . "\tMySQL: inserting data into database\n");
    
    # insert data
    my $sth = $dbh->prepare("INSERT INTO bug_logs (shard_id, log_time, type, number) VALUES (?, ?, ?, ?)");
    
    foreach my $key (keys(%number_bugs)) {
        $sth->execute($general_config{general}->{shardid}, $current_timestamp, $key, $number_bugs{$key});
    }
    
    # Logging
    print $LOG (get_current_datetime() . "\tMySQL: disconnecting from database\n");
    
    # close db handle
    database_disconnect($dbh);
    
}


# Logging
print $LOG (get_current_datetime() . "\tFINISHED: gather_bug_logs.exe\n");


################################################################################
# Formats
################################################################################


################################################################################
# Sub Routines
################################################################################

sub close_file($) {
    my $fh = $_[0];
    $fh->close();
}

sub copy_file($$) {
    my ($file1, $file2) = @_;
    copy($file1, $file2) or die;
    return 1
}

sub database_connect ($$$$$) {
    my ($server, $port, $database, $user, $pass) = @_;
    my $dsn = "DBI:mysql:database=$database;host=$server;port=$port";
    my $dbh = DBI->connect($dsn, $user, $pass);
    if (defined($dbh)) {
        return $dbh;
    }
}

# Disconnect from a specified MySQL server.  You must pass a database handle.
sub database_disconnect ($) {
    my ($dbh) = @_;
    $dbh->disconnect();
}

sub delete_file($) {
    my ($filepath) = @_;
    $filepath =~ s/\//\\/g;
    unlink ($filepath) || die print "Error deleting $filepath: $!";
}

sub ftp_cryptic_file ($$) {
    my ($file, $remote_dir) = @_;
    system($general_config{ftp}->{cryptic}->{full_path}, $file, $remote_dir)==0 or die;
}

sub move_file($$) {
    my ($src, $dest) = @_;
    move($src, $dest) or die "Error moving $src to $dest: $!\n";
    return 1
}

sub move_file_if_possible($$) {
   my ($src, $dest) = @_;
   return move($src, $dest);
}

sub open_file_reading($) {
    my ($file) = @_;
    my $fh = new IO::File "< $file" || die "Error opening $file for reading: $!\n";
    return $fh;
}

sub open_file_writing($$) {
    my ($file, $mode) = @_;
    my $fh;
    if ($mode == 1) {
        # Append Mode
        $fh = new IO::File ">> $file" || die "Error opening $file for writing: $!\n";
    } elsif ($mode == 2) {
        # Overwite / New Mode
        $fh = new IO::File "> $file" || die "Error opening $file for writing: $!\n";
    }
    return $fh;
}

sub queue_bugs() {
	
	# check for a queue lock
	if (-e "$general_config{directories}->{nccsCohBugReader}->{queue}->{base_path}$general_config{general}->{shardid}_BUGS.lock") {
		# logging
		print $LOG (get_current_datetime() . "\tQUEUE: nccsCohBugReader queue lock in place: new bugs will be queued on the next pass\n");
	} else {
		print $LOG (get_current_datetime() . "\tQUEUE: no nccsCohBugReader queue lock found: queueing bugs\n");
		# Grab list of bug logs
		opendir(DIR, $general_config{directories}->{buglog_dir}->{archive_ncsoft});
		my @files = grep { /\.log\.gz$/ } readdir(DIR);
		closedir(DIR);
		
		# logging
		my $num_files = @files;
		print $LOG (get_current_datetime() . "\tQUEUE: $num_files found to queue\n");
		
		# queue and store bug logs
		foreach my $file (@files) {
			my $buglog_src = $general_config{directories}->{buglog_dir}->{archive_ncsoft} . $file;
			my $buglog_archive = $general_config{directories}->{buglog_dir}->{archive_ncsoft} . "queued/" . $file;
			my $buglog_queue =  "$general_config{directories}->{nccsCohBugReader}->{queue}->{base_path}$general_config{general}->{shardid}_BUGS.txt";
			$buglog_src =~ s/\//\\/g;
			$buglog_archive =~ s/\//\\/g;
			$buglog_queue =~ s/\//\\/g;
			copy_file($buglog_src, $buglog_archive);
                        my $zcat = $general_config{applications}->{zcat}->{full_path};
			system("$zcat -d \"$buglog_src\" >> \"$buglog_queue\"")==0 or die print "Failed to run $zcat -d \"$buglog_src\" >> \"$buglog_queue\": $!\n";
			unlink($general_config{directories}->{buglog_dir}->{archive_ncsoft} . $file) or die;
		}
		
		open(LOCKF, ">$general_config{directories}->{nccsCohBugReader}->{queue}->{base_path}$general_config{general}->{shardid}_BUGS.lock");
		close(LOCKF);
		
		# logging
		print $LOG (get_current_datetime() . "\tQUEUE: bugs queued and nccsBugReader lock placed.\n");
		
	}
	
}
