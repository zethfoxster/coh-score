using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;

namespace COH_CostumeUpdater.assetEditor.objectTrick
{
    public class ObjectTricksToolTips:common.COH_htmlToolTips
    {
        public static void intilizeToolTips(ArrayList comboControlsList, string htmlFile)
        {
            common.FormatedToolTip toolTip1 = new common.FormatedToolTip();

            toolTip1.AutoPopDelay = 20000;

            toolTip1.InitialDelay = 0;

            toolTip1.ReshowDelay = 0;

            toolTip1.ShowAlways = true;

            toolTip1.StripAmpersands = false;

            Dictionary<string, string> flagsDic = buildDictionary(htmlFile);

            // Set up the ToolTip text for the Button and Checkbox.
            foreach (CheckBox_SpinCombo cbsc in comboControlsList)
            {
                Control ctl = cbsc.checkBoxControl;

                toolTip1.SetToolTip(ctl, getText(cbsc.controlName, flagsDic));
            }
        }
    }
}
