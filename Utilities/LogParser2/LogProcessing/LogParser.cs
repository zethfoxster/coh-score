namespace LogProcessing
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using CompressionLibrary;
    using System.Text;

    public class LogParser
    {
        private static readonly TimeSpan TimeLeeway = TimeSpan.FromDays(2);

        public event EventHandler ParsingStarted;

        public event ProgressChangedEventHandler ParsingProgressed;

        public event RunWorkerCompletedEventHandler ParsingFinished;

        public static bool TryParseDate(byte[] logLine, out DateTime d)
        {
            if (logLine != null && logLine.Length >= 15)
            {
                string dateString = Encoding.ASCII.GetString(logLine, 0, 15);
                if (TryParseDateMethodOne(dateString, out d))
                {
                    return true;
                }
            }

            if (logLine != null && logLine.Length >= 19)
            {
                string dateString = Encoding.ASCII.GetString(logLine, 0, 19);
                if (TryParseDateMethodTwo(dateString, out d))
                {
                    return true;
                }
            }

            d = new DateTime();
            return false;
        }

        public static bool TryParseDateMethodOne(string dateString, out DateTime d)
        {
            if (string.IsNullOrEmpty(dateString))
            {
                d = new DateTime();
                return false;
            }

            if (dateString.Length != 15)
            {
                d = new DateTime();
                return false;
            }

            int year;
            if (!int.TryParse(dateString.Substring(0, 2), out year))
            {
                d = new DateTime();
                return false;
            }

            int month;
            if (!int.TryParse(dateString.Substring(2, 2), out month))
            {
                d = new DateTime();
                return false;
            }

            int day;
            if (!int.TryParse(dateString.Substring(4, 2), out day))
            {
                d = new DateTime();
                return false;
            }

            int hour;
            if (!int.TryParse(dateString.Substring(7, 2), out hour))
            {
                d = new DateTime();
                return false;
            }

            int minute;
            if (!int.TryParse(dateString.Substring(10, 2), out minute))
            {
                d = new DateTime();
                return false;
            }

            int second;
            if (!int.TryParse(dateString.Substring(13, 2), out second))
            {
                d = new DateTime();
                return false;
            }

            try
            {
                d = new DateTime(2000 + year, month, day, hour, minute, second);
                return true;
            }
            catch (Exception)
            {
                d = new DateTime();
                return false;
            }
        }

        public static bool TryParseDateMethodTwo(string dateString, out DateTime d)
        {
            if (string.IsNullOrEmpty(dateString))
            {
                d = new DateTime();
                return false;
            }

            if (dateString.Length != 19)
            {
                d = new DateTime();
                return false;
            }

            int month;
            if (!int.TryParse(dateString.Substring(0, 2), out month))
            {
                d = new DateTime();
                return false;
            }

            int day;
            if (!int.TryParse(dateString.Substring(3, 2), out day))
            {
                d = new DateTime();
                return false;
            }

            int year;
            if (!int.TryParse(dateString.Substring(6, 4), out year))
            {
                d = new DateTime();
                return false;
            }

            int hour;
            if (!int.TryParse(dateString.Substring(11, 2), out hour))
            {
                d = new DateTime();
                return false;
            }

            int minute;
            if (!int.TryParse(dateString.Substring(14, 2), out minute))
            {
                d = new DateTime();
                return false;
            }

            int second;
            if (!int.TryParse(dateString.Substring(17, 2), out second))
            {
                d = new DateTime();
                return false;
            }

            try
            {
                d = new DateTime(year, month, day, hour, minute, second);
                return true;
            }
            catch (Exception)
            {
                d = new DateTime();
                return false;
            }
        }

        public void ParseLogs(string sourceDirectory, string fileSpec, string outputFile, string search, DateTime startDate, DateTime endDate)
        {
            this.ParsingStarted(null, EventArgs.Empty);

            BackgroundWorker parseWorker = new BackgroundWorker();
            parseWorker.DoWork += this.ParseWorker_DoWork;
            parseWorker.ProgressChanged += this.ParseWorker_ProgressChanged;
            parseWorker.RunWorkerCompleted += this.ParseWorker_RunWorkerCompleted;
            parseWorker.WorkerReportsProgress = true;
            parseWorker.RunWorkerAsync(new object[] { sourceDirectory, fileSpec, outputFile, search, startDate, endDate });
        }

        private static byte[] ReadLogLine(Stream stream)
        {
            List<byte> internalBuffer = new List<byte>();
            int value;
            do
            {
                value = stream.ReadByte();
            }
            while (value == 0);

            while (value != -1)
            {
                internalBuffer.Add((byte)value);
                if (value == (byte)'\n')
                {
                    break;
                }

                value = stream.ReadByte();
            }

            return internalBuffer.ToArray();
        }

        private static byte[] MakeLowerCaseBuffer(byte[] lineBuffer)
        {
            byte[] clone = (byte[])lineBuffer.Clone();
            for (int i = 0; i < clone.Length; ++i)
            {
                if ('A' <= clone[i] && clone[i] <= 'Z')
                {
                    clone[i] = (byte)(clone[i] | ' ');
                }
            }

            return clone;
        }

        private static bool PassesSearch(byte[] lineBuffer, string search)
        {
            if (string.IsNullOrEmpty(search))
            {
                return true;
            }

            byte[] searchBytesLowerCase = Encoding.ASCII.GetBytes(search.ToLower());
            byte[] lineBufferLowerCase = MakeLowerCaseBuffer(lineBuffer);
            for (int bufferIndex = 0; bufferIndex < lineBufferLowerCase.Length - searchBytesLowerCase.Length + 1; bufferIndex++)
            {
                bool match = true;
                for (int searchIndex = 0; searchIndex < search.Length; searchIndex++)
                {
                    if (lineBufferLowerCase[bufferIndex + searchIndex] != searchBytesLowerCase[searchIndex])
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                {
                    return true;
                }
            }

            return false;
        }

        private void ParseWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = (BackgroundWorker)sender;
            object[] arguments = (object[])e.Argument;
            string sourceDirectory = (string)arguments[0];
            string fileSpec = (string)arguments[1];
            string outputFile = (string)arguments[2];
            string search = (string)arguments[3];
            DateTime startDate = (DateTime)arguments[4];
            DateTime endDate = (DateTime)arguments[5];

            string[] files = Directory.GetFiles(sourceDirectory, fileSpec, SearchOption.TopDirectoryOnly);

            int lastPercentage = 0;
            int nextPercentage = 0;
            using (FileStream outputFileStream = File.Create(outputFile))
            {
                for (int fileIndex = 0; fileIndex < files.Length; fileIndex++)
                {
                    nextPercentage = (int)((100.0 * (double)fileIndex) / (double)files.Length);
                    if (nextPercentage > lastPercentage)
                    {
                        lastPercentage = nextPercentage;
                    }

                    worker.ReportProgress(nextPercentage, "Processing " + Path.GetFileName(files[fileIndex]));

                    string appDataFolder = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
                    string folderPath = Path.Combine(appDataFolder, "LogParser2");
                    string unzippedFileName = Path.Combine(folderPath, "logparser2.tmp");

                    if (startDate > File.GetLastWriteTime(files[fileIndex]) + TimeLeeway)
                    {
                        continue;
                    }

                    Directory.CreateDirectory(folderPath);
                    File.Delete(unzippedFileName);
                    GZipper.Read(files[fileIndex], unzippedFileName);

                    using (FileStream unzippedStream = File.OpenRead(unzippedFileName))
                    {
                        int count = 0;
                        byte[] lineBuffer;
                        bool firstLine = true;
                        while ((lineBuffer = ReadLogLine(unzippedStream)).Length > 0)
                        {
                            DateTime logDate;
                            bool dateLine = TryParseDate(lineBuffer, out logDate);
                            if (firstLine && dateLine)
                            {
                                if (endDate < logDate - TimeLeeway)
                                {
                                    break;
                                }

                                firstLine = false;
                            }

                            if (dateLine && logDate >= startDate && logDate <= endDate)
                            {
                                if (PassesSearch(lineBuffer, search))
                                {
                                    count++;
                                    outputFileStream.Position = outputFileStream.Length;
                                    outputFileStream.Write(lineBuffer, 0, lineBuffer.Length);
                                }
                            }
                        }
                    }

                    File.Delete(unzippedFileName);
                }
            }
        }

        private void ParseWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            this.ParsingProgressed(this, new ProgressChangedEventArgs(e.ProgressPercentage, e.UserState));
        }

        private void ParseWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.ParsingFinished(this, e);
        }
    }
}
