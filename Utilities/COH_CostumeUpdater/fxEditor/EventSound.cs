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
    public partial class EventSound : Panel
        //Form
    {
        private Color bColor;

        private string rootPath;

        public event EventHandler IsDirtyChanged;
                
        public bool isDirty;
        
        private bool settingCells;

        private bool selected;

        public event EventHandler CurrentRowChanged;

        private Event currentRowEvent;

        public EventSound(string root_path)
        {
            InitializeComponent();
            this.openFileD = new OpenFileDialog();
            rootPath = root_path;
            this.openFileD.InitialDirectory = rootPath + @"sound\Ogg";
            this.openFileD.Filter = "sound bank:(*.ogg)|*.ogg";
            this.bColor = this.BackColor;
            this.MouseClick += new MouseEventHandler(EventSound_MouseClick);
        }

        public void hideBtn()
        {
            this.addSound_btn.Hide();
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

        public bool Selected
        {
            get
            {
                return selected;
            }
        }

        public decimal Radius
        {
            get
            {
                return this.radius_num.Value;
            }
        }

        public decimal Ramp
        {
            get
            {
                return this.ramp_num.Value;
            }
        }

        public decimal Volume
        {
            get
            {
                return this.volume_num.Value;
            }
        }

        public string SoundFile
        {
            get
            {
                return this.sound_TxBx.Text;
            }
        }

        protected void OnCurrentRowChanged(EventArgs e)
        {
            if (CurrentRowChanged != null)
                CurrentRowChanged(this, e);
        }

        private void EventSound_MouseClick(object sender, MouseEventArgs e)
        {
            Event ev = currentRowEvent;

            currentRowEvent = (Event)this.Tag;

            if (currentRowEvent != null && ev != currentRowEvent)
            {
                if (CurrentRowChanged != null)
                    CurrentRowChanged(this, new EventArgs());
            }
        }

        public void removeHandles()
        {
            this.enable_checkBox.MouseClick -= new System.Windows.Forms.MouseEventHandler(EventSound_MouseClick);
            this.radius_Label.MouseClick -= new System.Windows.Forms.MouseEventHandler(EventSound_MouseClick);
            this.ramp_label.MouseClick -= new System.Windows.Forms.MouseEventHandler(EventSound_MouseClick);
            this.volume_label.MouseClick -= new System.Windows.Forms.MouseEventHandler(EventSound_MouseClick);
            this.soundFileBrowser_btn.Click -= new System.EventHandler(soundFileBrowser_btn_Click);
            this.sound_TxBx.Validated -= new System.EventHandler(sound_ValueChanged);
            this.radius_num.ValueChanged -= new System.EventHandler(sound_ValueChanged);
            this.ramp_num.ValueChanged -= new System.EventHandler(sound_ValueChanged);
            this.volume_num.ValueChanged -= new System.EventHandler(sound_ValueChanged);
        }

        public string getData()
        {
            if (sound_TxBx.Text.Trim().Length < 1)
                return null;

            else
                return string.Format("{0} {1} {2} {3}", this.sound_TxBx.Text,
                                                        this.radius_num.Value,
                                                        this.ramp_num.Value,
                                                        this.volume_num.Value);
        }

        public bool isCommented
        {
            get
            {
                return !enable_checkBox.Checked;
            }
        }

        public void selectSoundPanel()
        {
            selected = true;

            enable_checkBox_CheckedChanged(this.enable_checkBox, new EventArgs());
        }

        public void clearSelection()
        {
            selected = false;

            this.currentRowEvent = null;

            enable_checkBox_CheckedChanged(this.enable_checkBox, new EventArgs());
        }

        public void setData(string soundFileName, decimal radius, decimal ramp, decimal volume)
        {
            this.settingCells = true;

            this.sound_TxBx.Text = soundFileName;

            if (this.radius_num.Maximum < radius)
                this.radius_num.Maximum = radius;

            if (this.ramp_num.Maximum < ramp)
                this.ramp_num.Maximum = ramp;            
            
            if (this.volume_num.Maximum < volume)
                this.volume_num.Maximum = volume;

            this.radius_num.Value = radius;
            this.ramp_num.Value = ramp;
            this.volume_num.Value = volume;

            this.settingCells = false;
        }
       
        public void setCommentColor(bool isCommented)
        {
            this.enable_checkBox.Checked = !isCommented;
            enable_checkBox_CheckedChanged(this.enable_checkBox, new EventArgs());
        }
        
        private void soundFileBrowser_btn_Click(object sender, System.EventArgs e)
        {
            DialogResult dr = this.openFileD.ShowDialog();

            if (dr == DialogResult.OK)
            {
                this.sound_TxBx.Text = System.IO.Path.GetFileNameWithoutExtension(openFileD.FileName.Trim());
            }
        }

        private void sound_ValueChanged(object sender, System.EventArgs e)
        {
            if (!settingCells && !isDirty)
            {
                Event ev = (Event)this.Tag;
                if (ev != null)
                {
                    isDirty = true;

                    if (ev.isDirty != isDirty)
                    {
                        ev.isDirty = true;
                        ev.parent.isDirty = true;
                        ev.parent.parent.isDirty = true;

                        if (IsDirtyChanged != null)
                            IsDirtyChanged(this, e);
                    }
                }
            }
        }

        private void enable_checkBox_CheckedChanged(object sender, EventArgs e)
        {
            if (!enable_checkBox.Checked)
            {
                if(selected)
                    this.BackColor = Color.FromArgb(Math.Max(0, (int)Color.Khaki.R - 50),
                                            Math.Max(0, (int)Color.Khaki.G),
                                            Math.Max(0, (int)Color.Khaki.B - 50));
                else
                    this.BackColor = Color.FromArgb(Math.Max(0, (int)this.bColor.R - 50),
                                            Math.Max(0, (int)this.bColor.G),
                                            Math.Max(0, (int)this.bColor.B - 50));

            }
            else
            {
                if(selected)
                    this.BackColor = Color.Khaki;
                else
                    this.BackColor = bColor;
            }
        }
    }
}
