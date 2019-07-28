using System;
using System.Collections.Generic;
using System.Text;

namespace Runespy
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                System.Console.WriteLine("Usage: runespy <t>");
                System.Console.WriteLine("   t = minutes between site reads");

                return;
            }

            int timeBetweenReadsInMinutes = Int32.Parse(args[0]);

            // setup today's date vars and output filename
            System.DateTime today = System.DateTime.Today;
            System.DateTime tomorrow = today.AddDays(1);
            
            string outputFilename = "runespy " + today.Month.ToString() + "_" + today.Day.ToString() + "_" + today.Year.ToString() + ".log";

            const string SITE_URL = "http://www.runescape.com/lang/en/aff/runescape/title.ws";
            const string FIRST_SEARCH_STRING = "There are currently ";
            const string SECOND_SEARCH_STRING = " people playing!";

            // setup the output file stream writer
            System.IO.StreamWriter outStream = new System.IO.StreamWriter(new System.IO.FileStream(outputFilename, System.IO.FileMode.Append));

            // Process the next read time to be the most recent 15 minute increment
            DateTime nextReadTime = DateTime.Now;
            nextReadTime = nextReadTime.Subtract(System.TimeSpan.FromMilliseconds(nextReadTime.Millisecond));
            nextReadTime = nextReadTime.Subtract(System.TimeSpan.FromSeconds(nextReadTime.Second));
            nextReadTime = nextReadTime.Subtract(System.TimeSpan.FromMinutes(nextReadTime.Minute % timeBetweenReadsInMinutes));

            while (true)
            {
                System.Threading.Thread.Sleep(300); // save some of the CPU for other programs

                if (DateTime.Now >= nextReadTime)
                {
                    System.Net.WebClient webClient = new System.Net.WebClient();
                    try
                    {
                        string pageText = webClient.DownloadString(SITE_URL);
                        int startTextIdx = pageText.IndexOf(FIRST_SEARCH_STRING);
                        startTextIdx += FIRST_SEARCH_STRING.Length;
                        int endTextIdx = pageText.IndexOf(SECOND_SEARCH_STRING, startTextIdx);
                        string playerNumString = pageText.Substring(startTextIdx, endTextIdx - startTextIdx);
                        int numPlayers = Int32.Parse(playerNumString);

                        // setup output string
                        string output = nextReadTime.ToString() + "," + numPlayers.ToString();

                        // format the string with a delimiter for excel
                        int dateTimeIdx = output.IndexOf(' ');
                        output = output.Insert(dateTimeIdx, ",");

                        // output to file
                        if (System.DateTime.Today == tomorrow)
                        { // create a different file to output to and close the old file
                            // update our notion of "today"
                            today = tomorrow;
                            tomorrow = today.AddDays(1);

                            // create new output filename
                            outputFilename = "runespy " + today.Month.ToString() + "_" + today.Day.ToString() + "_" + today.Year.ToString() + ".log";

                            // close old file
                            outStream.Close();

                            // Force garbage collection
                            System.GC.Collect();

                            // create new file stream object
                            outStream = new System.IO.StreamWriter(new System.IO.FileStream(outputFilename, System.IO.FileMode.Append));
                        }

                        outStream.WriteLine(output);
                        outStream.Flush();

                        nextReadTime += System.TimeSpan.FromMinutes(timeBetweenReadsInMinutes);
                    }
                    catch (System.Net.WebException e)
                    {
                        if (e.Status == System.Net.WebExceptionStatus.ServerProtocolViolation)
                        {
                        }
                    }
                }
            }
        }
    }
}
