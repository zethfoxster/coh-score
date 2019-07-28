using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class ParticalKey:BehaviorKey
    {
        public ParticalKey(string key_name, string key_group, string key_subGroup, string key_label, int key_numFields, int key_min, int key_max, 
            decimal[] key_default, int key_numDecimal, bool key_isColor, string [] key_comboItems, bool key_isCkbx):
        base(key_name, key_group, key_subGroup, key_label, key_numFields, key_min, key_max, key_default, key_numDecimal, false, key_isColor, key_comboItems, key_isCkbx)
        {
        }
    }
}
