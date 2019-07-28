using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class BehaviorKey
    {
        public string name;
        public string group;
        public string subGroup;
        public string fieldsLabel;
        public int numFields;
        public int min;
        public int max;
        public decimal[] defaultVal;
        public int numDecimalPlaces;
        public decimal[] values;
        public bool isOverridable;
        public bool bhvrOveride;
        public bool isColor;
        public bool isCkbx;
        public string[] comboItems;

        public BehaviorKey(string key_name, string key_group, string key_subGroup, string key_label, int key_numFields, int key_min,
            int key_max, decimal[] key_default, int key_numDecimal, bool key_isOverridable, bool key_isColor, string [] key_comboItems, bool key_isCkbx)
        {
            comboItems = null;
            name = key_name;
            group = key_group;
            subGroup = key_subGroup;
            fieldsLabel = key_label;
            numFields = key_numFields;
            min = key_min;
            max = key_max;
            numDecimalPlaces = key_numDecimal;
            isOverridable = key_isOverridable;
            isColor = key_isColor;
            isCkbx = key_isCkbx;

            if (numFields > 0)
            {
                defaultVal = new decimal[numFields];
                key_default.CopyTo(defaultVal, 0);
                values = new decimal[numFields];
                key_default.CopyTo(values, 0);
            }
            else
            {
                //defaultVal = null;
                //values = null;
                defaultVal = new decimal[1];
                key_default.CopyTo(defaultVal, 0);
                values = new decimal[1];
                key_default.CopyTo(values, 0);
            }
            if (key_comboItems != null)
            {
                comboItems = new string[key_comboItems.Length];
                key_comboItems.CopyTo(comboItems, 0);
            }
            bhvrOveride = false;
        }
        public void reset()
        {
            defaultVal.CopyTo(values, 0);
            bhvrOveride = false;
        }
        public void setValues(decimal[] val)
        {
            if (val.Length == numFields)
                val.CopyTo(values, 0);
        }
        public decimal[] getValues()
        {
            return (decimal[])values.Clone();
        }
    }
}
