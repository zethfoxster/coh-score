using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;

namespace COH_CostumeUpdater.common
{
    public class FormatedToolTip : ToolTip
    {
        public FormatedToolTip()
        {
            this.OwnerDraw = true;

            this.Draw += new DrawToolTipEventHandler(OnDraw);

            this.Popup += new PopupEventHandler(OnPopup);
        }

        public FormatedToolTip(System.ComponentModel.IContainer Cont)
        {
            this.OwnerDraw = false;

            this.Draw += new DrawToolTipEventHandler(OnDraw);
        }

        private void OnPopup(object sender, PopupEventArgs e)
        {
            Font f = new Font("Helvetica", 9, FontStyle.Bold);

            e.ToolTipSize = TextRenderer.MeasureText(this.GetToolTip(e.AssociatedControl), f);
            int h = (int) Math.Ceiling(e.ToolTipSize.Height * 0.92);
            int w = e.ToolTipSize.Width;
            e.ToolTipSize = new Size(w, h);
        }

        private void OnDraw(object sender, DrawToolTipEventArgs e)
        {
            UnicodeEncoding uniEncoding = new UnicodeEncoding();

            Graphics g = e.Graphics;

            e.Graphics.FillRectangle(new SolidBrush(this.BackColor), e.Bounds);

            byte[] toolTipString = uniEncoding.GetBytes(e.ToolTipText);

            MemoryStream memStream = new MemoryStream(toolTipString);

            StreamReader stmRdr = new System.IO.StreamReader(memStream);

            string lineStr;

            int pos = 0;

            int p = 0;
            
            while ((lineStr = stmRdr.ReadLine()) != null)
            {

                string line = lineStr.Replace("\0", "");

                Color clr = getColor(line);

                string trimedLine = line.Trim("\t".ToCharArray());

                string htmlText = "";

                if (trimedLine.Length > 0)
                {
                    if (trimedLine.ToLower().StartsWith("<h1"))
                    {
                        string dlimit = "</h1";

                        int start = trimedLine.IndexOf(">") + 1;

                        int len = trimedLine.IndexOf(dlimit) - start;

                        if (len != -1 && start != -1)
                        {
                            htmlText = trimedLine.Substring(start, len);

                            p = drawText(e, clr, htmlText, 1, pos);
                        }
                    }
                    else if (trimedLine.ToLower().StartsWith("<h2"))
                    {
                        string dlimit = "</h2";

                        int start = trimedLine.IndexOf(">") + 1;

                        int len = trimedLine.IndexOf(dlimit) - start;

                        if (len != -1 && start != -1)
                        {
                            htmlText = trimedLine.Substring(start, len);// +"\n";

                            p = drawText(e, clr, htmlText, 2, pos);
                        }
                    }
                    else if (trimedLine.ToLower().StartsWith("<h3"))
                    {
                        string dlimit = "</h3";

                        int start = trimedLine.IndexOf(">") + 1;

                        int len = trimedLine.IndexOf(dlimit) - start;

                        if (len != -1 && start != -1)
                        {
                            htmlText = trimedLine.Substring(start, len);// +"\n";

                            p = drawText(e, clr, htmlText, 3, pos);
                        }
                    }
                    else
                    {
                        htmlText = "      " + line.Replace("<p>", "").Replace("</p>", "").Replace("<P>", "").Replace("</P>", "");

                        p = drawText(e, clr, htmlText, 0, pos);
                    }
                }

                pos += p;
            }

            stmRdr.Close();

            memStream.Close();
        }

        private Color getColor(string line)
        {
            if (line.ToLower().Contains("style=\"color:"))
            {
                int start = line.IndexOf(":") + 1;

                int len = line.IndexOf("\"", start) - start;

                string c = line.Substring(start, len).Trim();

                string[] cSplit = c.Split(' ');

                switch (cSplit.Length)
                {
                    case 3:
                        int r = 0, g = 0, b = 0;
                        int.TryParse(cSplit[0].Trim(), out r);
                        int.TryParse(cSplit[1].Trim(), out g);
                        int.TryParse(cSplit[2].Trim(), out b);

                        if (r + g + b > 0)
                            return Color.FromArgb(255, r, g, b);
                        break;

                    case 1:
                        if (cSplit[0].Trim().StartsWith("#"))
                        {
                            int cl = 0;
                            int.TryParse(cSplit[0].Trim().Replace("#", "FF"), System.Globalization.NumberStyles.HexNumber, System.Globalization.CultureInfo.InvariantCulture, out cl);
                            return Color.FromArgb(cl);
                        }

                        return Color.FromName(cSplit[0].Trim());
                }
            }

            return this.ForeColor;
        }

        private int drawText(DrawToolTipEventArgs e, Color clr, string htmlText, int hType, int pos)
        {
            int hsize = 8;

            Font f = new Font("Helvetica", hsize, FontStyle.Bold);

            switch (hType)
            {
                case 1:
                    hsize = 14;
                    f = new Font("Helvetica", hsize, FontStyle.Bold);
                    break;

                case 2:
                    hsize = 12;
                    f = new Font("Helvetica", hsize, FontStyle.Bold);
                    break;

                case 3:
                    hsize = 10;
                    f = new Font("Helvetica", hsize, FontStyle.Bold);
                    break;

                default:
                    hsize = 8;
                    f = new Font("Helvetica", hsize, FontStyle.Regular);
                    break;
            }

            return htmlLineRenderer(e, clr, f, htmlText, pos);
        }

        private int htmlLineRenderer(DrawToolTipEventArgs e, Color c, Font f, string htmlText, int h)
        {
            TextFormatFlags txF = TextFormatFlags.WordBreak;

            string drawTxt = htmlText.Replace("&lt;", "<").Replace("&gt;", ">").Replace("&#45;", "-").Replace("&#35;", "#");

            TextRenderer.DrawText(e.Graphics, drawTxt, f, new Point(e.Bounds.X, e.Bounds.Y + h), c, txF);

            return f.Height - 7;
        }

    }

    public class COH_ToolTipObject
    {
        public Control ttControl;
        public DataGridViewColumn ttDataGridViewColumn;
        public string ttControlName;

        public COH_ToolTipObject(Control ctl, string nameStr)
        {
            this.ttControl = ctl;
            this.ttDataGridViewColumn = null;
            this.ttControlName = nameStr;
        }

        public COH_ToolTipObject(DataGridViewColumn dgvc, string nameStr)
        {
            this.ttControl = null;
            this.ttDataGridViewColumn = dgvc;
            this.ttControlName = nameStr;
        }
    }

    public class COH_htmlToolTips
    {
        public static FormatedToolTip intilizeToolTips(ArrayList coh_ToolTipsControlList, string htmlFile)
        {
            FormatedToolTip toolTip1 = new FormatedToolTip();

            toolTip1.AutoPopDelay = 20000;

            toolTip1.InitialDelay = 0;

            toolTip1.ReshowDelay = 0;

            toolTip1.ShowAlways = true;

            toolTip1.StripAmpersands = false;

            Dictionary<string, string> flagsDic = buildDictionary(htmlFile);

            // Set up the ToolTip text for the controls
            foreach (COH_ToolTipObject cohTTO in coh_ToolTipsControlList)
            {
                if(cohTTO.ttControl != null)
                    toolTip1.SetToolTip(cohTTO.ttControl, getText(cohTTO.ttControlName, flagsDic));

                if (cohTTO.ttDataGridViewColumn != null && flagsDic.ContainsKey(cohTTO.ttControlName))
                    cohTTO.ttDataGridViewColumn.ToolTipText = flagsDic[cohTTO.ttControlName];
            }
            return toolTip1;
        }

        public static Dictionary<string, string> getToolTipsDic(string htmlFile)
        {
            return buildDictionary(htmlFile);
        }

        public static Dictionary<string, string> buildDictionary(string htmlFile)
        {
            StreamReader sr = new StreamReader(htmlFile);

            Dictionary<string, string> flags = new Dictionary<string, string>();

            string line;

            while ((line = sr.ReadLine()) != null)
            {
                if (line.ToLower().StartsWith("<id=\""))
                {
                    int start = line.IndexOf("\"") + 1;

                    int len = line.IndexOf("\"", start) - start;

                    string key = line.Substring(start, len);

                    string val = "";

                    bool exitLoop = false;

                    while ((line = sr.ReadLine()) != null && !exitLoop)
                    {

                        if (!line.StartsWith("</id>"))
                        {
                            val += line + "\r\n";
                        }
                        else
                        {
                            exitLoop = true;
                        }
                    }
                    flags[key] = val;
                }
            }

            sr.Close();

            return flags;
        }

        public static string formatString(string str)
        {
            string results = "";

            int maxLineLen = 120;

            int maxCount = 0;

            char[] cArray = str.Replace("\r","\n").Replace("\n\n","\n").ToCharArray();

            for (int count = 0; count < cArray.Length; count++)
            {
                bool isNewLine = false;

                if (cArray[count] == '\n')
                {
                    isNewLine = true;
                    maxCount = maxLineLen;
                }

                if (maxCount < maxLineLen)
                {
                    results += cArray[count];
                    maxCount++;
                }
                else
                {
                    bool exit = false;
                    while (!exit & !isNewLine)
                    {
                        if (count < cArray.Length &&
                            !cArray[count].ToString().Equals(" "))
                        {
                            results += cArray[count];
                            count++;
                        }
                        else
                        {
                            exit = true;
                        }
                    }

                    results += "\r\n";
                    maxCount = 0;
                }
            }

            return results;
        }

        public static string getText(string ControlName, Dictionary<string, string> flags)
        {
            string str = "";

            if (flags.ContainsKey(ControlName))
                return formatString(flags[ControlName]);

            return str;
        }

    }
}
