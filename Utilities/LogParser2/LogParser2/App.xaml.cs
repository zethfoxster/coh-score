namespace LogParser2
{
    using System;
    using System.Windows;
    using System.Windows.Threading;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_DispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
        {
            Exception ex = e.Exception.Message == "LogParser" ? e.Exception.InnerException : e.Exception;
            string message =
                ex.Message + "\n" +
                "\n" +
                "\n" +
                "\n" +
                "----------------------------------------------------------------\n" +
                "Debug Info:\n" +
                "\n" +
                "Exception: " + ex.GetType() + "\n" +
                "\n" +
                "Stack Trace (for programmers):\n" +
                ex.StackTrace;
            MessageBox.Show(message, "Application Must Exit", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }
}
