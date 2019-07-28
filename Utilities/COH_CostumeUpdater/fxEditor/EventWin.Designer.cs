namespace COH_CostumeUpdater.fxEditor
{
    partial class EventWin
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
            this.dataGridViewEvents = new COH_DataGridView();// System.Windows.Forms.DataGridView();
            this.EName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.At = new System.Windows.Forms.DataGridViewComboBoxColumn();

            this.Splat = new System.Windows.Forms.DataGridViewImageColumn();

            this.CRotation = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.CameraSpace = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.HardwareOnly = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.SoftwareOnly = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.PhysicsOnly = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();

            this.Inherit_Rotation = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Inherit_Position = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Inherit_Scale = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Inherit_SuperRotation = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();

            this.Update_Rotation = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Update_Position = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Update_Scale = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Update_SuperRotation = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();

            this.AtRayFx = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Geom = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.AnimPiv = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SetState = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Magnet = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.LookAt = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.PMagnet = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.POther = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.While = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Until = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.WorldGroup = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.RayLength = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.AltPiv = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.SoundNoRepeat = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.LifeSpan = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.LifeSpanJitter = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.Type = new System.Windows.Forms.DataGridViewComboBoxColumn();
            this.CEvent   = new System.Windows.Forms.DataGridViewTextBoxColumn() ;
            this.CThresh  = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.CDestroy = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.JEvent   = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.JDestroy = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Flags = new System.Windows.Forms.DataGridViewTextBoxColumn();
            //this.OneAtATime= new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            //this.AdoptParentEntity = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Lighting  = new COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn();
            this.Cape    = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Anim    = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.NewState = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.ChildFx = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.PowerMin = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.PowerMax = new COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn();
            this.dgvContextMenu = new System.Windows.Forms.ContextMenu();
            this.openChildFxMenuItem = new System.Windows.Forms.MenuItem();
            this.addChildFxMenuItem = new System.Windows.Forms.MenuItem();
            this.addGeoMenuItem = new System.Windows.Forms.MenuItem();



            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewEvents)).BeginInit();
            this.SuspendLayout();
            // 
            // dataGridViewEvents
            // 
            this.dataGridViewEvents.AllowUserToDeleteRows = false;
            this.dataGridViewEvents.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.EName,
            this.At,
            this.CRotation,
            this.CameraSpace,
            this.HardwareOnly,
            this.SoftwareOnly,
            this.PhysicsOnly,
            this.Flags,
            //this.OneAtATime,
            //this.AdoptParentEntity,
            this.Lighting,
            this.Type,
            this.Inherit_Position,
            this.Inherit_Rotation,
            this.Inherit_Scale,
            this.Inherit_SuperRotation,
            this.Update_Position,
            this.Update_Rotation,
            this.Update_Scale,
            this.Update_SuperRotation, 
            this.CEvent,  
            this.CThresh, 
            this.CDestroy,
            this.JEvent,  
            this.JDestroy,
            this.AtRayFx,
            this.Geom,
            this.AnimPiv,
            this.Anim,
            this.NewState,
            this.SetState,
            this.Magnet,
            this.LookAt,
            this.PMagnet,
            this.POther,
            this.PowerMin,
            this.PowerMax,
            this.While,
            this.Until,
            this.WorldGroup,
            this.RayLength,
            this.AltPiv,
            this.SoundNoRepeat,
            this.LifeSpan,
            this.LifeSpanJitter,
            this.Cape,
            this.ChildFx});
            dataGridViewCellStyle1.BackColor = System.Drawing.Color.FromArgb(System.Drawing.SystemColors.ControlLight.R-50,
                System.Drawing.SystemColors.ControlLight.G,System.Drawing.SystemColors.ControlLight.B-50);

            this.dataGridViewEvents.AlternatingRowsDefaultCellStyle = dataGridViewCellStyle1;
            //this.dataGridViewEvents.RowTemplate.Height = 64;
            this.dataGridViewEvents.MultiSelect = true;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle2.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle2.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewEvents.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle2;
            this.dataGridViewEvents.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle3.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle3.ForeColor = System.Drawing.SystemColors.ControlText;
            dataGridViewCellStyle3.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle3.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle3.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewEvents.DefaultCellStyle = dataGridViewCellStyle3;
            this.dataGridViewEvents.ClipboardCopyMode = System.Windows.Forms.DataGridViewClipboardCopyMode.EnableWithoutHeaderText;
            this.dataGridViewEvents.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dataGridViewEvents.EditMode = System.Windows.Forms.DataGridViewEditMode.EditOnEnter;
            this.dataGridViewEvents.Location = new System.Drawing.Point(0, 0);
            this.dataGridViewEvents.Name = "dataGridViewEvents";
            dataGridViewCellStyle4.BackColor = System.Drawing.Color.FromArgb(System.Drawing.SystemColors.Control.R - 50,
                System.Drawing.SystemColors.Control.G, System.Drawing.SystemColors.Control.B - 50);
            this.dataGridViewEvents.RowsDefaultCellStyle = dataGridViewCellStyle4;
            this.dataGridViewEvents.Size = new System.Drawing.Size(839, 381);
            this.dataGridViewEvents.TabIndex = 0;
            this.dataGridViewEvents.CurrentCellChanged += new System.EventHandler(dataGridView_CurrentCellChanged);
            this.dataGridViewEvents.CellMouseEnter += new System.Windows.Forms.DataGridViewCellEventHandler(dataGridViewEvents_CellMouseEnter);
            this.dataGridViewEvents.CellMouseLeave += new System.Windows.Forms.DataGridViewCellEventHandler(dataGridViewEvents_CellMouseLeave);
            this.dataGridViewEvents.CellValueChanged += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridView1_CellValueChanged);
            this.dataGridViewEvents.CellValidating += new System.Windows.Forms.DataGridViewCellValidatingEventHandler(this.dataGridView1_CellValidating);
            this.dataGridViewEvents.EditingControlShowing += new System.Windows.Forms.DataGridViewEditingControlShowingEventHandler(this.dataGridView1_EditingControlShowing);
            this.dataGridViewEvents.CellEndEdit += new System.Windows.Forms.DataGridViewCellEventHandler(dataGridViewEvents_CellEndEdit);
            //this.dataGridViewEvents.Paint += new System.Windows.Forms.PaintEventHandler(dataGridViewEvents_Paint);
            this.dataGridViewEvents.ContextMenu = this.dgvContextMenu;
            this.dataGridViewEvents.MouseDown += new System.Windows.Forms.MouseEventHandler(dataGridViewEvents_MouseDown);
            //this.dataGridViewEvents.Scroll += new System.Windows.Forms.ScrollEventHandler(dataGridViewEvents_Scroll);
            //
            // dgvContextMenue
            this.dgvContextMenu.Name = "dgvContextMenu";
            this.dgvContextMenu.MenuItems.Add(this.openChildFxMenuItem);
            this.dgvContextMenu.MenuItems.Add(this.addChildFxMenuItem);
            this.dgvContextMenu.MenuItems.Add(this.addGeoMenuItem);
            //
            // openChildFxMenuItem
            //
            this.openChildFxMenuItem.Name = "openChildFxMenuItem";
            this.openChildFxMenuItem.Text = "Open ChildFX/CEvent";
            this.openChildFxMenuItem.Click += new System.EventHandler(openChildFxMenuItem_Click);
            //
            // addChildFxMenuItem
            //
            this.addChildFxMenuItem.Name = "addChildFxMenuItem";
            this.addChildFxMenuItem.Text = "Add ChildFX/CEvent";
            this.addChildFxMenuItem.Click += new System.EventHandler(addChildFxMenuItem_Click);
            //
            // addGeoFxMenuItem
            //
            this.addGeoMenuItem.Name = "addChildFxMenuItem";
            this.addGeoMenuItem.Text = "Add Geo";
            this.addGeoMenuItem.Click += new System.EventHandler(addGeoMenuItem_Click);
            // 
            // EName
            // 
            this.EName.HeaderText = "EName";
            this.EName.Name = "EName";
            this.EName.Width = 80;
            this.EName.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // At
            // 
            this.At.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.At.HeaderText = "At";
            this.At.Name = "At";
            this.At.Width = 80;
            this.At.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // Splat
            //
            this.Splat.HeaderText = "Splat";
            this.Splat.Name = "Splat";
            this.Splat.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.Splat.Width = 64;
            // 
            // CRotation
            // 
            this.CRotation.HeaderText = "CRotation";
            this.CRotation.Name = "CRotation";
            this.CRotation.Width = 60;
            this.CRotation.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // CameraSpace
            // 
            this.CameraSpace.HeaderText = "Camera Space";
            this.CameraSpace.Name = "CameraSpace";
            this.CameraSpace.Width = 60;
            this.CameraSpace.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // HardwareOnly
            // 
            this.HardwareOnly.HeaderText = "Hardware Only";
            this.HardwareOnly.Name = "HardwareOnly";
            this.HardwareOnly.Width = 60;
            this.HardwareOnly.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // SoftwareOnly
            // 
            this.SoftwareOnly.HeaderText = "Software Only";
            this.SoftwareOnly.Name = "SoftwareOnly";
            this.SoftwareOnly.Width = 50;
            this.SoftwareOnly.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // PhysicsOnly
            // 
            this.PhysicsOnly.HeaderText = "Physics Only";
            this.PhysicsOnly.Name = "PhysicsOnly";
            this.PhysicsOnly.Width = 50;
            this.PhysicsOnly.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // AtRayFx
            // 
            this.AtRayFx.HeaderText = "AtRayFx";
            this.AtRayFx.Name = "AtRayFx";
            this.AtRayFx.Width = 80;
            this.AtRayFx.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Geom
            // 
            this.Geom.HeaderText = "Geom";
            this.Geom.Name = "Geom";
            this.Geom.Width = 80;
            this.Geom.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // AnimPiv
            // 
            this.AnimPiv.HeaderText = "AnimPiv";
            this.AnimPiv.Name = "AnimPiv";
            this.AnimPiv.Width = 80;
            this.AnimPiv.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // SetState
            // 
            this.SetState.HeaderText = "SetState";
            this.SetState.Name = "SetState";
            this.SetState.Width = 80;
            this.SetState.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Magnet
            // 
            this.Magnet.HeaderText = "Magnet";
            this.Magnet.Name = "Magnet";
            this.Magnet.Width = 80;
            this.Magnet.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // LookAt
            // 
            this.LookAt.HeaderText = "LookAt";
            this.LookAt.Name = "LookAt";
            this.LookAt.Width = 80;
            this.LookAt.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // PMagnet
            // 
            this.PMagnet.HeaderText = "PMagnet";
            this.PMagnet.Name = "PMagnet";
            this.PMagnet.Width = 80;
            this.PMagnet.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // POther
            // 
            this.POther.HeaderText = "POther";
            this.POther.Name = "POther";
            this.POther.Width = 80;
            this.POther.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // While
            // 
            this.While.HeaderText = "While";
            this.While.Name = "While";
            this.While.Width = 80;
            this.While.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Until
            // 
            this.Until.HeaderText = "Until";
            this.Until.Name = "Until";
            this.Until.Width = 80;
            this.Until.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // WorldGroup
            // 
            this.WorldGroup.HeaderText = "World Group";
            this.WorldGroup.Name = "WorldGroup";
            this.WorldGroup.Width = 80;
            this.WorldGroup.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // RayLength
            // 
            this.RayLength.DecimalPlaces = 6;
            this.RayLength.Maximum = 1000.0m;
            this.RayLength.Minimum = 0.0m;
            this.RayLength.HeaderText = "Ray Length";
            this.RayLength.Name = "RayLength";
            this.RayLength.Width = 80;
            this.RayLength.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // AltPiv
            // 
            this.AltPiv.DecimalPlaces = 0;
            this.AltPiv.Maximum = 1000.0m;
            this.AltPiv.Minimum = 0.0m;
            this.AltPiv.HeaderText = "AltPiv";
            this.AltPiv.Name = "AltPiv";
            this.AltPiv.Width = 80;
            this.AltPiv.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // SoundNoRepeat
            // 
            this.SoundNoRepeat.DecimalPlaces = 6;
            this.SoundNoRepeat.Maximum = 1000.0m;
            this.SoundNoRepeat.Minimum = 0.0m;
            this.SoundNoRepeat.HeaderText = "Sound No Repeat";
            this.SoundNoRepeat.Name = "SoundNoRepeat";
            this.SoundNoRepeat.Width = 80;
            this.SoundNoRepeat.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // LifeSpan
            //
            this.LifeSpan.DecimalPlaces = 0;
            this.LifeSpan.Maximum = 1000.0m;
            this.LifeSpan.Minimum = 0.0m;
            this.LifeSpan.HeaderText = "Life Span";
            this.LifeSpan.Name = "LifeSpan";
            this.LifeSpan.Width = 80;
            this.LifeSpan.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // LifeSpanJitter
            // 
            //this.LifeSpanJitter.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.LifeSpanJitter.DecimalPlaces = 6;
            this.LifeSpanJitter.Maximum = 1000.0m;
            this.LifeSpanJitter.Minimum = 0.0m;
            this.LifeSpanJitter.HeaderText = "Life Span Jitter";
            this.LifeSpanJitter.Name = "LifeSpanJitter";
            this.LifeSpanJitter.Width = 80;
            this.LifeSpanJitter.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Type
            // 
            this.Type.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.Type.HeaderText = "Type";
            this.Type.Items.AddRange(new object[] {
            "Create",
            "Destroy",
            "Local",
            "Posit",
            "PositOnly",
            "SetLight",
            "SetState",
            "Start",
            "StartPositOnly"
            });
            this.Type.Name = "Type";
            // 
            // Inherit_Position
            // 
            this.Inherit_Position.HeaderText = "Inherit Position";
            this.Inherit_Position.ToolTipText = "Position";
            this.Inherit_Position.Name = "Inherit_Position";
            this.Inherit_Position.Width = 60;
            this.Inherit_Position.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Inherit_Rotation
            // 
            this.Inherit_Rotation.HeaderText = "Inherit Rotation";
            this.Inherit_Rotation.ToolTipText = "Rotation";
            this.Inherit_Rotation.Name = "Inherit_Rotation";
            this.Inherit_Rotation.Width = 60;
            this.Inherit_Rotation.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Inherit_Scale
            // 
            this.Inherit_Scale.HeaderText = "Inherit Scale";
            this.Inherit_Scale.ToolTipText = "Scale";
            this.Inherit_Scale.Name = "Inherit_Scale";
            this.Inherit_Scale.Width = 60;
            this.Inherit_Scale.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Inherit_SuperRotation
            // 
            this.Inherit_SuperRotation.HeaderText = "Inherit SuperRot";
            this.Inherit_SuperRotation.ToolTipText = "SuperRot";
            this.Inherit_SuperRotation.Name = "Inherit_SuperRotation";
            this.Inherit_SuperRotation.Width = 60;
            this.Inherit_SuperRotation.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Update_Position
            // 
            this.Update_Position.HeaderText = "Update Position";
            this.Update_Position.ToolTipText = "Position";
            this.Update_Position.Name = "Update_Position";
            this.Update_Position.Width = 60;
            this.Update_Position.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Update_Rotation
            // 
            this.Update_Rotation.HeaderText = "Update Rotation";
            this.Update_Rotation.ToolTipText = "Rotation";
            this.Update_Rotation.Name = "Update_Rotation";
            this.Update_Rotation.Width = 60;
            this.Update_Rotation.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Update_Scale
            // 
            this.Update_Scale.HeaderText = "Update Scale";
            this.Update_Scale.ToolTipText = "Scale";
            this.Update_Scale.Name = "Update_Scale";
            this.Update_Scale.Width = 60;
            this.Update_Scale.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Update_SuperRotation
            // 
            this.Update_SuperRotation.HeaderText = "Update SuperRot";
            this.Update_SuperRotation.ToolTipText = "SuperRot";
            this.Update_SuperRotation.Name = "Update_SuperRotation";
            this.Update_SuperRotation.Width = 60;
            this.Update_SuperRotation.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;  
            //
            // CEvent
            //
            this.CEvent.HeaderText = "CEvent";
            this.CEvent.Name = "CEvent";
            this.CEvent.Width = 80;
            this.CEvent.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // CThresh
            //
            this.CThresh.DecimalPlaces = 6;
            this.CThresh.Maximum = 1000.0m;
            this.CThresh.Minimum = 0.0m;
            this.CThresh.HeaderText = "CThresh";
            this.CThresh.Name = "CThresh";
            this.CThresh.Width = 80;
            this.CThresh.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // CDestroy
            // 
            this.CDestroy.HeaderText = "CDestroy";
            this.CDestroy.Name = "CDestroy";
            this.CDestroy.Width = 60;
            this.CDestroy.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // JEvent
            //
            this.JEvent.HeaderText = "JEvent";
            this.JEvent.Name = "JEvent";
            this.JEvent.Width = 80;
            this.JEvent.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // JDestroy
            // 
            this.JDestroy.HeaderText = "JDestroy";
            this.JDestroy.Name = "JDestroy";
            this.JDestroy.Width = 60;
            this.JDestroy.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // OneAtATime
            // 
            this.Flags.HeaderText = "Flags";
            this.Flags.Name = "Flags";
            this.Flags.Width = 80;
            this.Flags.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.Flags.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
            /*/ 
            // OneAtATime
            // 
            this.OneAtATime.HeaderText = "One At A Time";
            this.OneAtATime.Name = "OneAtATime";
            this.OneAtATime.Width = 60;
            this.OneAtATime.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // AdoptParentEntity
            // 
            this.AdoptParentEntity.HeaderText = "Adopt Parent Entity";
            this.AdoptParentEntity.Name = "AdoptParentEntity";
            this.AdoptParentEntity.Width = 60;
            this.AdoptParentEntity.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            /*/ 
            // Lighting
            // 
            this.Lighting.HeaderText = "Lighting";
            this.Lighting.Name = "Lighting";
            this.Lighting.Width = 60;
            this.Lighting.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // Anim
            //
            this.Anim.HeaderText = "Anim";
            this.Anim.Name = "Anim";
            this.Anim.Width = 80;
            this.Anim.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // Cape
            //
            this.Cape.HeaderText = "Cape";
            this.Cape.Name = "Cape";
            this.Cape.Width = 80;
            this.Cape.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // NewState
            //
            this.NewState.HeaderText = "New State";
            this.NewState.Name = "NewState";
            this.NewState.Width = 80;
            this.NewState.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            //
            // ChildFx
            //
            this.ChildFx.HeaderText = "Child Fx";
            this.ChildFx.Name = "ChildFx";
            this.ChildFx.Width = 400;
            this.ChildFx.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // PowerMin
            // 
            this.PowerMin.DecimalPlaces = 0;
            this.PowerMin.Maximum = 10.0m;
            this.PowerMin.Minimum = 0.0m;
            this.PowerMin.HeaderText = "Power Min";
            this.PowerMin.Name = "PowerMin";
            this.PowerMin.Width = 80;
            this.PowerMin.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // PowerMax
            // 
            this.PowerMax.DecimalPlaces = 0;
            this.PowerMax.Maximum = 10.0m;
            this.PowerMax.Minimum = 0.0m;
            this.PowerMax.HeaderText = "Power Max";
            this.PowerMax.Name = "PowerMax";
            this.PowerMax.Width = 80;
            this.PowerMax.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // EventWin
            // 
            this.ClientSize = new System.Drawing.Size(839, 381);
            this.Controls.Add(this.dataGridViewEvents);
            this.Name = "EventWin";
            this.Text = "FXWin";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewEvents)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        //private System.Windows.Forms.DataGridView dataGridViewEvents;
        private COH_DataGridView dataGridViewEvents;
        private System.Windows.Forms.DataGridViewTextBoxColumn EName;
        //private System.Windows.Forms.DataGridViewTextBoxColumn At;

        //"BhvrOverride"
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn CRotation;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn CameraSpace;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn HardwareOnly;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn SoftwareOnly;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn PhysicsOnly;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn RayLength;

        private common.COH_DataGridViewCheckBoxColumn Inherit_Rotation;
        private common.COH_DataGridViewCheckBoxColumn Inherit_Position;
        private common.COH_DataGridViewCheckBoxColumn Inherit_Scale;
        private common.COH_DataGridViewCheckBoxColumn Inherit_SuperRotation;

        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn Update_Rotation;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn Update_Position;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn Update_Scale;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn Update_SuperRotation;

        private System.Windows.Forms.DataGridViewTextBoxColumn AtRayFx;
        private System.Windows.Forms.DataGridViewTextBoxColumn Geom;
        	
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn AltPiv;
        private System.Windows.Forms.DataGridViewTextBoxColumn AnimPiv;

        private System.Windows.Forms.DataGridViewTextBoxColumn SetState;
        	
        private System.Windows.Forms.DataGridViewTextBoxColumn Magnet;
        private System.Windows.Forms.DataGridViewTextBoxColumn LookAt;
        private System.Windows.Forms.DataGridViewTextBoxColumn PMagnet;
        private System.Windows.Forms.DataGridViewTextBoxColumn POther;
	
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn SoundNoRepeat;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn LifeSpan;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn LifeSpanJitter;
        		
        private System.Windows.Forms.DataGridViewTextBoxColumn While;
        private System.Windows.Forms.DataGridViewTextBoxColumn Until;
        private System.Windows.Forms.DataGridViewTextBoxColumn WorldGroup;

        private System.Windows.Forms.DataGridViewComboBoxColumn Type;
        private System.Windows.Forms.DataGridViewComboBoxColumn At;


        private System.Windows.Forms.DataGridViewTextBoxColumn CEvent;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn CThresh;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn CDestroy;
        private System.Windows.Forms.DataGridViewTextBoxColumn JEvent;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn JDestroy;

        private System.Windows.Forms.DataGridViewTextBoxColumn Flags;
        // moved in Flags
        //private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn OneAtATime;
        //private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn AdoptParentEntity;
        private COH_CostumeUpdater.common.COH_DataGridViewCheckBoxColumn Lighting;

        private System.Windows.Forms.DataGridViewTextBoxColumn Cape;
        private System.Windows.Forms.DataGridViewTextBoxColumn Anim;
        private System.Windows.Forms.DataGridViewTextBoxColumn NewState;
        private System.Windows.Forms.DataGridViewTextBoxColumn ChildFx;


        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn PowerMin;
        private COH_CostumeUpdater.common.DataGridViewNumericUpDownColumn PowerMax;
        private System.Windows.Forms.DataGridViewImageColumn Splat;

        private System.Windows.Forms.ContextMenu dgvContextMenu;
        private System.Windows.Forms.MenuItem openChildFxMenuItem;
        private System.Windows.Forms.MenuItem addChildFxMenuItem;
        private System.Windows.Forms.MenuItem addGeoMenuItem;
        //"Splat"//image		


        //"Part" 

    }
}