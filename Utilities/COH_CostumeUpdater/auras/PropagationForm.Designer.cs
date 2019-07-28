namespace COH_CostumeUpdater.auras
{
    partial class PropagationForm
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
            this.propagationChBxList = new System.Windows.Forms.CheckedListBox();
            this.propagateFiles = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // propagationChBxList
            // 
            this.propagationChBxList.CheckOnClick = true;
            this.propagationChBxList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propagationChBxList.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.propagationChBxList.FormattingEnabled = true;
            this.propagationChBxList.Items.AddRange(new object[] {
            "Female",
            "Huge",
            "Arachnos Female Soldier",
            "Arachnos Female Widow",
            "Arachnos Male Soldier",
            "Arachnos Male Widow",
            "Arachnos Huge Soldier",
            "Arachnos Huge Widow"});
            this.propagationChBxList.Location = new System.Drawing.Point(0, 0);
            this.propagationChBxList.Margin = new System.Windows.Forms.Padding(100, 20, 20, 20);
            this.propagationChBxList.Name = "propagationChBxList";
            this.propagationChBxList.Size = new System.Drawing.Size(284, 229);
            this.propagationChBxList.TabIndex = 0;
            // 
            // propagateFiles
            // 
            this.propagateFiles.AutoSize = true;
            this.propagateFiles.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.propagateFiles.Location = new System.Drawing.Point(0, 239);
            this.propagateFiles.Name = "propagateFiles";
            this.propagateFiles.Size = new System.Drawing.Size(284, 23);
            this.propagateFiles.TabIndex = 1;
            this.propagateFiles.Text = "Propagate Files";
            this.propagateFiles.UseVisualStyleBackColor = true;
            this.propagateFiles.Click += new System.EventHandler(this.propagateFiles_Click);
            // 
            // PropagationForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.propagationChBxList);
            this.Controls.Add(this.propagateFiles);
            this.Name = "PropagationForm";
            this.Text = "Propagation";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button propagateFiles;
        private System.Windows.Forms.CheckedListBox propagationChBxList;
        public System.Collections.ArrayList checkedItems;
    }
}

