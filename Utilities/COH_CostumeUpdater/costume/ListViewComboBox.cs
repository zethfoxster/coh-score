using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;


namespace COH_CostumeUpdater.costume
{
    class ListViewComboBox : System.Windows.Forms.ListView
    {
      private System.ComponentModel.Container components = null;

      public ListViewComboBox()
      {
        InitializeComponent();
      }

      protected override void Dispose( bool disposing )
      {
         if( disposing )
         {
            if( components != null )
               components.Dispose();
         }
         base.Dispose( disposing );
      }

      #region Component Designer generated code
      private void InitializeComponent()
      {
         components = new System.ComponentModel.Container();
      }
      #endregion

      private const int WM_HSCROLL = 0x114;
      private const int WM_VSCROLL = 0x115;

      protected override void WndProc(ref Message msg)
      {
         // Look for the WM_VSCROLL or the WM_HSCROLL messages.
         if ((msg.Msg == WM_VSCROLL) || (msg.Msg == WM_HSCROLL))
         {
            // Move focus to the ListView to cause ComboBox to lose focus.
            this.Focus();  
         }

         // Pass message to default handler.
         base.WndProc(ref msg);
      } 

    }
}
