@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -w -x -S  "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9	
goto endofperl
:WinNT
n:\nobackup\bin\perl -w -x -S %0 %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!perl
#line 15


#################################################################
# make_html_images.bat
#
#   1 MP Initial Revision

#####################
## USING DECLARATIONS
				 
use GD::Graph::lines;
use GD::Graph::bars;
use GD;
use POSIX;
use Spreadsheet::WriteExcel;
#####################




my $gbDebugOut = 0;




$MAP_IMAGE_DIR = 'C:\game\tools\scripts\log parsing\Map Images';

$SECONDS_PER_DAY	= (60 * 60 * 24);
$SECONDS_PER_HOUR	= (60 * 60);
$SECONDS_PER_MINUTE = 60;
$DAYS_PER_WEEK		=  7;							 

$FEET_PER_MILE 		= 5280;		

$MAP_GRID_SQUARES_PER_MILE 	= 25;  # determines precision of map grid graphs
$MAP_GRID_FEET_PER_SQUARE 	= $FEET_PER_MILE / $MAP_GRID_SQUARES_PER_MILE;


#
#########################











## dimensions of each map, (in miles) 	(x miles, y miles, x offset, y offset)
## (note: I am mapping game Z-coordinates to map Y-coordinates for my 2-D graphing)
my $ghMapDimensions =  {'City_00_01' 	=>	[1,1, -254, 1432],
				        'City_01_01'	=>  [1,1, 120, -749],
						'City_01_02'	=>  [1,2, -861, -211],
						'City_02_01'	=>	[1,2, -3260, -514],
						'City_02_02'	=>	[1,2, 537, -3887],
						'Hazard_01_01'	=>	[1,1, -1407.2, 1953.5],
					   };
$gbPlotGroup = 1;
$gbEncounterDebug = 0; ## toggles white crosshair and green circle @ origin




my $fileCount = 0;   # counts # of files processed

## Set subscript operator to 'comma'
$; = ',';



$sCommandFile = $ARGV[0];

open(INFILE, $sCommandFile) || die "Unable to open file $sCommandFile ($_)";
   

while($sLine = <INFILE>)
{

	#print "LINE: $sLine";

	if($sLine !~ m/\@\@BEGIN\@\@/)
	{
		next;
	}

   $sType     = <INFILE>;
   $sFilename = <INFILE>;
   chomp($sFilename);

   ## remove leading & trailing whitespace
   $sType =~ s/^\s*(.*?)\s*$/$1/;

   print "Creating $sFilename ($sType)\n";

	   if($sType eq "Frequency Plot")
	   {
  			CommandFrequencyPlot(INFILE, $sFilename);
	   }
	   elsif($sType eq "Grid Frequency Plot")
	   {
  			CommandGridFrequencyPlot(INFILE, $sFilename);
	   }
	   elsif($sType eq "Line Graph")
	   {
  #			CommandLineGraph(INFILE, $sFilename);
	   }
	   elsif($sType eq "SpreadSheet")
	   {
  			CommandSpreadSheet(INFILE, $sFilename);
	   }
	   else
	   {
			die "Bad Type: $sType";
	   }
}



sub CommandSpreadSheet
{
	my ($fh, $filename) = @_;

	my $oWorkBook = Spreadsheet::WriteExcel->new($filename);

  	die "Failed to open workbook $sExcelFile" unless($oWorkBook);

	DebugOut("Opened $filename for output\n");

	my $iRow = 0;
	my $oWorkSheet;

	while($line = <$fh>)
	{
		my $val;
		if($line =~ m/\@\@END\@\@/)
		{
			last;
		}
		elsif(($val) = $line =~ m/^WorkSheet:\s*(.*?)\s*$/i)
		{
			$oWorkSheet = $oWorkBook->add_worksheet($val);
			$oWorkSheet->set_column(0,0,45);	## left most column is X characters wide
			$iRow = 0;
		}
		else
		{
			die "no worksheet specified" unless $oWorkSheet;

			my @aVals = split(', ', $line);		 
			
			die "No values! ($line)" unless @aVals;

			my $iCol = 0;
			for(my $i = 0; $i < @aVals; $i++)
			{
				$aVals[$i] =~ s/^\s*(.*?)\s*$/$1/;  ## get rid of surrounding whitespace
				$oWorkSheet->write($iRow, $iCol, $aVals[$i]);
				$iCol++;	
			}
			$iRow++;
		}
	}

	$oWorkBook->close();
}

  


sub CommandFrequencyPlot #(INFILE, $sFilename);
{
	my ($fh, $sOutFilename) = @_;

	my $sMapName = <$fh>;
	chomp($sMapName);
	$sMapName =~ s/,//;  # remove commas
	$sMapName =~ s/\s//;  # remove whitespace

	DebugOut("MapName: $sMapName\n");
	my %hValues = ();

	while(   $sLine = <$fh>)
	{
		if($sLine =~ m/\@\@END\@\@/)
		{
			last;
		}

		DebugOut("$sLine\n");
		unless(($key, $val) = $sLine =~ m/^([^,]+,\s*[^,]+,\s*[^,]+),\s+([^,]+)/)
		{
			die "Bad Input: $sLine";
		}

		DebugOut("($key) ==> ($val)\n");

		$hValues{$key} = $val;
	}

	CreateCoordinateFrequencyMap($sMapName, \%hValues, $sOutFilename);
}


sub CommandGridFrequencyPlot 
{
	my ($fh, $sOutFilename) = @_;
	
	my $sMapName = <$fh>;
	my %hValues = ();

	while(   $sLine = <$fh>
	      && $sLine !~ m/\@\@END\@\@/)
	{
		unless(($key, $val) = $sLine =~ m/^([0-9\-+.]*, [0-9\-+.]*, [0-9\-+.])\s+(\w+)/)
		{
			die "Bad Input: $sLine";
		}

		DebugOut("($key) ==> ($val)\n");

		$hValues{$key} = $val;
	}		


	CreateGridFrequencyPlot($sMapName, \%hValues, $sOutFilename);
}


sub CommandLineGraph
{
	my ($fh, $sOutFilename) = @_;

	my @aPlotData = ();

	my $sTitle = <$fh>;
	my $sXAxisLabel = <$fh>;
	my $sYAxisLabel = <$fh>;

	chomp $sTitle;
	chomp $sXAxisLabel;
	chomp $sYAxisLabel;

	while($sLine = <$fh>)
	{
		$sLine =~ s/^(.*?,)\s*$/$1/;	## cleanup trailing whitespace, newline

		if($sLine =~ m/\@\@END\@\@/)
		{
			last;
		}

		#print "$sLine\n";

		@aValues = split(',', $sLine);

		for($i = 0; $i < scalar(@aValues); $i++)
		{
			$aValues[$i] =~ s/\s+//;	## remove whitespace, we just want numbers

			#print "INDEX, VALUE ==> ($i, $aValues[$i])\n";
			push @{$aPlotData[$i]}, $aValues[$i];
		}
	}

	#CreateCoordinateFrequencyMap($sMapName, \%hValues, $sOutFilename);

	DebugOut("SIZE: " . scalar(@aPlotData) . "\n");

	CreateLineGraphFromArrays($sTitle, $sXAxisLabel, $sYAxisLabel, $sOutFilename, \@aPlotData);
}











# returns filename
sub CreateCoordinateFrequencyMap
{
	my ($sMapName, $hashRef, $sOutFilename) = @_;
				   
	DebugOut("Map Name: |$sMapName|\n");

	unless(exists $ghMapDimensions->{$sMapName})
	{
		DebugOut("No map image exist for this map. Image will not be made.");
		return;
	}

	$sFilename = "$MAP_IMAGE_DIR\\map_$sMapName";

	$image = ImageFromJPEG($sFilename);

	SetMaxDimensions($image, $sMapName);

	# get max # of spawns
	$first = (keys %{$hashRef})[0];
	$max = $hashRef->{$first};
	foreach $group (keys %{$hashRef})
 	{
  	   $val = $hashRef->{$group};
	   if($val > $max)
	   {
			$max = $val;
		}
 	}

	DebugOut("Max: $max\n");
	foreach $group (keys %{$hashRef})
 	{
		if($gbPlotGroup)
		{	
  	   		PlotGroup($image, $group, $hashRef->{$group}, $max);
		}
 	}

 	$color1 = $image->colorAllocate(40,40,255); 


	if($gbEncounterDebug)
	{
	 	$image->filledEllipse(Real2ImageCoord(0,0),18,18,$color1); 

 		# add a white crosshair
 	 	DrawGrid($image);
	}

	 DrawThermometer($image, $max);  

	 $image->string(gdGiantFont, 80, $gImageHeight - 20, "Spawn Frequency",$color1);
 
	OutputJPEG($image, $sOutFilename);
}



# takes a map name and hash of x,y,z coords => frequency values
sub CreateGridFrequencyPlot
{
	my ($sMapName, $hashRef, $sOutFilename) = @_;
				   
	DebugOut("Map Name: $sMapName\n");

	$sFilename = "$MAP_IMAGE_DIR\\map_$sMapName";

	$image = ImageFromJPEG($sFilename);
	SetMaxDimensions($image, $sMapName);

	## Compute frequencies for each square
	my @aCount = ();
	foreach $position (keys %{$hashRef})
	{
		#print "Position: $position\n";
		my ($x,$y) = $position =~ m/(.*),.*,(.*)/; # don't care about 'y' coords
		my ($row, $col) = Position2GridSquare($x, $y);

		#print "($x,$y) --> ($row, $col)\n";


		$aCount[$row][$col] += $hashRef->{$position};
	}

	# get max value
	$max = $aCount[0][0];  # any actual value will do
	for($row = 0; $row < $gSquareRows; $row++)
	{
		for($col = 0; $col < $gSquareCols; $col++)
		{
			$val = $aCount[$row][$col];
			if($val > $max)
			{
				$max = $val;
			}
		}
	}

	DebugOut("Max: $max\n");

	for($row = 0; $row < $gSquareRows; $row++)
	{
		for($col = 0; $col < $gSquareCols; $col++)
		{
			$val = $aCount[$row][$col];
			if($val)
			{
				$color = GetColor($image, $aCount[$row][$col], 0, $max);
				$image->filledRectangle(  $row      * $gPixelsPerSquare,
				                          $col      * $gPixelsPerSquare,
				                         ($row + 1) * $gPixelsPerSquare,
				                         ($col + 1) * $gPixelsPerSquare,
				                          $color);
			}
		}
	}

	DrawThermometer($image, $max);

	OutputJPEG($image, $sOutFilename);
}

 

sub CreateLineGraphFromArrays
{
   #	my ($sTitle, $sLabelX, $sLabelY, $xValues, $yValues) = @_;
	
	$sTitle  = shift;
	$sLabelX = shift;
	$sLabelY = shift;     # all the rest are the data values

	$sFilename = shift;
	
	$aArrayRefs = shift; 

	DebugOut("Passed in " . scalar(@{$aArrayRefs}) . " hash refs\n");
	  
	DebugOut( "****************\nCreating Graph:\n");
	DebugOut( "Title: $sTitle\n");
	DebugOut( "X Label: $sLabelX\n");
	DebugOut( "Y Label: $sLabelY\n");
	DebugOut( "Filename: $sFilename\n");

	$yMax = 0; # max y value found (for setting proper graph dimensions)


	my $bInitial = 1;
	$iEntryCount = 0;
	my @aData = ();

	$iYMax = 10;

	while(scalar(@{$aArrayRefs}))
	{
		DebugOut("GOT ONE!\n");
		$arrayRef = shift @{$aArrayRefs};  # get next hash table full of data points

		if($bInitial)
		{
			$bInitial = 0;

			## keys are time values
			@aX_Values = @{$arrayRef};

			$iEntryCount = scalar(@aX_Values); 
		
			#foreach $val (@aX_Values)
			#{
			# 	print "VAL: $val\n";   	
			#}
		
			push @aData, [@aX_Values];		
		}
		else
		{
			foreach $val (@{$arrayRef})
			{
				if($val > $iYMax)
			 	{
			 		$iYMax = $val;
			 	}		
			}

			push @aData, [@{$arrayRef}];
		}
	}

 
	foreach $a (@aData)
	{
  #		print "Array Size: " . scalar(@{$a}) . "\n";

	#	print "Contents: " . join("\t", @{$a}) . "\n";
	}

	#Plot all at once


	$graph = GD::Graph::lines->new($iEntryCount * 10 + 200, 300);

	$graph->set(
					x_label     =>		$sLabelX,
					y_label     =>		$sLabelY,
					title		=>		$sTitle,
					x_tick_number => 	($iEntryCount / 24),
					y_max_value		=>	Max(10, ($iYMax * 1.05)),	 ## adds a little room above highest point
					y_min_value 	=> 	0,
					y_tick_number	=>	5,
					y_label_skip	=>	1,
	) or die "Graphing Error: " . $graph->error; 

	foreach $val (@aData)
	{
		DebugOut("$val, ");
	}
	DebugOut( "\n");

	my $gd = $graph->plot(\@aData) or die $graph->error;

	OpenFile(IMG, ">$sFilename") or die;
	binmode IMG;
	print IMG $gd->jpeg;

	return $sFilename; 
}	
	 

















   


sub byCoordinate
{
	my ($aX, $aY, $aZ) = $a =~ m/(.*), (.*), (.*)/;
	my ($bX, $bY, $bZ) = $b =~ m/(.*), (.*), (.*)/;

	my $compX = ($aX <=> $bX);
	my $compY = ($aY <=> $bY);
	
	if($compX)
	{
		return $compX;
	}
	elsif($compY)
	{
		return $compY;
	}
	else
	{
		return ($aZ <=> $bZ);
	}
}


 

sub Position2GridSquare
{
	my ($realX, $realY) = @_;

	my ($imageX, $imageY) = Real2ImageCoord($realX, $realY);
	
	$row = $imageX / $gPixelsPerSquare;
	$col = $imageY / $gPixelsPerSquare;
	
	return ($row, $col);  
}



sub ImageFromJPEG
{
	my ($sFilename) = @_;

	#if .jpg is already in filename, remove it (cause it's added below)
	$sFilename =~ s/\.jpg$//;
	$sFilename .= '.jpg'; 

	OpenFile(JPEG,"$sFilename") or die;
  	$image = newFromJpeg GD::Image(\*JPEG, 1) || die "Unable to open image file $sFilename: $_";
  	close JPEG;

	DebugOut("Opened JPEG    $sFilename\n");

	return $image;  
}


sub OutputJPEG
{
	my ($image, $sFilename) = @_;

	#if .jpg is already in filename, remove it (cause it's added below)
	$sFilename =~ s/\.jpg//ig; 
	$sFilename .= ".jpg";

    OpenFile(IMG_FILE, ">$sFilename") or die;
    binmode IMG_FILE;
    print IMG_FILE $image->jpeg;
    close IMG_FILE;
}


sub DrawThermometer
{
	my ($image, $max) = @_;

	$increment = 50;
	for($i = 0; $i <= $increment; $i++)
	{
		$color = GetColor($image, ($increment - $i), 0, $increment);
		$minY = ($gImageHeight / $increment) * $i;
		$maxY = ($gImageHeight / $increment) * ($i + 1);
		$image->filledRectangle(0,$minY,10,$maxY, $color);
	}

	#display 3 values (min, median, max) along thermometer
	$color = GetColor($image, 0, 0, 1);
	$image->string(gdGiantFont, 15, $gImageHeight - 20, "0",$color);

	$color = GetColor($image, 0.5, 0, 1);
	$image->string(gdGiantFont, 15, $gImageHeight / 2, ($max / 2),$color);
	
	$color = GetColor($image, 1, 0, 1);
	$image->string(gdGiantFont, 15, 5, "$max",$color);	
}																    

sub GetColor
{
	my ($image, $val, $min, $max) = @_;	    

	$percent = ($val - $min) / ($max - $min);

 	## NEW FORMULA
 	my ($r,$g,$b) = (0,0,0);
 	if($percent > 0.5)
 	{
 		$r = 255 * (($percent - 0.5) * 2);
 		$g = 255 * (2 - ($percent * 2));
 	}
 	else
 	{
 		$b = 255 * (1 - ($percent * 2));
 		$g = 255 *      ($percent * 2);
 	}

	$r = MinMax($r, 0, 255);
	$g = MinMax($g, 0, 255);
 	$b = MinMax($b, 0, 255);

	return $image->colorAllocateAlpha($r,$g,$b, 60) 
#	return $image->colorAllocate($r,$g,$b) 
 }

 sub MinMax
 {
	my ($a, $min, $max) = @_;
	return(Max($min, Min($max, $a)));
}

 sub Max
 {
	my ($a, $b) = @_;
	if($a > $b)
	{
		return $a;
	}
	else
	{
		return $b;
	}
 }


 sub Min
 {
	my ($a, $b) = @_;
	if($a < $b)
	{
		return $a;
	}
	else
	{
		return $b;
	}
 }

			
sub PlotGroup
{
	my ($image, $group, $val, $max) = @_;

	my ($x,$y) = $group =~ m/(.*),.*,(.*)/;

 	$color = GetColor($image, $val, 0, $max);
	$white = $image->colorAllocate(255,255,255);
	
	$rad = 10;
	#$rad = 5 + 14 * ($spawns / $max);
	#$rad = GetRadius($completions, $spawns);
	if(($x,$y) = Real2ImageCoord($x,$y))
	{
		# draw white outline by first drawing slightly larger 'white' circle
	   	$image->filledEllipse($x, $y,$rad + 2, $rad + 2,$white); 
	   	$image->filledEllipse($x, $y,$rad, $rad,$color); 
	}
}

sub GetRadius()
{
	my ($a, $b) = @_;

	return (5 + (14 * ($a / $b)));
}

sub DrawGrid
{
  my ($image) = @_;	 
  
  ($width,$height) = $image->getBounds();
    
  $white = $image->colorAllocate(255,255,255);

  $x0 = 0;
  $x1 = $width - 1;
  $y0 = $height / 2;
  $y1 = $y0;

  $image->line($x0,$y0,$x1,$y1,$white); 	

  $x0 = $width / 2;
  $x1 = $x0;
  $y0 = 0;
  $y1 = $height - 1;

  $image->line($x0,$y0,$x1,$y1,$white) 

}


 
sub SetMaxDimensions
{
 	my ($image, $sMapName) = @_;


   	($gImageWidth,$gImageHeight) = $image->getBounds();

 	
	if(exists $ghMapDimensions->{$sMapName})
	{
		#($minX, $minY, $maxX, $maxY) = @{$ghMapDimensions->{$sMapName}};
		($xMiles, $yMiles, $xOffset, $yOffset) = @{$ghMapDimensions->{$sMapName}};

		$xWidth = $xMiles * $FEET_PER_MILE;
		$yWidth = $yMiles * $FEET_PER_MILE;

		$minX = -($xWidth / 2) + $xOffset;
		$minY = -($yWidth / 2) + $yOffset;


		DebugOut("Dimensions: ($minX, $minY) -> (" . ($minX + $xWidth) . ', ' . ($minY + $yWidth) . ")\n");

				 	 
	 	$gXOffset = (- $minX);
	 	$gYOffset = (- $minY);
	 
	 	$gXReal2ImageScale = $gImageWidth  / ($xWidth);
	 	$gYReal2ImageScale = $gImageHeight / ($yWidth);

		# used by grid frequency plots
		$gPixelsPerSquare = $gXReal2ImageScale * $MAP_GRID_FEET_PER_SQUARE;
		$gSquareRows = $gImageHeight / $gPixelsPerSquare;
		$gSquareCols = $gImageWidth  / $gPixelsPerSquare;
		DebugOut( "Feet Per Square:   $MAP_GRID_FEET_PER_SQUARE\n");
		DebugOut( "Pixels Per Square: $gPixelsPerSquare\n");
		DebugOut( "Square rows/cols = ($gSquareRows, $gSquareCols)\n");
	}
}

sub Real2ImageCoord
{
	my ($rX,$rY) = @_;

	$iX = ($rX + $gXOffset) * $gXReal2ImageScale;
	
	$iY = ($rY + $gYOffset) * $gYReal2ImageScale; 

	# x axis is flipped
	$iX = $gImageWidth - $iX;

	if(   $iX >= $gImageWidth
	   || $iX < 0
	   || $iY >= $gImageHeight
	   || $iY < 0)
	{
	   DebugOut("Bad graph coordinate: ($rX, $rY) --> ($iX, $iY)\n");
	}

	return ($iX, $iY);
}

	  




		  
# used to optionally add an "s" to the end of a word if the parameter != 1
sub plural
{
	$val = shift;
	if($val != 1)
	{
		return "s";
	}
	else
	{
		return "";
	}
}


sub max
{
	$max = shift;
	foreach $val (@_) {
		if($val > $max)
		{
			$max = $val;
		}
	}

	return $max;
}

# add leading 1 if number is between 0-9
sub ZeroPad
{
	$num = shift;
	if(   $num >= 0 
	   && $num <= 9)
	{
		return "0$num";
	}
	else
	{
		return $num;
	}
}

	

sub Percent
{
	my ($val, $total) = @_;
	
	#print "Percent($val, $total)\n";

	if($val && $total)
	{ 
		$result = ( ($val / $total) * 100);
		return (Precision($result, 2) . "%");
	}
	else
	{
		Warning("Val or Total equals '0' in Percent()");
		return  "0";
	}
}


sub Precision
{
	my $num = shift;
	my $prec = shift;

	my $format = "%." . $prec . "f";
	return sprintf($format, $num);
}
   

#####################################################################################
##
##    TIME CONVERSION FUNCTIONS
##
#####################################################################################

## convert HH:MM:SS format into # of seconds since midnight (00:00:00). (returns an integer)
sub clockTime2Seconds
{
	my $timeSec = shift;

	my $hour;
	my $min;
	my $sec;

	if($timeSec =~ m/\d\d?:\d\d?:\d\d?/)
	{
		#format is HH:MM:SS (each field 1 or 2 digits)
		($hour, $min, $sec) = $timeSec =~ m/(\d\d?):(\d\d?):(\d\d?)/;
	}
	else
	{
		#format is MM:SS (each field 1 or 2 digits)
		($min, $sec) = $timeSec =~ m/(\d\d?):(\d\d?)/;
		$hour = 0;
	}

	my $total = ($hour * 3600) + ($min * 60) + $sec;
#	print "Time ($timeSec) == $total seconds\n";
}




## Convert "YYMMDD HH:MM:SS" format to seconds
sub dateTime2Sec
{
	$logEntry = shift;
   	if($logEntry)
	{
		my ($year, $month, $day, $hour, $min, $sec) = $logEntry =~ m/^(\d\d)(\d\d)(\d\d) (\d\d):(\d\d):(\d\d)/;

		$month -= 1;  #convert from 0-11 to 1-12

		## HACK!!!
		if($month < 0)
		{
			if($bFirstTime == 0)
			{
				DebugOut("\n\n\n*********  Warning: hack to fix month=-1 problem\n\n\n\n!!!");
				$bFirstTime = 1;
			}
			return 0;
		}

		return timelocal($sec, $min, $hour, $day, $month, $year);

	}
	else
	{
		Warning("Bad logEntry value passed into dateTime2Sec()");
		return 0;
	}
}


# expect time in HH:MM:SS format. Converts to a #
sub clockTime2HourBlock {
	my $time = shift;
	my ($ret) = $time =~ m/^(\d\d?):\d\d:\d\d/;
	return $ret;
}


sub sec2Hours
{
	my $sec = shift;
	return ($sec / $SECONDS_PER_HOUR);
}


sub sec2HourOfDay {
	my $seconds = shift;
	my ($sec, $min, $hour) = localtime $seconds;
	return $hour;
}

   ## misleading title...need to rename!!
sub sec2Day {

	my $seconds = shift;

	## THIS IS TERRIBLY INEFFICIENT, but it works...
	## I tried some modulus stuff (ie % $SECONDS_PER_DAY) but couldn't get it to work right...
	##
	my ($sec, $min, $hour, $day, $month, $year) = localtime $seconds;

	return timelocal(0, 0, 0, $day, $month, $year);

}








 



#####################################################################################
##
##    OUTPUT FUNCTIONS
##
#####################################################################################

sub OpenFile
{
	my ($fh, $sFilename) = @_;
	if(open($fh, "$sFilename"))
	{
		return 1;
	}
	else
	{
		print "\n\tUnable to open file.\n\n\t\tFile: $sFilename:\n\n\t\tReason: $!\n\n\n";
		return 0;
	}
	#print "Opened file $sFilename\n";
}	


	  
sub Warning
{
	my $sOut = shift;

	$giWarningCount++;
	if($gbShowWarnings)
	{
		print "\n(Warning)\t$sOut\n";
	}
}


sub Output
{	
	my $fh = shift;				 
	print $fh @_, "\n";
}


sub StatusOut
{					 
	print @_, "\n";
}






 ### 
# CreateHourGraph  -  graph an array of length 24, with each index representing a one hour block of a day
#
# Params: <TTILE> <FILENAME> [VALUES]
#
sub CreateHourGraph {
	$title = shift;
	$sFilename = shift;
	@values = @_;     # all the rest are the data values

	$sFilename .= ".png";

	DebugOut("creating graph image $sFilename\n");

#	print "Values: @values\n";

	my @axis_labels = ();
	for($i = 0; $i < 24; $i++)
	{
		push @axis_labels, $i;
	}

	my @data = ([@axis_labels], [@values]);

	my $graph = GD::Graph::bars->new(400, 300);

	$graph->set(
		x_label     =>		'Hour of Day',
		y_label     =>		'Count',
		title		=>		$title,
		y_max_value	=>		(max(@values) * 1.2),
		y_tick_number	=>	10,
		y_label_skip	=>	2,
	) or die $graph->error; 

	my $gd = $graph->plot(\@data) or die $graph->error;

	OpenFile(IMG, ">$sFilename") or die;
	binmode IMG;
	print IMG $gd->png;

	return $sFilename;
}


sub CreateLineGraph
{
	my ($sTitle, $sXLabel, $sYLabel, $sFilename, $arrayRef) = @_;
	
	$title = shift;
	$sFilename = shift;
	$arrayRef = shift;     # all the rest are the data values

	$iCount = scalar(@{$arrayRef});
	
	DebugOut("Graphing $iCount values\n");
	  
	$sFilename .= ".png";

	DebugOut("creating graph image $sFilename\n");

#	print "Values: @aValues\n";

	my @axis_labels = ();
	for($i = 0; $i < $iCount; $i++)
	{
		push @axis_labels, $i;
	}

	my @data = ([@axis_labels], [@{$arrayRef}]);

	my $graph = GD::Graph::bars->new($iCount * 10 + 200, 300);

	$graph->set(
		x_label     =>		$sXLabel,
		y_label     =>		$sYLabel,
		title		=>		$sTitle,
		y_max_value	=>		(max(@{$arrayRef}) * 1.2),
		y_tick_number	=>	$iCount,
		y_label_skip	=>	2,
	) or die $graph->error; 

	my $gd = $graph->plot(\@data) or die $graph->error;

	OpenFile(IMG, ">$sFilename") or die;
	binmode IMG;
	print IMG $gd->png;

	return $sFilename; 
}	



   

sub MakeDirectory
{
	$sDirName = shift;
	
	unless(-d $sDirName)	  ## only make if directory doesn't already exist
	{
		system(qq(mkdir "$sDirName")); 
	}
}

sub ForwardSlashes
{
	my ($str) = @_;
	$str =~ s'\\'/'g;
	return $str;
}


sub BackSlashes
{
	my ($str) = @_;
	$str =~ s'/'\\'g;
	return $str;
}



 
	

sub HashValueSum
{
	my ($hashRef) = @_;

	my $sum = 0;
	foreach $key (keys %{$hashRef})
	{
		$sum += $hashRef->{$key};	
	}
	
	return $sum;
}	


sub DebugOut
{
	if($gbDebugOut)
	{
		print @_;
	}
}



__END__
:endofperl
