using System;
using System.Collections;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.objectTrick
{
    class CheckBox_SpinCombo
    {
        public CheckBox checkBoxControl;
        public NumericUpDown[] spinBoxControls;
        public ComboBox comboBoxControl;
        public RadioButton[] radioButtonControls;
        public string controlName;

        public CheckBox_SpinCombo(CheckBox chkBx, NumericUpDown[] numUpDwns, string name)
        {
            initialize(chkBx, numUpDwns, null, null, name);
        }

        public CheckBox_SpinCombo(CheckBox chkBx, NumericUpDown[] numUpDwns, ComboBox cmboBx, RadioButton[] radioBtns, string name)
        {
            initialize(chkBx, numUpDwns, cmboBx, radioBtns, name);
        }
        public void initialize(CheckBox chkBx, NumericUpDown[] numUpDwns, ComboBox cmboBx, RadioButton[] radioBtns, string name)
        {

            checkBoxControl = chkBx;

            this.checkBoxControl.Click += new EventHandler(checkBoxControl_Click);

            if (numUpDwns != null)
            {
                spinBoxControls = new NumericUpDown[numUpDwns.Length];
                numUpDwns.CopyTo(spinBoxControls, 0);
            }
            else
                spinBoxControls = null;

            if (radioBtns != null)
            {
                radioButtonControls = new RadioButton[radioBtns.Length];
                radioBtns.CopyTo(radioButtonControls, 0);
            }
            else
                radioButtonControls = null;

            if (cmboBx != null)
            {
                comboBoxControl = cmboBx;
            }
            else
                comboBoxControl = null;

            if (name.Length > 0)
                controlName = name;
            else
                controlName = checkBoxControl.Text;
        }

        void checkBoxControl_Click(object sender, EventArgs e)
        {
            this.enableControl(this.checkBoxControl.Checked);
        }

        public void enableControl(bool enable)
        {
            if (spinBoxControls != null)
            {
                foreach (NumericUpDown n in spinBoxControls)
                {
                    n.Enabled = enable;
                }
            }

            if (radioButtonControls != null)
            {
                foreach (RadioButton r in radioButtonControls)
                {
                    r.Enabled = enable;
                }
            }

            if (comboBoxControl != null)
            {
                comboBoxControl.Enabled = enable;
                if (!enable)
                {
                    comboBoxControl.Text = "-Type-";
                }
            }

            checkBoxControl.Checked = enable;

            if (enable)
                checkBoxControl.BackColor = Color.Yellow;
            else
                checkBoxControl.BackColor = SystemColors.Control;
        }

        public void updateSpinBoxes(string[] fieldsValStr)
        {
            this.enableControl(true);

            if (spinBoxControls != null)
            {
                for (int i = 0; i < spinBoxControls.Length; i++)
                {
                    NumericUpDown spinBx = (NumericUpDown)spinBoxControls[i];

                    if (spinBx != null)
                    {
                        decimal val = decimal.Parse(fieldsValStr[i]);
                        if (val > spinBx.Maximum)
                            spinBx.Maximum = val;

                        else if (val < spinBx.Minimum)
                            spinBx.Minimum = val;

                        spinBx.Value = val;
                    }
                }
            }

            if (comboBoxControl != null && fieldsValStr.Length > 2)
            {
                comboBoxControl.Text = fieldsValStr[2];
                //comboBoxControl.SelectedIndex = comboBoxControl.FindString(fieldsValStr[2]);
                if (fieldsValStr.Length == 4)
                    setRadioButton(fieldsValStr[3].ToLower());
                else if (radioButtonControls != null)
                    radioButtonControls[0].Checked = true;
            }
            if (this.controlName.Equals("CastShadow") &&
                fieldsValStr.Length > 0 &&
                !fieldsValStr[0].Trim().Equals("1"))
            {
                this.enableControl(false);
            }
        }

        private void setRadioButton(string str)
        {
            if (radioButtonControls != null)
            {
                switch (str)
                {
                    case "framesnap":
                        radioButtonControls[2].Checked = true;
                        break;

                    case "pingpong":
                        radioButtonControls[1].Checked = true;
                        break;

                    default:
                        radioButtonControls[0].Checked = true;
                        break;
                }
            }
        }

    }
}
