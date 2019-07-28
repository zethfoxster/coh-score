using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    public partial class ConditionWin : Panel
        //Form
    {
        public event EventHandler CurrentRowChanged;
        
        public event EventHandler IsDirtyChanged;

        private Dictionary<string, string> htmlDic;

        private common.FormatedToolTip dgvTooltip;
        
        public bool isDirty;

        private bool settingCells;
        
        private bool cellIsChanging;

        private Condition currentRowCondition;

        public ConditionWin()
        {
            cellIsChanging = false;
            InitializeComponent();
            initializeToolTip();
        }

        private void initializeToolTip()
        {
            dgvTooltip = new common.FormatedToolTip();

            dgvTooltip.AutoPopDelay = 20000;

            dgvTooltip.InitialDelay = 0;

            dgvTooltip.ReshowDelay = 0;

            dgvTooltip.ShowAlways = true;

            dgvTooltip.StripAmpersands = false;

            htmlDic = common.COH_htmlToolTips.getToolTipsDic(@"assetEditor/objectTrick/FxToolTips.html");
        }

        public Condition CurrentRowCondition
        {
            get
            {
                return currentRowCondition;
            }
            set
            {
                currentRowCondition = CurrentRowCondition;
            }
        }

        private void OnCurrentRowChanged(EventArgs e)
        {
            if (CurrentRowChanged != null)
                CurrentRowChanged(this, e);
        }

        private void dataGridView_CurrentCellChanged(object sender, System.EventArgs e)
        {
            if (dataGridView1.CurrentCell != null)
            {
                Condition cnd = currentRowCondition;
                
                currentRowCondition = (Condition)dataGridView1.Rows[dataGridView1.CurrentCell.RowIndex].Tag;
                
                if (currentRowCondition != null && currentRowCondition != cnd)
                {
                    this.Invoke(CurrentRowChanged);
                }
            }
        }
        
        public void setConditionsData(FX fx)
        {
            settingCells = true;

            if(this.dataGridView1.Rows.Count > 0)
                this.dataGridView1.Rows.Clear();

            foreach (Condition condition in fx.conditions)
            {
                setCondition(condition);
            }

            settingCells = false;
        }
        
        public void selectConditionRow(Condition efcondition)
        {
            this.dataGridView1.ClearSelection();

            if (efcondition.mCondWinDGIndx != -1 &&
                efcondition.mCondWinDGIndx < this.dataGridView1.Rows.Count)
            {
                DataGridViewRow dgr = this.dataGridView1.Rows[efcondition.mCondWinDGIndx];
                dgr.Selected = true;
                foreach (DataGridViewCell dgvc in dgr.Cells)
                {
                    if (dgvc.Visible)
                    {
                        dataGridView1.CurrentCell = dgvc;
                        return;
                    }
                }
            }
            else
            {
                foreach (DataGridViewRow dgr in this.dataGridView1.Rows)
                {
                    Condition e = (Condition)dgr.Tag;
                    if (e.Equals(efcondition))
                    {
                        dgr.Selected = true;
                        foreach (DataGridViewCell dgvc in dgr.Cells)
                        {
                            if (dgvc.Visible)
                            {
                                dataGridView1.CurrentCell = dgvc;
                                return;
                            }
                        }
                        return;
                    }
                }
            }
        }
        
        public void setCondition(Condition condition)
        {
            DataGridViewCellStyle dgcStyle = dataGridView1.ColumnHeadersDefaultCellStyle;

            int ind = dataGridView1.Rows.Add();

            condition.mCondWinDGIndx = ind;

            dataGridView1.Rows[ind].Tag = condition;

            Dictionary<string, object> condParam = condition.cParameters.ToDictionary(k => k.Key, k => k.Value);

            foreach (KeyValuePair<string, object> kvp in condParam)
            {
                if (kvp.Value != null)
                {
                    DataGridViewCell dgc = dataGridView1.Rows[ind].Cells[kvp.Key];
                    
                    string kvpValStr = ((string) kvp.Value).Trim();

                    bool is_Commented = false;

                    if (kvpValStr.StartsWith("#"))
                    {
                        dgc.Style.ForeColor = Color.Red;
                        dgc.Style.Font = new Font(dgcStyle.Font, FontStyle.Bold | FontStyle.Italic);
                        kvpValStr = kvpValStr.Trim().Substring(1);
                        is_Commented = true;
                    }
                    else
                    {
                        dgc.Style.ForeColor = dgcStyle.ForeColor;
                        dgc.Style.Font = new Font(dgcStyle.Font, FontStyle.Regular);                        
                    }

                    switch (dgc.GetType().Name)
                    {
                        case "DataGridViewNumericUpDownCell":
                            decimal dec;
                            if (decimal.TryParse(kvpValStr, out dec))
                                dgc.Value = dec;
                            break;

                        case "DataGridViewTextBoxCell":
                        case "DataGridViewComboBoxCell":
                            dgc.Value = kvpValStr;
                            break;

                        case "COH_DataGridViewCheckBoxCell":
                        case "DataGridViewCheckBoxCell":
                            dgc.Value = kvpValStr.Equals("0") ? false : true;
                            break;
                    }
                    dataGridView1.Rows[ind].Cells[dgc.ColumnIndex].Tag = kvp.Value;
                    setCellColor(kvp.Key, ind, is_Commented);
                }
            }

            dataGridView1.AllowUserToAddRows = false;
            dataGridView1.AllowUserToDeleteRows = false;
        }

        public void updateFXConditionsData()
        {
            foreach (DataGridViewRow dgr in this.dataGridView1.Rows)
            {
                getConditionRowData(dgr);
            }
        }

        private void setCellColor(string colName, int rowInd, bool isCommented)
        {
            DataGridViewCellStyle dgcStyle = dataGridView1.ColumnHeadersDefaultCellStyle;

            if (isCommented)
            {
                dataGridView1.Rows[rowInd].Cells[colName].Style.ForeColor = Color.Red;
                dataGridView1.Rows[rowInd].Cells[colName].Style.Font = new Font(dgcStyle.Font, FontStyle.Bold | FontStyle.Italic);
                if (dataGridView1.Rows[rowInd].Cells[colName].GetType().Name.Equals("DataGridViewCheckBoxCell"))
                {
                    DataGridViewCheckBoxCell dgck = (DataGridViewCheckBoxCell)dataGridView1.Rows[rowInd].Cells[colName];
                    dgck.Style.ForeColor = Color.Red;
                }
                else if (dataGridView1.Rows[rowInd].Cells[colName].GetType().Name.Equals("COH_DataGridViewCheckBoxCell"))
                {
                    common.COH_DataGridViewCheckBoxCell dgck = (common.COH_DataGridViewCheckBoxCell)dataGridView1.Rows[rowInd].Cells[colName];
                    dgck.Style.ForeColor = Color.Red;
                }
            }
            else
            {
                dataGridView1.Rows[rowInd].Cells[colName].Style.ForeColor = dgcStyle.ForeColor;
                dataGridView1.Rows[rowInd].Cells[colName].Style.Font = new Font(dgcStyle.Font, FontStyle.Regular);
            }
        }

        private void clearConditionParameters(ref Condition cond)
        {
            string [] keys = cond.cParameters.Keys.ToArray<string>();

            foreach( string k in keys )
            {
                cond.cParameters[k] = null;
            }            
        }

        private void getConditionRowData(DataGridViewRow dgr)
        {
            if (dgr.Tag != null)
            {
                Condition conditon = (Condition)dgr.Tag;

                clearConditionParameters(ref conditon);

                foreach (DataGridViewCell dgc in dgr.Cells)
                {
                    bool isCommented = dgc.Style.ForeColor == Color.Red;

                    string colName = this.dataGridView1.Columns[dgc.ColumnIndex].HeaderText;

                    //dgc.Tag

                    if (conditon.cParameters.ContainsKey(colName) && dgc.Value!= null && !dgc.ReadOnly)
                    {
                        string valueStr = null;

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

                        conditon.cParameters[colName] = isCommented ? ("#" + valueStr) : valueStr;
                    }
                }
            } 
        }

        private void dataGridView1_CellMouseEnter(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex > -1 && e.ColumnIndex > -1 && ModifierKeys == Keys.Control)
            {
                DataGridViewRow dgvr = dataGridView1.Rows[e.RowIndex];

                string colName = dataGridView1.Columns[e.ColumnIndex].Name;

                DataGridViewCell dgvc = dgvr.Cells[e.ColumnIndex];
                if (true)
                {
                    string toolTipText = common.COH_htmlToolTips.getText(colName, htmlDic);

                    if (toolTipText.Trim().Length > 0)
                    {
                        Point p = this.PointToClient(Cursor.Position);

                        dgvTooltip.Show(toolTipText, this, p.X + 40, p.Y + 20, dgvTooltip.AutoPopDelay);
                    }
                }
            }
        }

        private void dataGridView1_CellMouseLeave(object sender, DataGridViewCellEventArgs e)
        {
            dgvTooltip.Hide(this);
            dataGridView1.EndEdit();
        }

        private void dataGridView1_CellValidating(object sender, System.Windows.Forms.DataGridViewCellValidatingEventArgs e)
        {
            try
            {
                if (e.RowIndex > -1)
                {
                    if (dataGridView1.Rows[e.RowIndex].IsNewRow)
                    {
                        return;
                    }

                    /*else if (dataGridView1.Columns[e.ColumnIndex] is common.DataGridViewNumericUpDownColumn &&
                        e.FormattedValue != null)
                    {
                        decimal dec = 0.0m;
                        decimal.TryParse(e.FormattedValue.ToString(), out dec);
                        dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value = dec;
                    }*/

                    else if (dataGridView1.Columns[e.ColumnIndex] is DataGridViewTextBoxColumn &&
                            e.FormattedValue != null)
                    {
                        string cellval = e.FormattedValue.ToString();
                        char firstChar = cellval.ToCharArray()[0];
                        bool isValidStr = true;//!char.IsNumber(firstChar); //artist uses digits for the start of var and game doesn't care!!!

                        switch (firstChar)
                        {
                            case '~':
                            case '!':
                            case '@':
                            case '#':
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
                            //case ';':
                                isValidStr = false;
                                break;
                        }

                        if (!isValidStr)
                        {
                            MessageBox.Show("The value entered in \"Triggerbits\" is not a valid string");
                            e.Cancel = true;
                            dataGridView1.Rows[e.RowIndex].ErrorText = "the value must be a non-negative integer";
                        }
                        else
                        {
                            e.Cancel = false;
                            dataGridView1.Rows[e.RowIndex].ErrorText = null;
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                //MessageBox.Show(string.Format("row {0}, col {1}\r\nerror: {2}",e.RowIndex, e.ColumnIndex, ex.Message));
            }
        }

        private void dataGridView1_EditingControlShowing(object sender, System.Windows.Forms.DataGridViewEditingControlShowingEventArgs e)
        {
            //I need to know that the combobox control selection has changed and let the 
            //data grid know of the changes event thought this control sitll under edit mode
            ComboBox combo = e.Control as ComboBox;
            if (combo != null)
            {
                // Remove an existing event-handler, if present, to avoid 
                // adding multiple handlers when the editing control is reused.
                combo.SelectedIndexChanged -=
                    new EventHandler(ComboBox_SelectedIndexChanged);

                // Add the event handler. 
                combo.SelectedIndexChanged +=
                    new EventHandler(ComboBox_SelectedIndexChanged);            }

            
        }

        private void ComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            //commit combobox edit in order to update relevent cells
            dataGridView1.CommitEdit(DataGridViewDataErrorContexts.CurrentCellChange);
            dataGridView1.Invalidate();
        }

        //disable some cells based on the combobox "On" selection
        void dataGridView1_CellValueChanged(object sender, System.Windows.Forms.DataGridViewCellEventArgs e)
        {
            if (e.RowIndex > -1)
            {
                if (!settingCells)
                {
                    Condition cond = (Condition)dataGridView1.Rows[e.RowIndex].Tag;

                    isDirty = true;

                    if (!cond.isDirty)
                    {
                        cond.isDirty = true;
                        cond.parent.isDirty = true;

                        if (IsDirtyChanged != null)
                            IsDirtyChanged(this, e);
                    }
                }

                if (!cellIsChanging)
                {
                    Color cColor = dataGridView1.Rows[e.RowIndex].InheritedStyle.BackColor;

                    Color dgcForeColor = dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Style.ForeColor;

                    if (dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value != null && dgcForeColor != Color.Red)
                    {
                        dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Style.BackColor = Color.FromArgb(cColor.R + 50,
                            cColor.G, cColor.B + 50);
                        this.dataGridView1.Invalidate();
                    }

                    if (!dataGridView1.Rows[e.RowIndex].IsNewRow &&
                        dataGridView1.Rows[e.RowIndex].Cells["On"].Value != null)
                    {
                        cellIsChanging = true; ;
                        string onVal = (string)dataGridView1.Rows[e.RowIndex].Cells["On"].Value;

                        dataGridView1.Rows[e.RowIndex].Cells["DayStart"].ReadOnly = true;
                        dataGridView1.Rows[e.RowIndex].Cells["DayEnd"].ReadOnly = true;
                        dataGridView1.Rows[e.RowIndex].Cells["ForceThreshold"].ReadOnly = true;
                        dataGridView1.Rows[e.RowIndex].Cells["Time"].ReadOnly = true;

                        ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["DayStart"]).Enabled = false;
                        ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["DayEnd"]).Enabled = false;
                        ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["ForceThreshold"]).Enabled = false;
                        ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["Time"]).Enabled = false;


                        if (onVal.Equals("TimeOfDay"))
                        {
                            dataGridView1.Rows[e.RowIndex].Cells["DayStart"].ReadOnly = false;
                            dataGridView1.Rows[e.RowIndex].Cells["DayEnd"].ReadOnly = false;
                            ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["DayStart"]).Enabled = true;
                            ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["DayEnd"]).Enabled = true;

                        }
                        else if (onVal.Equals("Force"))
                        {
                            dataGridView1.Rows[e.RowIndex].Cells["ForceThreshold"].ReadOnly = false;
                            dataGridView1.Rows[e.RowIndex].Cells["Time"].ReadOnly = false;
                            ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["ForceThreshold"]).Enabled = true;
                            ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["Time"]).Enabled = true;
                        }
                        else if (onVal.Equals("Time") || onVal.Equals("Cycle"))
                        {
                            dataGridView1.Rows[e.RowIndex].Cells["Time"].ReadOnly = false;
                            ((common.DataGridViewNumericUpDownCell)dataGridView1.Rows[e.RowIndex].Cells["Time"]).Enabled = true;
                        }
                        dataGridView1.Invalidate();
                        cellIsChanging = false;
                    }
                }
            }
        }
    }
}
