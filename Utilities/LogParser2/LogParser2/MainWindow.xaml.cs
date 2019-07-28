namespace LogParser2
{
    using System;
    using System.ComponentModel;
    using System.IO;
    using System.Windows;
    using LogProcessing;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private LogParser logParser = new LogParser();

        public MainWindow()
        {
            InitializeComponent();

            this.logParser.ParsingStarted += this.LogParser_ParsingStarted;
            this.logParser.ParsingProgressed += this.LogParser_ParsingProgressed;
            this.logParser.ParsingFinished += this.LogParser_ParsingFinished;
        }

        private void LogParser_ParsingStarted(object sender, EventArgs e)
        {
            this.IsEnabled = false;
        }

        private void LogParser_ParsingProgressed(object sender, ProgressChangedEventArgs e)
        {
            if (e.ProgressPercentage == -1)
            {
                MessageBox.Show((string)e.UserState);
                return;
            }

            this.ParseProgressBar.Value = e.ProgressPercentage;
            this.ParseFileLabel.Content = e.UserState;
        }

        private void LogParser_ParsingFinished(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                throw new ApplicationException("LogParser", e.Error);
            }

            this.ParseProgressBar.Value = 0;
            this.ParseFileLabel.Content = null;

            this.IsEnabled = true;
        }

        private void ParseButton_Click(object sender, RoutedEventArgs e)
        {
            if (!Directory.Exists(this.LogDirectoryTextBox.Text))
            {
                MessageBox.Show(this, "Directory does not exist!");
                return;
            }

            if (string.IsNullOrEmpty(this.FileSpecTextBox.Text))
            {
                MessageBox.Show(this, "File spec hasn't been specified!");
                return;
            }

            if (string.IsNullOrEmpty(this.SearchTextBox.Text))
            {
                MessageBox.Show(this, "Search string hasn't been specified!");
                return;
            }

            if (string.IsNullOrEmpty(this.OutputFileTextBox.Text))
            {
                MessageBox.Show(this, "Target file hasn't been specified.");
                return;
            }
            
            DateTime startDate;
            if (!LogParser.TryParseDateMethodOne(this.StartDateTextBox.Text, out startDate))
            {
                MessageBox.Show(this, "Start date is missing or invalid.");
                return;
            }

            DateTime endDate;
            if (!LogParser.TryParseDateMethodOne(this.EndDateTextBox.Text, out endDate))
            {
                MessageBox.Show(this, "Start date is missing or invalid.");
                return;
            }

            this.logParser.ParseLogs(this.LogDirectoryTextBox.Text, this.FileSpecTextBox.Text, this.OutputFileTextBox.Text, this.SearchTextBox.Text, startDate, endDate);
        }
    }
}
