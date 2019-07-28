namespace COH_CostumeUpdater.fxEditor
{
    partial class FilterActive
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
            this.filter_tabControl = new System.Windows.Forms.TabControl();
            this.Event_tabPage = new System.Windows.Forms.TabPage();
            this.part_tabPage = new System.Windows.Forms.TabPage();
            this.bhvr_tabPage = new System.Windows.Forms.TabPage();
            this.panel1 = new System.Windows.Forms.Panel();
            this.allOn_btn = new System.Windows.Forms.Button();
            this.allOff_btn = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.ok_btn = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.basic_tabPage = new System.Windows.Forms.TabPage();
            this.colorAlpha_tabPage = new System.Windows.Forms.TabPage();
            this.physics_tabPage = new System.Windows.Forms.TabPage();
            this.shakeBlurLight_tabPage = new System.Windows.Forms.TabPage();
            this.animationSplat_tabPage = new System.Windows.Forms.TabPage();
            this.part_tabControl = new System.Windows.Forms.TabControl();
            this.emitter_tabPage = new System.Windows.Forms.TabPage();
            this.motion_tabPage = new System.Windows.Forms.TabPage();
            this.particle_tabPage = new System.Windows.Forms.TabPage();
            this.textureColor_tabPage = new System.Windows.Forms.TabPage();
            this.filter_tabControl.SuspendLayout();
            this.part_tabPage.SuspendLayout();
            this.bhvr_tabPage.SuspendLayout();
            this.panel1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.part_tabControl.SuspendLayout();
            this.SuspendLayout();
            // 
            // filter_tabControl
            // 
            this.filter_tabControl.Controls.Add(this.Event_tabPage);
            this.filter_tabControl.Controls.Add(this.part_tabPage);
            this.filter_tabControl.Controls.Add(this.bhvr_tabPage);
            this.filter_tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.filter_tabControl.Location = new System.Drawing.Point(0, 0);
            this.filter_tabControl.Name = "filter_tabControl";
            this.filter_tabControl.SelectedIndex = 0;
            this.filter_tabControl.Size = new System.Drawing.Size(755, 418);
            this.filter_tabControl.TabIndex = 0;
            // 
            // Event_tabPage
            // 
            this.Event_tabPage.AutoScroll = true;
            this.Event_tabPage.Location = new System.Drawing.Point(4, 22);
            this.Event_tabPage.Name = "Event_tabPage";
            this.Event_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.Event_tabPage.Size = new System.Drawing.Size(747, 392);
            this.Event_tabPage.TabIndex = 0;
            this.Event_tabPage.Text = "Event";
            this.Event_tabPage.UseVisualStyleBackColor = true;
            // 
            // part_tabPage
            // 
            //this.part_tabPage.Controls.Add(this.part_tabControl);
            this.part_tabPage.Location = new System.Drawing.Point(4, 22);
            this.part_tabPage.Name = "part_tabPage";
            this.part_tabPage.Size = new System.Drawing.Size(747, 392);
            this.part_tabPage.TabIndex = 2;
            this.part_tabPage.Text = "Part";
            this.part_tabPage.UseVisualStyleBackColor = true;
            this.part_tabPage.AutoScroll = true;
            // 
            // bhvr_tabPage
            // 
            //this.bhvr_tabPage.Controls.Add(this.tabControl1);
            this.bhvr_tabPage.Location = new System.Drawing.Point(4, 22);
            this.bhvr_tabPage.Name = "bhvr_tabPage";
            this.bhvr_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.bhvr_tabPage.Size = new System.Drawing.Size(747, 392);
            this.bhvr_tabPage.TabIndex = 1;
            this.bhvr_tabPage.Text = "Bhvr";
            this.bhvr_tabPage.UseVisualStyleBackColor = true;
            this.bhvr_tabPage.AutoScroll = true;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.allOn_btn);
            this.panel1.Controls.Add(this.allOff_btn);
            this.panel1.Controls.Add(this.button2);
            this.panel1.Controls.Add(this.ok_btn);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 418);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(755, 38);
            this.panel1.TabIndex = 1;
            // 
            // allOn_btn
            // 
            this.allOn_btn.Location = new System.Drawing.Point(171, 7);
            this.allOn_btn.Name = "allOn_btn";
            this.allOn_btn.Size = new System.Drawing.Size(40, 20);
            this.allOn_btn.TabIndex = 1;
            this.allOn_btn.Text = "ON";
            this.allOn_btn.UseVisualStyleBackColor = true;
            this.allOn_btn.Click += new System.EventHandler(this.allOn_btn_Click);
            // 
            // allOff_btn
            // 
            this.allOff_btn.Location = new System.Drawing.Point(126, 7);
            this.allOff_btn.Name = "allOff_btn";
            this.allOff_btn.Size = new System.Drawing.Size(40, 20);
            this.allOff_btn.TabIndex = 1;
            this.allOff_btn.Text = "OFF";
            this.allOff_btn.UseVisualStyleBackColor = true;
            this.allOff_btn.Click += new System.EventHandler(this.allOff_btn_Click);
            // 
            // button2
            // 
            this.button2.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.button2.Location = new System.Drawing.Point(676, 7);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 0;
            this.button2.Text = "Cancel";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // ok_btn
            // 
            this.ok_btn.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.ok_btn.Location = new System.Drawing.Point(595, 7);
            this.ok_btn.Name = "ok_btn";
            this.ok_btn.Size = new System.Drawing.Size(75, 23);
            this.ok_btn.TabIndex = 0;
            this.ok_btn.Text = "OK";
            this.ok_btn.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(4, 10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(121, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Turn All Checkboxs:";
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.basic_tabPage);
            this.tabControl1.Controls.Add(this.colorAlpha_tabPage);
            this.tabControl1.Controls.Add(this.physics_tabPage);
            this.tabControl1.Controls.Add(this.shakeBlurLight_tabPage);
            this.tabControl1.Controls.Add(this.animationSplat_tabPage);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(3, 3);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(741, 386);
            this.tabControl1.TabIndex = 0;
            // 
            // basic_tabPage
            // 
            this.basic_tabPage.AutoScroll = true;
            this.basic_tabPage.Location = new System.Drawing.Point(4, 22);
            this.basic_tabPage.Name = "basic_tabPage";
            this.basic_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.basic_tabPage.Size = new System.Drawing.Size(733, 360);
            this.basic_tabPage.TabIndex = 0;
            this.basic_tabPage.Text = "Basic";
            this.basic_tabPage.UseVisualStyleBackColor = true;
            // 
            // colorAlpha_tabPage
            // 
            this.colorAlpha_tabPage.AutoScroll = true;
            this.colorAlpha_tabPage.Location = new System.Drawing.Point(4, 22);
            this.colorAlpha_tabPage.Name = "colorAlpha_tabPage";
            this.colorAlpha_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.colorAlpha_tabPage.Size = new System.Drawing.Size(733, 360);
            this.colorAlpha_tabPage.TabIndex = 1;
            this.colorAlpha_tabPage.Text = "Color/Alpha";
            this.colorAlpha_tabPage.UseVisualStyleBackColor = true;
            // 
            // physics_tabPage
            // 
            this.physics_tabPage.AutoScroll = true;
            this.physics_tabPage.Location = new System.Drawing.Point(4, 22);
            this.physics_tabPage.Name = "physics_tabPage";
            this.physics_tabPage.Size = new System.Drawing.Size(733, 360);
            this.physics_tabPage.TabIndex = 2;
            this.physics_tabPage.Text = "Physics";
            this.physics_tabPage.UseVisualStyleBackColor = true;
            // 
            // shakeBlurLight_tabPage
            // 
            this.shakeBlurLight_tabPage.AutoScroll = true;
            this.shakeBlurLight_tabPage.Location = new System.Drawing.Point(4, 22);
            this.shakeBlurLight_tabPage.Name = "shakeBlurLight_tabPage";
            this.shakeBlurLight_tabPage.Size = new System.Drawing.Size(733, 360);
            this.shakeBlurLight_tabPage.TabIndex = 3;
            this.shakeBlurLight_tabPage.Text = "Shake/Blur/Light";
            this.shakeBlurLight_tabPage.UseVisualStyleBackColor = true;
            // 
            // animationSplat_tabPage
            // 
            this.animationSplat_tabPage.AutoScroll = true;
            this.animationSplat_tabPage.Location = new System.Drawing.Point(4, 22);
            this.animationSplat_tabPage.Name = "animationSplat_tabPage";
            this.animationSplat_tabPage.Size = new System.Drawing.Size(733, 360);
            this.animationSplat_tabPage.TabIndex = 4;
            this.animationSplat_tabPage.Text = "Animation/Splat";
            this.animationSplat_tabPage.UseVisualStyleBackColor = true;
            // 
            // part_tabControl
            //
            this.part_tabControl.Controls.Add(this.emitter_tabPage);
            this.part_tabControl.Controls.Add(this.motion_tabPage);
            this.part_tabControl.Controls.Add(this.particle_tabPage);
            this.part_tabControl.Controls.Add(this.textureColor_tabPage);
            this.part_tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.part_tabControl.Location = new System.Drawing.Point(0, 0);
            this.part_tabControl.Name = "part_tabControl";
            this.part_tabControl.SelectedIndex = 0;
            this.part_tabControl.Size = new System.Drawing.Size(747, 392);
            this.part_tabControl.TabIndex = 0;
            // 
            // emitter_tabPage
            // 
            this.emitter_tabPage.AutoScroll = true;
            this.emitter_tabPage.Location = new System.Drawing.Point(4, 22);
            this.emitter_tabPage.Name = "emitter_tabPage";
            this.emitter_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.emitter_tabPage.Size = new System.Drawing.Size(739, 366);
            this.emitter_tabPage.TabIndex = 0;
            this.emitter_tabPage.Text = "Emitter";
            this.emitter_tabPage.UseVisualStyleBackColor = true;
            // 
            // motion_tabPage
            // 
            this.motion_tabPage.AutoScroll = true;
            this.motion_tabPage.Location = new System.Drawing.Point(4, 22);
            this.motion_tabPage.Name = "motion_tabPage";
            this.motion_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.motion_tabPage.Size = new System.Drawing.Size(739, 366);
            this.motion_tabPage.TabIndex = 1;
            this.motion_tabPage.Text = "Motion";
            this.motion_tabPage.UseVisualStyleBackColor = true;
            // 
            // particle_tabPage
            // 
            this.particle_tabPage.AutoScroll = true;
            this.particle_tabPage.Location = new System.Drawing.Point(4, 22);
            this.particle_tabPage.Name = "particle_tabPage";
            this.particle_tabPage.Size = new System.Drawing.Size(739, 366);
            this.particle_tabPage.TabIndex = 2;
            this.particle_tabPage.Text = "Particle";
            this.particle_tabPage.UseVisualStyleBackColor = true;
            // 
            // textureColor_tabPage
            // 
            this.textureColor_tabPage.AutoScroll = true;
            this.textureColor_tabPage.Location = new System.Drawing.Point(4, 22);
            this.textureColor_tabPage.Name = "textureColor_tabPage";
            this.textureColor_tabPage.Size = new System.Drawing.Size(739, 366);
            this.textureColor_tabPage.TabIndex = 3;
            this.textureColor_tabPage.Text = "Texture/Color";
            this.textureColor_tabPage.UseVisualStyleBackColor = true;
            // 
            // FilterActive
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(755, 456);
            this.Controls.Add(this.filter_tabControl);
            this.Controls.Add(this.panel1);
            this.Name = "FilterActive";
            this.Text = "FilterActive";
            this.filter_tabControl.ResumeLayout(false);
            this.part_tabPage.ResumeLayout(false);
            this.bhvr_tabPage.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.part_tabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl filter_tabControl;
        private System.Windows.Forms.TabPage Event_tabPage;
        private System.Windows.Forms.TabPage bhvr_tabPage;
        private System.Windows.Forms.TabPage part_tabPage;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button ok_btn;
        private System.Windows.Forms.Button allOn_btn;
        private System.Windows.Forms.Button allOff_btn;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage basic_tabPage;
        private System.Windows.Forms.TabPage colorAlpha_tabPage;
        private System.Windows.Forms.TabPage physics_tabPage;
        private System.Windows.Forms.TabPage shakeBlurLight_tabPage;
        private System.Windows.Forms.TabPage animationSplat_tabPage;
        private System.Windows.Forms.TabControl part_tabControl;
        private System.Windows.Forms.TabPage emitter_tabPage;
        private System.Windows.Forms.TabPage motion_tabPage;
        private System.Windows.Forms.TabPage particle_tabPage;
        private System.Windows.Forms.TabPage textureColor_tabPage;
    }
}