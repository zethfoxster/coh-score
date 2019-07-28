using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.common
{
    //http://www.codeproject.com/KB/combobox/ImageCombo_NET.aspx
    public class MComboBoxItem : object
    {
        // forecolor: transparent = inherit
        private Color forecolor = Color.FromKnownColor(KnownColor.Transparent);
        private bool mark = false;
        private bool isSaved = true;
        private bool isDirty = false;
    
        private int imageindex = -1;
        private object tag = null;
        private string text = null;

        // constructors
        public MComboBoxItem()
        {
        }

        public MComboBoxItem(string Text)
        {
            text = Text;
        }

        public MComboBoxItem(string Text, int ImageIndex)
        {
            text = Text;
            imageindex = ImageIndex;
        }

        public MComboBoxItem(string Text, int ImageIndex, bool Mark)
        {
            text = Text;
            imageindex = ImageIndex;
            mark = Mark;
        }

        public MComboBoxItem(string Text, int ImageIndex, bool Mark, Color ForeColor)
        {
            text = Text;
            imageindex = ImageIndex;
            mark = Mark;
            forecolor = ForeColor;
        }

        public MComboBoxItem(string Text, int ImageIndex, bool Mark, Color ForeColor, object Tag)
        {
            text = Text;
            imageindex = ImageIndex;
            mark = Mark;
            forecolor = ForeColor;
            tag = Tag;
        }

        public MComboBoxItem(string Text, int ImageIndex, bool Mark, bool IsSaved, bool IsDirty, object Tag)
        {
            text = Text;
            imageindex = ImageIndex;
            mark = Mark;
            isSaved = IsSaved;
            isDirty = IsDirty;
            tag = Tag;
        }
        // forecolor
        public Color ForeColor
        {
            get
            {
                return forecolor;
            }
            set
            {
                forecolor = value;
            }
        }

        // image index
        public int ImageIndex
        {
            get
            {
                return imageindex;
            }
            set
            {
                imageindex = value;
            }
        }

        // mark (bold)
        public bool Mark
        {
            get
            {
                return mark;
            }
            set
            {
                mark = value;
            }
        }
        
        // IsSaved (bold)
        public bool IsSaved
        {
            get
            {
                return isSaved;
            }
            set
            {
                isSaved = value;
            }
        }

        // IsDirty (bold)
        public bool IsDirty
        {
            get
            {
                return isDirty;
            }
            set
            {
                isDirty = value;
            }
        }

        // tag
        public object Tag
        {
            get
            {
                return tag;
            }
            set
            {
                tag = value;
            }
        }

        // item text
        public string Text
        {
            get
            {
                return text;
            }
            set
            {
                text = value;
            }
        }

        // ToString() should return item text
        public override string ToString()
        {
            return text;
        }

    }
}
