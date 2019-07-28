namespace COH_CostumeUpdater.fxEditor
{
    partial class ConditionWin
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle4 = new System.Windows.Forms.DataGridViewCellStyle();
            this.dataGridView1 = new COH_DataGridView();// System.Windows.Forms.DataGridView();
            this.Random = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.DoMany = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Repeat = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.RepeatJitter = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.Chance = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.On = new System.Windows.Forms.DataGridViewComboBoxColumn();
            this.ForceThreshold = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.Time = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.DayStart = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.DayEnd = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.TriggerBits = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            this.SuspendLayout();
            // 
            // dataGridView1
            // 
            this.dataGridView1.AllowUserToDeleteRows = false;
            dataGridViewCellStyle1.BackColor = System.Drawing.Color.FromArgb(System.Drawing.SystemColors.ControlLight.R - 50,
                System.Drawing.SystemColors.ControlLight.G, System.Drawing.SystemColors.ControlLight.B - 50);

            this.dataGridView1.AlternatingRowsDefaultCellStyle = dataGridViewCellStyle1;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle2.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle2.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridView1.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle2;
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.MultiSelect = true;
            this.dataGridView1.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Random,
            this.DoMany,
            this.Repeat,
            this.RepeatJitter,
            this.Chance,
            this.On,
            this.ForceThreshold,
            this.Time,
            this.DayStart,
            this.DayEnd,
            this.TriggerBits});
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle3.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle3.ForeColor = System.Drawing.SystemColors.ControlText;
            dataGridViewCellStyle3.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle3.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle3.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridView1.DefaultCellStyle = dataGridViewCellStyle3;
            this.dataGridView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dataGridView1.EditMode = System.Windows.Forms.DataGridViewEditMode.EditOnEnter;
            this.dataGridView1.Location = new System.Drawing.Point(0, 0);
            this.dataGridView1.Name = "dataGridView1";
            dataGridViewCellStyle4.BackColor = System.Drawing.Color.FromArgb(System.Drawing.SystemColors.Control.R - 50,
                System.Drawing.SystemColors.Control.G, System.Drawing.SystemColors.Control.B - 50);

            this.dataGridView1.RowsDefaultCellStyle = dataGridViewCellStyle4;
            this.dataGridView1.Size = new System.Drawing.Size(839, 381);
            this.dataGridView1.TabIndex = 0;
            this.dataGridView1.CurrentCellChanged += new System.EventHandler(dataGridView_CurrentCellChanged);
            this.dataGridView1.CellValueChanged += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridView1_CellValueChanged);
            this.dataGridView1.CellValidating += new System.Windows.Forms.DataGridViewCellValidatingEventHandler(this.dataGridView1_CellValidating);
            this.dataGridView1.EditingControlShowing += new System.Windows.Forms.DataGridViewEditingControlShowingEventHandler(this.dataGridView1_EditingControlShowing);
            this.dataGridView1.CellMouseEnter += new System.Windows.Forms.DataGridViewCellEventHandler(dataGridView1_CellMouseEnter);
            this.dataGridView1.CellMouseLeave += new System.Windows.Forms.DataGridViewCellEventHandler(dataGridView1_CellMouseLeave);
            // 
            // Random
            // 
            this.Random.HeaderText = "Random";
            this.Random.Name = "Random";
            this.Random.Width = 50;
            // 
            // DoMany
            // 
            this.DoMany.HeaderText = "DoMany";
            this.DoMany.Name = "DoMany";
            this.DoMany.Width = 50;
            // 
            // Repeat
            // 
            this.Repeat.HeaderText = "Repeat";
            this.Repeat.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.Repeat.Name = "Repeat";
            this.Repeat.Width = 80;
            // 
            // RepeatJitter
            // 
            this.RepeatJitter.HeaderText = "RepeatJitter";
            this.RepeatJitter.Maximum = new decimal(new int[] {
            256,
            0,
            0,
            0});
            this.RepeatJitter.Name = "RepeatJitter";
            this.RepeatJitter.Width = 80;
            // 
            // Chance
            // 
            this.Chance.DecimalPlaces = 6;
            this.Chance.HeaderText = "Chance";
            this.Chance.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.Chance.Increment = 0.001m;
            this.Chance.Name = "Chance";
            this.Chance.Width = 80;
            // 
            // On
            // 
            this.On.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.On.HeaderText = "On";
            this.On.Items.AddRange(new object[] {
            "Cycle",
            "Death",
            "Force",
            "Time",
            "TimeOfDay",
            "TriggerBits",
            "PrimeHit",
            "Prime1Hit",
            "Prime2Hit",
            "Prime3Hit",
            "Prime4Hit",
            "Prime5Hit",
            "PrimeBeamHit"});
            this.On.Name = "On";
            // 
            // ForceThreshold
            // 
            this.ForceThreshold.HeaderText = "ForceThreshold";
            this.ForceThreshold.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.ForceThreshold.Name = "ForceThreshold";
            this.ForceThreshold.Width = 80;
            // 
            // Time
            // 
            this.Time.HeaderText = "Time";
            this.Time.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.Time.Name = "Time";
            this.Time.Width = 80;
            // 
            // DayStart
            // 
            this.DayStart.DecimalPlaces = 6;
            this.DayStart.HeaderText = "DayStart";
            this.DayStart.Maximum = new decimal(new int[] {
            24,
            0,
            0,
            0});
            this.DayStart.Name = "DayStart";
            this.DayStart.Width = 80;
            // 
            // DayEnd
            // 
            this.DayEnd.DecimalPlaces = 6;
            this.DayEnd.HeaderText = "DayEnd";
            this.DayEnd.Maximum = new decimal(new int[] {
            24,
            0,
            0,
            0});
            this.DayEnd.Name = "DayEnd";
            this.DayEnd.Width = 80;
            // 
            // TriggerBits
            // 
            this.TriggerBits.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
            this.TriggerBits.HeaderText = "TriggerBits";
            this.TriggerBits.Name = "TriggerBits";
            this.TriggerBits.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // ConditionWin
            // 
            this.ClientSize = new System.Drawing.Size(839, 381);
            this.Controls.Add(this.dataGridView1);
            this.Name = "ConditionWin";
            this.Text = "FXWin";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.ResumeLayout(false);

        }



        #endregion

        //private System.Windows.Forms.DataGridView dataGridView1;
        private COH_DataGridView dataGridView1;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn Random;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn DoMany;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn Repeat;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn RepeatJitter;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn Chance;
        private System.Windows.Forms.DataGridViewComboBoxColumn On;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn ForceThreshold;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn Time;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn DayStart;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn DayEnd;
        private System.Windows.Forms.DataGridViewTextBoxColumn TriggerBits;
    }
}