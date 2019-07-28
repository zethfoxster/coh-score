namespace COH_CostumeUpdater.common
{
    partial class UserPrompt
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
            this.prompSplitContainer = new System.Windows.Forms.SplitContainer();
            this.displayMessageLable = new System.Windows.Forms.Label();
            this.inputLabel = new System.Windows.Forms.Label();
            this.inputTextBox = new System.Windows.Forms.TextBox();
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.prompSplitContainer.Panel1.SuspendLayout();
            this.prompSplitContainer.Panel2.SuspendLayout();
            this.prompSplitContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // prompSplitContainer
            // 
            this.prompSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.prompSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.prompSplitContainer.Name = "prompSplitContainer";
            this.prompSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // prompSplitContainer.Panel1
            // 
            this.prompSplitContainer.Panel1.Controls.Add(this.displayMessageLable);
            this.prompSplitContainer.Panel1.Controls.Add(this.inputLabel);
            this.prompSplitContainer.Panel1.Controls.Add(this.inputTextBox);
            // 
            // prompSplitContainer.Panel2
            // 
            this.prompSplitContainer.Panel2.Controls.Add(this.cancelButton);
            this.prompSplitContainer.Panel2.Controls.Add(this.okButton);
            this.prompSplitContainer.Size = new System.Drawing.Size(444, 122);
            this.prompSplitContainer.SplitterDistance = 84;
            this.prompSplitContainer.TabIndex = 0;
            // 
            // displayMessageLable
            // 
            this.displayMessageLable.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.displayMessageLable.AutoSize = true;
            this.displayMessageLable.Location = new System.Drawing.Point(12, 9);
            this.displayMessageLable.Name = "displayMessageLable";
            this.displayMessageLable.Size = new System.Drawing.Size(162, 13);
            this.displayMessageLable.TabIndex = 3;
            this.displayMessageLable.Text = "";
            // 
            // inputLabel
            // 
            this.inputLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.inputLabel.AutoSize = true;
            this.inputLabel.Location = new System.Drawing.Point(12, 43);
            this.inputLabel.Name = "inputLabel";
            this.inputLabel.Size = new System.Drawing.Size(75, 13);
            this.inputLabel.TabIndex = 2;
            this.inputLabel.Text = "Material Name";
            // 
            // inputTextBox
            // 
            this.inputTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.inputTextBox.Location = new System.Drawing.Point(93, 40);
            this.inputTextBox.Name = "inputTextBox";
            this.inputTextBox.Size = new System.Drawing.Size(339, 20);
            this.inputTextBox.TabIndex = 1;
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(364, 6);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 25);
            this.cancelButton.TabIndex = 1;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // okButton
            // 
            this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(284, 6);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 25);
            this.okButton.TabIndex = 0;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // UserPrompt
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(444, 122);
            this.Controls.Add(this.prompSplitContainer);
            this.AcceptButton = this.okButton;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.formClosing);
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(460, 160);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(460, 160);
            this.Name = "UserPrompt";
            this.ShowIcon = false;
            this.Text = "UserPrompt";
            this.prompSplitContainer.Panel1.ResumeLayout(false);
            this.prompSplitContainer.Panel1.PerformLayout();
            this.prompSplitContainer.Panel2.ResumeLayout(false);
            this.prompSplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer prompSplitContainer;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.TextBox inputTextBox;
        private System.Windows.Forms.Label inputLabel;
        private System.Windows.Forms.Label displayMessageLable;

    }
}