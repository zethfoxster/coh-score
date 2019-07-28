namespace FxLauncher
{
    partial class AddFavoriteForm
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.name_txtBx = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.favorite_comboBox = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.newFolder_btn = new System.Windows.Forms.Button();
            this.add_btn = new System.Windows.Forms.Button();
            this.cancel_btn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(60, 28);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(367, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Add the selected node as a favorite. The favorite FX collection  are saved at";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(60, 46);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(206, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "this file \"C:/Cryptic/scratch/favoriteFX.txt\"";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(60, 9);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(79, 13);
            this.label3.TabIndex = 1;
            this.label3.Text = "Add Favorite";
            // 
            // name_txtBx
            // 
            this.name_txtBx.Location = new System.Drawing.Point(63, 68);
            this.name_txtBx.Name = "name_txtBx";
            this.name_txtBx.ReadOnly = true;
            this.name_txtBx.Size = new System.Drawing.Size(372, 20);
            this.name_txtBx.TabIndex = 2;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(8, 71);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(38, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Name:";
            // 
            // favorite_comboBox
            // 
            this.favorite_comboBox.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.favorite_comboBox.FormattingEnabled = true;
            this.favorite_comboBox.Location = new System.Drawing.Point(63, 97);
            this.favorite_comboBox.Name = "favorite_comboBox";
            this.favorite_comboBox.Size = new System.Drawing.Size(277, 21);
            this.favorite_comboBox.TabIndex = 4;
            this.favorite_comboBox.Text = "      Favorites";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(8, 100);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(52, 13);
            this.label5.TabIndex = 3;
            this.label5.Text = "Create in:";
            // 
            // newFolder_btn
            // 
            this.newFolder_btn.Location = new System.Drawing.Point(355, 97);
            this.newFolder_btn.Name = "newFolder_btn";
            this.newFolder_btn.Size = new System.Drawing.Size(80, 23);
            this.newFolder_btn.TabIndex = 5;
            this.newFolder_btn.Text = "New Folder";
            this.newFolder_btn.UseVisualStyleBackColor = true;
            this.newFolder_btn.Click += new System.EventHandler(this.newFolder_btn_Click);
            // 
            // add_btn
            // 
            this.add_btn.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.add_btn.Location = new System.Drawing.Point(265, 132);
            this.add_btn.Name = "add_btn";
            this.add_btn.Size = new System.Drawing.Size(80, 23);
            this.add_btn.TabIndex = 6;
            this.add_btn.Text = "Add";
            this.add_btn.UseVisualStyleBackColor = true;
            // 
            // cancel_btn
            // 
            this.cancel_btn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancel_btn.Location = new System.Drawing.Point(355, 132);
            this.cancel_btn.Name = "cancel_btn";
            this.cancel_btn.Size = new System.Drawing.Size(80, 23);
            this.cancel_btn.TabIndex = 6;
            this.cancel_btn.Text = "Cancel";
            this.cancel_btn.UseVisualStyleBackColor = true;
            // 
            // AddFavoriteForm
            // 
            this.AcceptButton = this.add_btn;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancel_btn;
            this.ClientSize = new System.Drawing.Size(447, 167);
            this.Controls.Add(this.cancel_btn);
            this.Controls.Add(this.add_btn);
            this.Controls.Add(this.newFolder_btn);
            this.Controls.Add(this.favorite_comboBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.name_txtBx);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.MaximumSize = new System.Drawing.Size(463, 205);
            this.MinimumSize = new System.Drawing.Size(463, 205);
            this.Name = "AddFavoriteForm";
            this.Text = "AddFavoriteForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }


        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        public System.Windows.Forms.TextBox name_txtBx;
        private System.Windows.Forms.Label label4;
        public System.Windows.Forms.ComboBox favorite_comboBox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button newFolder_btn;
        private System.Windows.Forms.Button add_btn;
        private System.Windows.Forms.Button cancel_btn;
    }
}