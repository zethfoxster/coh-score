Paragon memtrack analysis sample:

The sample analys tools can be used as a starting point for doing your own
data mining and analysis of memtrack logs.

To use the python script the first thing that needs to be done is to resolve all
the callstack address symbols and place them in a sqlite database for use by the
analysis script.

To do this you will need a copy of the original executable and its pdb file that was used
to generate the memtrack log (N.B. always save a copy of these with any logs that you capture).

Place these together in the analysis folder or wherever (maybe easiest to start by putting everything
in the analysis folder).

Run the memtrack executable to generate the sqlite db as follows:

-Open a commnad prompt in the analysis folder
-Enter the following command
      memtrack <module name> <dictionary name>
      e.g.,
      memtrack mapserver.exe memtrack_20101129_233058_dict.dat

This will create a file called address.db in the same folder. 

You can then run the python analysis script by using the following command line (as an example).


  memtrack.py -S -i memtrack_20101129_233058_log.dat -d memtrack_20101129_233058_dict.dat -a address.db

This command will run a simple heap simulation.

There are 3 key parameters that need to be supplied:

   The path of the event log (defaults to memtrack_log.dat)
   The path of the event log (defaults to memtrack_dict.dat)
   The path of the symbol datablase (defaults to address.db)

use memtrack.py --help for more details on command line options.

NOTE: Most reporting and graph generation features have been removed because they used proprietary code.


