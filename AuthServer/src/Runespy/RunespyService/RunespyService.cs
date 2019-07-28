using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.ServiceProcess;
using System.Text;
using System.Threading;

namespace RunespyService
{
    public partial class RunespyService : ServiceBase
    {
        public RunespyService()
        {
            InitializeComponent();

            // Initialize class vars
            m_numMinutes = 15;      // defaults to 15 minute intervals
            m_bRunning = false;     // defaults to not running
        }

        protected override void OnStart(string[] args)
        {
            // Trigger Worker Thread to run the main loop
            m_bRunning = true;

            // Check if the time between reads in minutes has changed
            if (args.Length == 1)
                m_numMinutes = Int32.Parse(args[0]);

            // Start worker thread
            StartWorkerThread();
        }

        protected override void OnStop()
        {
            // Trigger Worker Thread to stop the main loop
            m_bRunning = false;

            // Close the output file
            m_outStream.Close();
        }

        private void StartWorkerThread()
        {
            //IntPtr handle = this.ServiceHandle;
            //System.Threading.
            //myServiceStatus.currentState = (int)State.SERVICE_START_PENDING;
            //SetServiceStatus(handle, myServiceStatus);

            // Start a separate thread that does the actual work.

            if ((m_workerThread == null) ||
                ((m_workerThread.ThreadState &
                 (System.Threading.ThreadState.Unstarted | System.Threading.ThreadState.Stopped)) != 0))
            {
                EventLog.WriteEntry("Creating the Runespy Service worker thread", DateTime.Now.ToLongTimeString() +
                    " - Starting the service worker thread.");

                m_workerThread = new Thread(new ThreadStart(ThreadMainMethod));
                m_workerThread.Start();
            }
            if (m_workerThread != null)
            {
                EventLog.WriteEntry("Starting the worker thread", DateTime.Now.ToLongTimeString() +
                    " - Worker thread state = " + m_workerThread.ThreadState.ToString());
            }
            //myServiceStatus.currentState = (int)State.SERVICE_RUNNING;
            //SetServiceStatus(handle, myServiceStatus);
        }

        private void ThreadMainMethod()
        {
            int timeBetweenReadsInMinutes = m_numMinutes;

            // setup today's date vars and output filename
            System.DateTime today = System.DateTime.Today;
            System.DateTime tomorrow = today.AddDays(1);

            string outputFilename = "C:\\runespy " + today.Month.ToString() + "_" + today.Day.ToString() + "_" + today.Year.ToString() + ".log";

            const string SITE_URL = "http://www.runescape.com/lang/en/aff/runescape/title.ws";
            const string FIRST_SEARCH_STRING = "There are currently ";
            const string SECOND_SEARCH_STRING = " people playing!";

            // setup the output file stream writer
            m_outStream = new System.IO.StreamWriter(new System.IO.FileStream(outputFilename, System.IO.FileMode.Append));

            // Process the next read time to be the most recent 15 minute increment
            DateTime nextReadTime = DateTime.Now;
            nextReadTime = nextReadTime.Subtract(System.TimeSpan.FromMilliseconds(nextReadTime.Millisecond));
            nextReadTime = nextReadTime.Subtract(System.TimeSpan.FromSeconds(nextReadTime.Second));
            nextReadTime = nextReadTime.Subtract(System.TimeSpan.FromMinutes(nextReadTime.Minute % timeBetweenReadsInMinutes));

            while (true)
            {
                System.Threading.Thread.Sleep(300); // save some of the CPU for other programs

                // if service is not in running mode then skip useful work
                if (!m_bRunning)
                {
                    continue;
                }
                else
                {
                    // reinitialize the time between reads in minutes
                    timeBetweenReadsInMinutes = m_numMinutes;
                }

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
                            outputFilename = "C:\\runespy " + today.Month.ToString() + "_" + today.Day.ToString() + "_" + today.Year.ToString() + ".log";

                            // close old file
                            m_outStream.Close();

                            // Force garbage collection
                            System.GC.Collect();

                            // create new file stream object
                            m_outStream = new System.IO.StreamWriter(new System.IO.FileStream(outputFilename, System.IO.FileMode.Append));
                        }

                        m_outStream.WriteLine(output);
                        m_outStream.Flush();

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

        #region RunespyService Data Members
        
        private Thread m_workerThread = null;
        private static int m_numMinutes;
        private static bool m_bRunning;
        private static System.IO.StreamWriter m_outStream;

        #endregion
    }
}
