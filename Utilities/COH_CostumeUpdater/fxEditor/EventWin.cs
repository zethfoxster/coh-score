using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    public partial class EventWin : Panel
        //Form
    {
        public event EventHandler CurrentRowChanged;

        public event EventHandler IsDirtyChanged;

        public event EventHandler ENameChanged;

        public bool isDirty;

        private Dictionary<string, string> htmlDic;

        private common.FormatedToolTip dgvTooltip;

        private bool cellIsChanging;

        private bool settingCells;

        private Event currentRowEvent;

        private COH_CostumeUpdaterForm cuf;

        private ArrayList bones;

        public EventWin()
        {
            cellIsChanging = false;

            bones = new ArrayList();

            common.COH_IO.fillList(bones, @"assetEditor/objectTrick/bones.txt", true);

            InitializeComponent();

            initializeToolTip();
            
            this.At.Items.AddRange(bones.ToArray());

            foreach (DataGridViewColumn dgvc in this.dataGridViewEvents.Columns)
            {
                dgvc.Visible = false;
            }
        }

        public DataGridView getDGV()
        {
            return this.dataGridViewEvents;
        }

        public void setEventsData(FX fx)
        {
            settingCells = true;

            if (this.dataGridViewEvents.Rows.Count > 0)
                this.dataGridViewEvents.Rows.Clear();

            foreach (Condition condition in fx.conditions)
            {
                foreach (Event ev in condition.events)
                {
                    setEventParameters(ev);
                }
            }
            //requested by Leo to be allways visible
            this.dataGridViewEvents.Columns["EName"].Visible = true;
            this.dataGridViewEvents.Columns["At"].Visible = true;
            this.dataGridViewEvents.Columns["Type"].Visible = true;
            settingCells = false;
        }

        public Event CurrentRowEvent
        {
            get
            {
                return currentRowEvent;
            }
            set
            {
                currentRowEvent = value;
            }
        }

        //private void OnCurrentRowChanged(EventArgs e)
        //{
        //    if (CurrentRowChanged != null)
        //        CurrentRowChanged(this, e);
        //}

        public void updateFXEventsData()
        {
            foreach (DataGridViewRow dgr in this.dataGridViewEvents.Rows)
            {
                getEventRowData(dgr);
            }
        }
       
        private void initializeToolTip()
        {
            dgvTooltip  = new common.FormatedToolTip();

            dgvTooltip.AutoPopDelay = 20000;

            dgvTooltip.InitialDelay = 0;

            dgvTooltip.ReshowDelay = 0;

            dgvTooltip.ShowAlways = true;

            dgvTooltip.StripAmpersands = false;

            htmlDic = common.COH_htmlToolTips.getToolTipsDic(@"assetEditor/objectTrick/FxToolTips.html");
        }

        private void dataGridViewEvents_CellMouseEnter(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex > -1 && e.ColumnIndex > -1 && ModifierKeys == Keys.Control)
            {
                DataGridViewRow dgvr = dataGridViewEvents.Rows[e.RowIndex];

                string colName = dataGridViewEvents.Columns[e.ColumnIndex].Name;

                DataGridViewCell dgvc = dgvr.Cells[e.ColumnIndex];

                string toolTipText = common.COH_htmlToolTips.getText(colName,htmlDic);

                if (toolTipText.Trim().Length > 0)
                {
                    Point p = this.PointToClient(Cursor.Position);
                    dgvTooltip.Show(toolTipText, this, p.X + 40, p.Y + 20, dgvTooltip.AutoPopDelay);
                }               
            }
        }

        private void dataGridViewEvents_CellMouseLeave(object sender, DataGridViewCellEventArgs e)
        {
            dgvTooltip.Hide(this);
            dataGridViewEvents.EndEdit();
        }

        private void clearEventParameters(ref Event ev)
        {
            string[] keys = ev.eParameters.Keys.ToArray<string>();

            foreach (string k in keys)
            {
                ev.eParameters[k] = null;
            }
        }

        private void updateGeomData(ref Event ev, DataGridViewRow dgr)
        {
            ev.eGeoms.Clear();

            if (dgr.Cells["Geom"].Visible &&
                dgr.Cells["Geom"].Value != null)
            {
                string []geoms = ((string)dgr.Cells["Geom"].Value).Trim().Split(';');

                foreach (string geom in geoms)
                {
                    if (geom.Trim().Length > 0)
                    {
                        ev.eGeoms.Add(geom.Trim());
                    }
                }
            }
        }

        private void updateEventInherit(DataGridViewRow dgr, ref Event ev)
        {
            bool isCommented;

            string commentStr = "";

            string valStr = "";        

            if(dgr.Cells["Inherit_Position"].Visible &&
                dgr.Cells["Inherit_Position"].Value != null &&
                (bool)dgr.Cells["Inherit_Position"].Value)
            {
                isCommented = dgr.Cells["Inherit_Position"].Style.ForeColor == Color.Red;
                
                if(isCommented)
                    commentStr += " Position";
                else
                    valStr += " Position";
            }
            if( dgr.Cells["Inherit_Rotation"].Visible &&
                dgr.Cells["Inherit_Rotation"].Value != null &&
                (bool)dgr.Cells["Inherit_Rotation"].Value)
            {
                isCommented = dgr.Cells["Inherit_Rotation"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " Rotation";
                else
                    valStr += " Rotation";
            }
            if( dgr.Cells["Inherit_Scale"].Visible &&
                dgr.Cells["Inherit_Scale"].Value != null &&
                (bool)dgr.Cells["Inherit_Scale"].Value)
            {
                isCommented = dgr.Cells["Inherit_Scale"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " Scale";
                else
                    valStr += " Scale";
            }

            if (valStr.Contains("Position") &&
                valStr.Contains("Rotation") &&
                valStr.Contains("Scale"))
            {
                valStr = "ALL";
            }

            if (commentStr.Contains("Position") &&
                commentStr.Contains("Rotation") &&
                commentStr.Contains("Scale"))
            {
                commentStr = "ALL";
            }

            if( dgr.Cells["Inherit_SuperRotation"].Visible &&
                dgr.Cells["Inherit_SuperRotation"].Value != null &&
                (bool)dgr.Cells["Inherit_SuperRotation"].Value)
            {
                isCommented = dgr.Cells["Inherit_SuperRotation"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " SuperRotation";
                else
                    valStr += " SuperRotation";
            }

            if(valStr.Length == 0 && commentStr.Length == 0)
                valStr = "NONE";


            if (dgr.Cells["Inherit_Position"].Visible ||
               dgr.Cells["Inherit_Rotation"].Visible ||
               dgr.Cells["Inherit_Scale"].Visible ||
               dgr.Cells["Inherit_SuperRotation"].Visible)
            {
                ev.eParameters["Inherit"] = (valStr.Trim() + " #" + commentStr.Trim()).Trim();
            }
        }

        private void updateEventPosRotSclSRUpdates(DataGridViewRow dgr, ref Event ev)
        {
            bool isCommented;

            string commentStr = "";

            string valStr = "";

            if (dgr.Cells["Update_Position"].Visible &&
                dgr.Cells["Update_Position"].Value != null &&
                (bool)dgr.Cells["Update_Position"].Value)
            {
                isCommented = dgr.Cells["Update_Position"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " Position";
                else
                    valStr += " Position";
            }

            if (dgr.Cells["Update_Rotation"].Visible &&
                dgr.Cells["Update_Rotation"].Value != null &&
                (bool)dgr.Cells["Update_Rotation"].Value)
            {
                isCommented = dgr.Cells["Update_Rotation"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " Rotation";
                else
                    valStr += " Rotation";
            }

            if (dgr.Cells["Update_Scale"].Visible &&
                dgr.Cells["Update_Scale"].Value != null &&
                (bool)dgr.Cells["Update_Scale"].Value)
            {
                isCommented = dgr.Cells["Update_Scale"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " Scale";
                else
                    valStr += " Scale";
            }

            if (valStr.Contains("Position") &&
                valStr.Contains("Rotation") &&
                valStr.Contains("Scale"))
            {
                valStr = "ALL";
            }

            if (commentStr.Contains("Position") &&
                commentStr.Contains("Rotation") &&
                commentStr.Contains("Scale"))
            {
                commentStr = "ALL";
            }

            if (dgr.Cells["Update_SuperRotation"].Visible &&
                dgr.Cells["Update_SuperRotation"].Value != null &&
                (bool)dgr.Cells["Update_SuperRotation"].Value)
            {
                isCommented = dgr.Cells["Update_SuperRotation"].Style.ForeColor == Color.Red;

                if (isCommented)
                    commentStr += " SuperRotation";
                else
                    valStr += " SuperRotation";
            }

            if (valStr.Length == 0 && commentStr.Length == 0)
                valStr = "NONE";

            if (dgr.Cells["Update_Position"].Visible ||
               dgr.Cells["Update_Rotation"].Visible ||
               dgr.Cells["Update_Scale"].Visible ||
               dgr.Cells["Update_SuperRotation"].Visible)
            {
                ev.eParameters["Update"] = (valStr.Trim() + " #" + commentStr.Trim()).Trim();
            }
        }

        private void updateEventPower(DataGridViewRow dgr, ref Event ev)
        {
            decimal min, max;

            string minStr = null, maxStr = null;

            bool commentMin = dgr.Cells["PowerMin"].Style.ForeColor == Color.Red;

            bool commentMax = dgr.Cells["PowerMax"].Style.ForeColor == Color.Red;

            if (dgr.Cells["PowerMin"].Visible && 
                dgr.Cells["PowerMin"].Value != null)
            {
                minStr = dgr.Cells["PowerMin"].Value.ToString();

                if (decimal.TryParse(minStr, out min))
                    minStr = min.ToString();
            }
            if (dgr.Cells["PowerMax"].Visible &&
                dgr.Cells["PowerMax"].Value != null)
            {
                maxStr = dgr.Cells["PowerMax"].Value.ToString();

                if (decimal.TryParse(maxStr, out max))
                    maxStr = max.ToString();
            }
            if (minStr != null && maxStr != null)
            {
                if(!commentMin && !commentMax)
                    ev.eParameters["Power"] = minStr + " " + maxStr;
                else
                    ev.eParameters["Power"] = "#" + minStr + " " + maxStr;
            }
        }

        private void getEventRowData(DataGridViewRow dgr)
        {
            if (dgr.Tag != null)
            {
                Event ev = (Event)dgr.Tag;

                clearEventParameters(ref ev);

                updateGeomData(ref ev, dgr);

                bool inheritKeyDone = false;

                bool updateKeyDone = false;

                bool powerKeyDone = false;

                foreach (DataGridViewCell dgc in dgr.Cells)
                {
                    string colName = this.dataGridViewEvents.Columns[dgc.ColumnIndex].Name;

                    if (dgc.Value != null && !dgc.ReadOnly)
                    {
                        string valueStr = null;

                        if (colName.StartsWith("Inherit"))
                        {
                            //done implementing comment
                            if (!inheritKeyDone)
                            {
                                updateEventInherit(dgr, ref ev);
                                inheritKeyDone = true;
                            }
                            continue;
                        }

                        else if (colName.StartsWith("Update"))
                        {
                            //done implementing comment
                            if (!updateKeyDone)
                            {
                                updateEventPosRotSclSRUpdates(dgr, ref ev);
                                updateKeyDone = true;
                            }
                            continue;
                        }

                        else if (colName.StartsWith("Power"))
                        {
                            //done implementing comment
                            if (!powerKeyDone)
                            {
                                updateEventPower(dgr, ref ev);
                                powerKeyDone = true;
                            }
                            continue;
                        }

                        else if (ev.eParameters.ContainsKey(colName))
                        {
                            switch (dgc.GetType().Name)
                            {
                                case "DataGridViewNumericUpDownCell":
                                    string dStr = dgc.Value.ToString();
                                    decimal dec;
                                    if (decimal.TryParse(dStr, out dec))
                                        valueStr = dec.ToString();
                                    break;

                                case "DataGridViewTextBoxCell":
                                case "DataGridViewComboBoxCell":
                                    valueStr = (string)dgc.Value;
                                    break;

                                case "DataGridViewCheckBoxCell":
                                case "COH_DataGridViewCheckBoxCell":
                                    valueStr = (bool)dgc.Value ? "1" : "0";
                                    break;
                            }
                        }
                        
                        bool isCommented = dgc.Style.ForeColor == Color.Red;

                        //done implementing comment
                        if(ev.eParameters.ContainsKey(colName))
                            ev.eParameters[colName] = isCommented ? ("#" + valueStr) : valueStr;
                    }
                }
            }
        }

        private void dataGridView_CurrentCellChanged(object sender, System.EventArgs e)
        {
            if (dataGridViewEvents.CurrentCell != null)
            {
                Event ev = currentRowEvent;
                
                currentRowEvent = (Event)dataGridViewEvents.Rows[dataGridViewEvents.CurrentCell.RowIndex].Tag;
                
                if (currentRowEvent != null && currentRowEvent != ev)
                {
                    if (CurrentRowChanged != null)
                        CurrentRowChanged(this, e);

                    dataGridViewEvents.InvalidateCell(dataGridViewEvents.CurrentCell);
                    
                    dataGridViewEvents.Invalidate();
                }
            }
        }

        private void setCellColor(string colName, int rowInd, bool isCommented)
        {
            DataGridViewCellStyle dgcStyle = dataGridViewEvents.ColumnHeadersDefaultCellStyle;

            if (isCommented)
            {
                dataGridViewEvents.Rows[rowInd].Cells[colName].Style.ForeColor = Color.Red;
                dataGridViewEvents.Rows[rowInd].Cells[colName].Style.Font = new Font(dgcStyle.Font, FontStyle.Bold | FontStyle.Italic);
                if (dataGridViewEvents.Rows[rowInd].Cells[colName].GetType().Name.Equals("DataGridViewCheckBoxCell"))
                {
                    DataGridViewCheckBoxCell dgck = (DataGridViewCheckBoxCell)dataGridViewEvents.Rows[rowInd].Cells[colName];
                    dgck.Style.ForeColor = Color.Red;
                }
                else if (dataGridViewEvents.Rows[rowInd].Cells[colName].GetType().Name.Equals("COH_DataGridViewCheckBoxCell"))
                {
                    common.COH_DataGridViewCheckBoxCell dgck = (common.COH_DataGridViewCheckBoxCell)dataGridViewEvents.Rows[rowInd].Cells[colName];
                    dgck.Style.ForeColor = Color.Red;
                }
            }
            else
            {
                dataGridViewEvents.Rows[rowInd].Cells[colName].Style.ForeColor = dgcStyle.ForeColor;
                dataGridViewEvents.Rows[rowInd].Cells[colName].Style.Font = new Font(dgcStyle.Font, FontStyle.Regular);
            }
        }

        public void setEventParameters(Event efEvent)
        {
            DataGridViewCellStyle dgcStyle = dataGridViewEvents.ColumnHeadersDefaultCellStyle;

            int ind = this.dataGridViewEvents.Rows.Add();

            efEvent.mEventWinDGIndx = ind;

            dataGridViewEvents.Rows[ind].Tag = efEvent;

            if (efEvent.eGeoms.Count > 0)
            {
                string geoms = "";
                
                bool is_Commented = false;

                foreach (string geom in efEvent.eGeoms)
                {
                    if (!is_Commented)
                    {
                        is_Commented = geom.Trim().StartsWith("#");
                    }
                    geoms += geom + "; ";
                }

                setCellColor("Geom", ind, is_Commented);

                DataGridViewCell geomDgc = dataGridViewEvents.Rows[ind].Cells["Geom"];
                
                this.dataGridViewEvents.Columns["Geom"].Visible = true;
                
                geomDgc.Value = geoms.Trim();
                
                this.dataGridViewEvents.Columns["Geom"].AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;                
            }

            Dictionary<string, object> evEParam = efEvent.eParameters.ToDictionary(k => k.Key, k => k.Value);

            foreach (KeyValuePair<string, object> kvp in evEParam)
            {
                if (kvp.Value != null)
                {
                    bool isCommented = ((string)kvp.Value).Trim().StartsWith("#");

                    string kvpVal = isCommented ? ((string)kvp.Value).Trim().Substring(1) : ((string)kvp.Value).Trim();

                    #region if (kvp.Key.StartsWith("Inherit") || kvp.Key.StartsWith("Update")
                    
                    if (kvp.Key.StartsWith("Inherit") || kvp.Key.StartsWith("Update"))
                    {   
                        string[] kvpValSplit = kvpVal.Split(' ');

                        foreach (string str in kvpValSplit)
                        {
                            if (str.ToLower().Equals("none"))
                            {
                                if (kvp.Key.Equals("Update"))
                                {
                                    dataGridViewEvents.Columns["Update_Position"].Visible = true;
                                    dataGridViewEvents.Columns["Update_Rotation"].Visible = true;
                                    dataGridViewEvents.Columns["Update_Scale"].Visible = true;
                                    dataGridViewEvents.Columns["Update_SuperRotation"].Visible = true;

                                    setCellColor("Update_Position",ind,isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_Position"].Value = false;
                                    
                                    setCellColor("Update_Rotation",ind,isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_Rotation"].Value = false;
                                    
                                    setCellColor("Update_Scale", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_Scale"].Value = false;
                                    
                                    setCellColor("Update_SuperRotation", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_SuperRotation"].Value = false;
                                }
                                else
                                {
                                    dataGridViewEvents.Columns["Inherit_Position"].Visible = true;
                                    dataGridViewEvents.Columns["Inherit_Rotation"].Visible = true;
                                    dataGridViewEvents.Columns["Inherit_Scale"].Visible = true;
                                    dataGridViewEvents.Columns["Inherit_SuperRotation"].Visible = true;

                                    setCellColor("Inherit_Position", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_Position"].Value = false;
                                    
                                    setCellColor("Inherit_Rotation", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_Rotation"].Value = false;
                                    
                                    setCellColor("Inherit_Scale", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_Scale"].Value = false;
                                   
                                    setCellColor("Inherit_SuperRotation", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_SuperRotation"].Value = false;
                                }
                            }
                            else if (str.ToLower().Equals("all"))
                            {
                                if (kvp.Key.Equals("Update"))
                                {
                                    dataGridViewEvents.Columns["Update_Position"].Visible = true;
                                    dataGridViewEvents.Columns["Update_Rotation"].Visible = true;
                                    dataGridViewEvents.Columns["Update_Scale"].Visible = true;
                                    //dataGridViewEvents.Columns["Update_SuperRotation"].Visible = true;

                                    setCellColor("Update_Position", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_Position"].Value = true;
                                    
                                    setCellColor("Update_Rotation", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_Rotation"].Value = true;
                                    
                                    setCellColor("Update_Scale", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Update_Scale"].Value = true;
                                    
                                    //setCellColor("Update_SuperRotation", ind, isCommented);
                                    //dataGridViewEvents.Rows[ind].Cells["Update_SuperRotation"].Value = true;
                                }
                                else
                                {
                                    dataGridViewEvents.Columns["Inherit_Position"].Visible = true;
                                    dataGridViewEvents.Columns["Inherit_Rotation"].Visible = true;
                                    dataGridViewEvents.Columns["Inherit_Scale"].Visible = true;
                                    //dataGridViewEvents.Columns["Inherit_SuperRotation"].Visible = true;

                                    setCellColor("Inherit_Position", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_Position"].Value = true;
                                    
                                    setCellColor("Inherit_Rotation", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_Rotation"].Value = true;
                                    
                                    setCellColor("Inherit_Scale", ind, isCommented);
                                    dataGridViewEvents.Rows[ind].Cells["Inherit_Scale"].Value = true;
                                    
                                    //setCellColor("Inherit_SuperRotation", ind, isCommented);
                                    //dataGridViewEvents.Rows[ind].Cells["Inherit_SuperRotation"].Value = true;
                                }
                            }
                            else if(!str.Equals("#"))
                            {
                                string colName = kvp.Key + "_" + str;
                                setCellColor(colName, ind, isCommented);
                                DataGridViewCell dgcIU = dataGridViewEvents.Rows[ind].Cells[colName];
                                this.dataGridViewEvents.Columns[colName].Visible = true;
                                dgcIU.Value = true;
                            }                            
                        }
                    }
                    #endregion

                    #region if (kvp.Key.StartsWith("Power"))

                    else if (kvp.Key.StartsWith("Power"))
                    {
                        decimal min, max;

                        string[] powerSplit = kvpVal.Split(' ');

                        setCellColor("PowerMin", ind, isCommented);

                        setCellColor("PowerMax", ind, isCommented);

                        if (decimal.TryParse(powerSplit[0], out min) && powerSplit.Length > 0)
                        {
                            DataGridViewCell dgcIU = dataGridViewEvents.Rows[ind].Cells["PowerMin"];
                            
                            this.dataGridViewEvents.Columns["PowerMin"].Visible = true;
                            
                            dgcIU.Value = min;
                        }

                        if (decimal.TryParse(powerSplit[1], out max) && powerSplit.Length > 1)
                        {
                            DataGridViewCell dgcIU = dataGridViewEvents.Rows[ind].Cells["PowerMax"];
                            
                            this.dataGridViewEvents.Columns["PowerMax"].Visible = true;
                            
                            dgcIU.Value = max;
                        }
                    }
                    #endregion
                        
                    #region if (dataGridViewEvents.Columns.Contains(kvp.Key))
                    
                    else if (dataGridViewEvents.Columns.Contains(kvp.Key))
                    {
                        DataGridViewCell dgc = dataGridViewEvents.Rows[ind].Cells[kvp.Key];

                        setCellColor(kvp.Key, ind, isCommented);

                        this.dataGridViewEvents.Columns[kvp.Key].Visible = true;

                        switch (dgc.GetType().Name)
                        {
                            case "DataGridViewNumericUpDownCell":
                                decimal dec;
                                if (decimal.TryParse(kvpVal, out dec))
                                    dgc.Value = dec;
                                break;

                            case "DataGridViewTextBoxCell":
                                dgc.Value = kvpVal;
                                break;

                            case "DataGridViewComboBoxCell":
                                if (((DataGridViewComboBoxColumn)dataGridViewEvents.Columns[kvp.Key]).Items.Contains(kvpVal))
                                    dgc.Value = kvpVal;
                                else
                                {
                                    ((DataGridViewComboBoxColumn)dataGridViewEvents.Columns[kvp.Key]).Items.Add(kvpVal);
                                    dgc.Value = kvpVal;
                                }
                                break;

                            case "COH_DataGridViewCheckBoxCell":
                            case "DataGridViewCheckBoxCell":
                                dgc.Value = kvpVal.Equals("0") ? false : true;
                                break;
                        }
                    }
                    #endregion

                    else if (kvp.Key.Equals("Splat"))
                    {
                        continue;
                    }
                    else
                    {
                        MessageBox.Show(kvp.Key + " is not a dataGrid column");
                    }
                }
            }

            dataGridViewEvents.AllowUserToAddRows = false;
            dataGridViewEvents.AllowUserToDeleteRows = false;
        }

        public void selectEventRow(Event efEvent)
        {
            this.dataGridViewEvents.ClearSelection();

            if (efEvent.mEventWinDGIndx != -1 &&
                efEvent.mEventWinDGIndx < this.dataGridViewEvents.Rows.Count)
            {
                DataGridViewRow dgr = this.dataGridViewEvents.Rows[efEvent.mEventWinDGIndx];
                dgr.Selected = true;
                foreach (DataGridViewCell dgvc in dgr.Cells)
                {
                    if (dgvc.Visible)
                    {
                        dataGridViewEvents.CurrentCell = dgvc;
                        return;
                    }
                }
            }
            else
            {
                foreach (DataGridViewRow dgr in this.dataGridViewEvents.Rows)
                {
                    Event e = (Event)dgr.Tag;
                    if (e.Equals(efEvent))
                    {
                        dgr.Selected = true;
                        foreach (DataGridViewCell dgvc in dgr.Cells)
                        {
                            if (dgvc.Visible)
                            {
                                dataGridViewEvents.CurrentCell = dgvc;
                                return;
                            }
                        }
                    }
                }
            }
        }

        public void filter(Dictionary<string, bool> fdic)
        {
            this.dataGridViewEvents.SuspendLayout();
            foreach (DataGridViewColumn dgvc in this.dataGridViewEvents.Columns)
            {
                if (fdic.ContainsKey(dgvc.Name))
                {
                    dgvc.Visible = fdic[dgvc.Name];
                }
            }
            this.dataGridViewEvents.ResumeLayout();
            this.dataGridViewEvents.Invalidate();
        }

        private void dataGridView1_CellValidating(object sender, System.Windows.Forms.DataGridViewCellValidatingEventArgs e)
        {
            try
            {
                if (e.RowIndex > -1)
                {
                    if (dataGridViewEvents.Rows[e.RowIndex].IsNewRow)
                    {
                        return;
                    }
                    else if (dataGridViewEvents.Columns[e.ColumnIndex] is common.DataGridViewNumericUpDownColumn)
                       
                    {
                        if (dataGridViewEvents.Rows[e.RowIndex].Cells[e.ColumnIndex].Value != null &&
                            e.FormattedValue != null)
                        {
                            decimal dec = 0.0m;
                            decimal.TryParse(e.FormattedValue.ToString(), out dec);
                            dataGridViewEvents.Rows[e.RowIndex].Cells[e.ColumnIndex].Value = dec;
                        }

                    }

                    else if (dataGridViewEvents.Columns[e.ColumnIndex] is DataGridViewTextBoxColumn &&
                            e.FormattedValue != null)
                    {
                        string cellval = e.FormattedValue.ToString();
                        char firstChar = cellval.ToCharArray()[0];
                        bool isValidStr = true; //!char.IsNumber(firstChar);//artist uses digits for the start of var and game doesn't care!!!
                        string colName = dataGridViewEvents.Columns[e.ColumnIndex].HeaderText;

                        switch (firstChar)
                        {
                            case '~':
                            case '!':
                            case '@':
                            //case '#'://used by commented geo
                            case '$':
                            case '%':
                            case '^':
                            case '&':
                            case '*':
                            case '(':
                            case ')':
                            case '-':
                            case '+':
                            case '=':
                            case ',':
                            case '.':
                            case '<':
                            case '>':
                            case ';':
                                isValidStr = false;
                                break;
                        }

                        if (!isValidStr)
                        {
                            MessageBox.Show(string.Format("The value entered in \"{0}\" is not a valid string",colName));
                            e.Cancel = true;
                            dataGridViewEvents.Rows[e.RowIndex].ErrorText = "the value must not be a character-symbols";
                        }
                        else
                        {
                            e.Cancel = false;
                            dataGridViewEvents.Rows[e.RowIndex].ErrorText = null;
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                //MessageBox.Show(string.Format("row {0}, col {1}\r\nerror: {2}",e.RowIndex, e.ColumnIndex, ex.Message));
            }
        }

        private void dataGridViewEvents_CellEndEdit(object sender, System.Windows.Forms.DataGridViewCellEventArgs e)
        {
            try
            {
                if (e.RowIndex > -1)
                {
                    if (dataGridViewEvents.Rows[e.RowIndex].IsNewRow)
                    {
                        return;
                    }
                    if (dataGridViewEvents.SelectedCells.Count > 1 && 
                        dataGridViewEvents.SelectedCells[0].Value != null && ModifierKeys == Keys.Control)
                    {
                        int count = dataGridViewEvents.SelectedCells.Count;
                        int colInd = dataGridViewEvents.SelectedCells[0].ColumnIndex;
                        for (int i = 1; i < count; i++)
                        {
                            DataGridViewCell dgvc = dataGridViewEvents.SelectedCells[i];

                            if (dgvc.ColumnIndex == colInd)
                            {
                                dgvc.Value = dataGridViewEvents.SelectedCells[0].Value;
                            }
                        }
                    }

                }
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message);
            }
        }

        private void dataGridView1_EditingControlShowing(object sender, System.Windows.Forms.DataGridViewEditingControlShowingEventArgs e)
        {
            //I need to know that the combobox control selection has changed and let the 
            //data grid know of the changes event thought this control sitll under edit mode
            ComboBox combo = e.Control as ComboBox;
            TextBox tbx = e.Control as TextBox;

            if (combo != null)
            {          
                // Remove an existing event-handler, if present, to avoid 
                // adding multiple handlers when the editing control is reused.
                combo.SelectedIndexChanged -=
                    new EventHandler(ComboBox_SelectedIndexChanged);

                // Add the event handler. 
                combo.SelectedIndexChanged +=
                    new EventHandler(ComboBox_SelectedIndexChanged);

                string colName = dataGridViewEvents.Columns[dataGridViewEvents.CurrentCell.ColumnIndex].Name.ToLower();

                if (colName.Equals("at"))
                {
                    combo.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
                    combo.DropDownStyle = ComboBoxStyle.DropDown;
                    combo.Validating -= new CancelEventHandler(combo_Validating);
                    combo.Validating += new CancelEventHandler(combo_Validating);
                }
            }
            else if (tbx != null)
            {
                //tbx.KeyDown -= new KeyEventHandler(dataGridViewEvents_KeyDown);
                //tbx.KeyDown += new KeyEventHandler(dataGridViewEvents_KeyDown);
            }

            
        }

        private void combo_Validating(object sender, CancelEventArgs e)     
        {         
            DataGridViewComboBoxEditingControl combo = sender as DataGridViewComboBoxEditingControl;          
            DataGridView table = combo.EditingControlDataGridView; 
            
            // Add value to list if not there         
            if (combo != null && !combo.Items.Contains(combo.Text))         
            {             
                DataGridViewComboBoxColumn cboCol = table.Columns[table.CurrentCell.ColumnIndex] as DataGridViewComboBoxColumn;             
                // Must add to both the current combobox as well as the template, to avoid duplicate entries...             
                combo.Items.Add(combo.Text);             
                cboCol.Items.Add(combo.Text);             
                table.CurrentCell.Value = combo.Text;         
            }     
        }

        private void dataGridViewEvents_Scroll(object sender, System.Windows.Forms.ScrollEventArgs e)
        {
            this.dataGridViewEvents.Invalidate();
        }

        private string getChildFXfileName(string childFxName)
        {
            string path = null;

            FXWin fxwin = ((FXWin)this.Parent.Parent.Parent.Parent.Parent);

            string fileName = fxwin.FileName;

            if (childFxName.StartsWith(":"))
            {
                path = System.IO.Path.GetDirectoryName(fileName) + @"\" + childFxName.Substring(1);
            }
            else
            {
                path = fxwin.RootPath + @"fx\" + childFxName.Replace("/", @"\");
            }

            if (path != null && System.IO.File.Exists(path))
            {
                return path;
            }
            return path;
        }

        private void openChildFxMenuItem_Click(object sender, System.EventArgs e)
        {
            if (dgvContextMenu.Tag != null)
            {
                DataGridViewCell cell = dgvContextMenu.Tag as DataGridViewCell;

                if (cell != null &&
                    cell.Value != null)
                {
                    dgvContextMenu.Tag = null;

                    COH_CostumeUpdaterForm cForm = (COH_CostumeUpdaterForm)this.FindForm();

                    cuf = new COH_CostumeUpdaterForm();
                    
                    cuf.FormClosing += new FormClosingEventHandler(cuf_FormClosing);

                    COH_CostumeUpdaterForm cuform = (COH_CostumeUpdaterForm)this.FindForm();

                    cuf.tgaFilesDictionary = cuform.tgaFilesDictionary.ToDictionary(k => k.Key, k => k.Value);

                    string fxFileName = getChildFXfileName((string)cell.Value);

                    if (cForm.Owner != null)
                        cuf.Owner = cForm.Owner;
                    else
                        cuf.Owner = cForm;

                    cuf.loadFxFile(fxFileName);

                    cuf.showCfx(cuform.MenuStripColor);

                    cuf.fxLauncher.selectFxFile(fxFileName);
                }
            }
        }

        private void addChildFxMenuItem_Click(object sender, System.EventArgs e)
        {
            if (dgvContextMenu.Tag != null)
            {
                DataGridViewCell cell = dgvContextMenu.Tag as DataGridViewCell;

                dgvContextMenu.Tag = null; 
                
                OpenFileDialog ofd = new OpenFileDialog();

                FXWin fxwin = ((FXWin)this.Parent.Parent.Parent.Parent.Parent);

                string filePath = System.IO.Path.GetDirectoryName(fxwin.FileName);

                ofd.InitialDirectory = filePath;

                ofd.Filter = "Fx file (*.fx)|*.fx";

                DialogResult dr = ofd.ShowDialog();

                if (dr == DialogResult.OK)
                {
                    string fxFileRootPath = common.COH_IO.getRootPath(filePath);

                    string partRootPath = common.COH_IO.getRootPath(ofd.FileName);

                    if (partRootPath.ToLower().Equals(fxFileRootPath.ToLower()))
                    {
                        string path = ofd.FileName.Substring(partRootPath.Length + @"fx\".Length);

                        if (path.Trim().Length > 0)
                        {
                            if (ofd.FileName.StartsWith(filePath))
                            {
                                path = ":" + ofd.FileName.Substring((filePath + @"\").Length);
                            }

                            cell.Value = path;

                            dataGridViewEvents.RefreshEdit();

                            dataGridViewEvents.Columns[cell.ColumnIndex].AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;
                        }
                    }
                }
            }
        }

        private void addGeoMenuItem_Click(object sender, System.EventArgs e)
        {//DEF carpart_tag Transform
            
            if (dgvContextMenu.Tag != null)
            {
                DataGridViewCell cell = dgvContextMenu.Tag as DataGridViewCell;

                dgvContextMenu.Tag = null; 
                
                OpenFileDialog ofd = new OpenFileDialog();

                FXWin fxwin = ((FXWin)this.Parent.Parent.Parent.Parent.Parent);

                string rootPath = common.COH_IO.getRootPath(fxwin.FileName);

                if (rootPath != null)
                {
                    if (rootPath.Contains("data\\"))
                        rootPath = rootPath.Substring(0, rootPath.IndexOf("data\\"));
                }
                else
                {
                    rootPath = @"c:\game\";
                }

                ofd.InitialDirectory = rootPath + @"src";

                ofd.Filter = "WRL file (*.wrl)|*.wrl";

                DialogResult dr = ofd.ShowDialog();

                if (dr == DialogResult.OK)
                {
                    ArrayList wrlGeos = getGeosFromWrl(ofd.FileName);

                    if (wrlGeos.Count > 0)
                    {
                        string geomList = showGeomSelectionWindow(wrlGeos, ofd.FileName);
                        if (geomList.Length > 0)
                        {
                            cell.Value = cell.Value + geomList;

                            dataGridViewEvents.RefreshEdit();

                            dataGridViewEvents.Columns[cell.ColumnIndex].AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;
                        }
                    }
                    else
                    {
                        MessageBox.Show("Could not find any geo in the selected file");
                    }
                }
            }          
        }

        private string showGeomSelectionWindow(ArrayList geoms,string wrlFileName)
        {
            string geomlist = "";

            GeomSelectionForm gsf = new GeomSelectionForm();

            gsf.builGemoList(geoms);

            string subFileName = System.IO.Path.GetFileName(wrlFileName);
            
            gsf.Text = gsf.Text + ": " + (subFileName);

            DialogResult dr = gsf.ShowDialog();

            if (dr == DialogResult.OK)
            {
                geomlist = gsf.geomList;

                gsf.Dispose();
            }
            else
            {
                if (gsf != null)
                    gsf.Dispose();
            }

            gsf = null;

            return geomlist;
        }

        private ArrayList getGeosFromWrl(string wrlFile)
        {
            ArrayList geoms = new ArrayList();

            ArrayList data = new ArrayList();

            common.COH_IO.fillList(data, wrlFile);

            string geoToAdd = null;

            bool geoFound = false;

            foreach (string line in data)
            {
                string ln = line.Replace("\t", " ").Trim();

                if (ln.StartsWith("DEF") &&
                    ln.EndsWith("Transform {"))
                {
                    geoFound = true;
                    geoToAdd = ln.Replace("DEF ", "").Replace(" Transform {", "");
                }
                if (ln.StartsWith("texCoord DEF ") && geoFound && geoToAdd != null)
                {
                    int objTrickStart = geoToAdd.IndexOf("__");

                    if (objTrickStart > -1)
                        geoToAdd = geoToAdd.Substring(0, objTrickStart);

                    geoms.Add(geoToAdd);
                    geoFound = false;
                    geoToAdd = null;
                }
            }
            return geoms;
        }

        private void cuf_FormClosing(object sender, FormClosingEventArgs e)
        {
            cuf = null;
            ((Form)sender).Dispose();
        }

        private void ComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            //commit combobox edit in order to update relevent cells
            string comboboxText = ((ComboBox)sender).Text;
            dataGridViewEvents.CommitEdit(DataGridViewDataErrorContexts.CurrentCellChange);
            dataGridViewEvents.CurrentCell.Value = comboboxText;
            dataGridViewEvents.Invalidate();
        }

        private void dataGridViewEvents_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                DataGridView.HitTestInfo hit = dataGridViewEvents.HitTest(e.X, e.Y);

                if (hit.Type == DataGridViewHitTestType.Cell)
                {
                    DataGridViewCell cell = dataGridViewEvents.Rows[hit.RowIndex].Cells[hit.ColumnIndex] as DataGridViewCell;
                    if (dataGridViewEvents.Columns[hit.ColumnIndex].Name == "ChildFx" ||
                        dataGridViewEvents.Columns[hit.ColumnIndex].Name == "CEvent" ||
                        dataGridViewEvents.Columns[hit.ColumnIndex].Name == "Geom")
                    {
                        dataGridViewEvents.ContextMenu.Tag = cell;

                        this.openChildFxMenuItem.Enabled = false;
                        this.addChildFxMenuItem.Enabled = true;
                        this.addGeoMenuItem.Enabled = false;
                        
                        if (cell != null && cell.Value != null)
                            this.openChildFxMenuItem.Enabled = true;

                        if (dataGridViewEvents.Columns[hit.ColumnIndex].Name == "Geom")
                        {
                            this.addGeoMenuItem.Enabled = true;
                            this.addChildFxMenuItem.Enabled = false;
                            this.openChildFxMenuItem.Enabled = false;
                        }

                        dataGridViewEvents.ContextMenu.Show(dataGridViewEvents, e.Location);
                    }
                }
            }

        }
        
        //disable some cells based on the combobox "On" selection
        private void dataGridView1_CellValueChanged(object sender, System.Windows.Forms.DataGridViewCellEventArgs e)
        {
            if (e.RowIndex > -1)
            {
                if (!settingCells)
                {
                    Event ev = (Event)dataGridViewEvents.Rows[e.RowIndex].Tag;

                    isDirty = true;

                    if (!ev.isDirty)
                    {
                        ev.isDirty = true;
                        ev.parent.isDirty = true;
                        ev.parent.parent.isDirty = true;

                        if (IsDirtyChanged != null)
                            IsDirtyChanged(this, e);
                    }
                }

                if (!cellIsChanging)
                {
                    Color cColor = dataGridViewEvents.Rows[e.RowIndex].InheritedStyle.BackColor;

                    Color dgcForeColor = dataGridViewEvents.Rows[e.RowIndex].Cells[e.ColumnIndex].Style.ForeColor;

                    if (dataGridViewEvents.Rows[e.RowIndex].Cells[e.ColumnIndex].Value != null && dgcForeColor != Color.Red)
                    {
                        string ename = (string) dataGridViewEvents.Rows[e.RowIndex].Cells["EName"].Value;
                        Event ev = (Event)dataGridViewEvents.Rows[e.RowIndex].Tag;
                        string evEname = (string)ev.eParameters["EName"];
                        if (evEname == null)
                            evEname = "@";

                        if (ename != null &&
                            !ename.ToLower().Equals(evEname.ToLower()))
                        {
                            ev.eParameters["EName"] = ename;

                            if (ENameChanged != null)
                                ENameChanged(ev, e);
                        }

                        dataGridViewEvents.Rows[e.RowIndex].Cells[e.ColumnIndex].Style.BackColor = Color.FromArgb(cColor.R + 50,
                            cColor.G, cColor.B + 50);

                        this.dataGridViewEvents.Invalidate();
                    }
                }
            }
        }
    }


    //derived class to change Enter(return) key behavior
    public class COH_DataGridView : DataGridView
    {
        [System.Security.Permissions.UIPermission(
        System.Security.Permissions.SecurityAction.LinkDemand,
        Window = System.Security.Permissions.UIPermissionWindow.AllWindows)]
        protected override bool ProcessDialogKey(Keys keyData)
        {
            DataGridViewCell dgc = this.CurrentCell;

            bool returnVal = base.ProcessDialogKey(keyData);

            // Extract the key code from the key value. 
            Keys key = (keyData & Keys.KeyCode);
            // Extract the Modifires from the key value. 
            Keys modifires = (keyData & Keys.Modifiers);

            //this for Andras so not to accidentaly over right a valu when using "ctl s"
            if (modifires == Keys.Control && key == Keys.S)
            {
                return true;
            }

            if (key == Keys.Delete)
            {
                this.EndEdit( DataGridViewDataErrorContexts.Commit);
                this.RefreshEdit();
                this.CurrentCell.DetachEditingControl();
                this.CurrentCell.Value = null;
                this.CurrentCell.Style.BackColor = this.Rows[dgc.RowIndex].InheritedStyle.BackColor;
            }
         
            // Handle the ENTER key as if it were a RIGHT ARROW key. 
            if (key == Keys.Enter)
            {
                this.CurrentCell = dgc;
            }
            
            return returnVal;
        }
    }
}
