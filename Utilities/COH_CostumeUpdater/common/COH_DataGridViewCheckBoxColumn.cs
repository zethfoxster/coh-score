using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace COH_CostumeUpdater.common
{
    public class COH_DataGridViewCheckBoxColumn : DataGridViewCheckBoxColumn
    {
        public override DataGridViewCell CellTemplate
        {
            get
            {
                return new COH_DataGridViewCheckBoxCell();
            }
        }
    }

    public class COH_DataGridViewCheckBoxCell : DataGridViewCheckBoxCell
    {
        /// <summary>
        /// Little utility function called by the Paint function to see if a particular part needs to be painted. 
        /// </summary>
        private static bool PartPainted(DataGridViewPaintParts paintParts, DataGridViewPaintParts paintPart)
        {
            return (paintParts & paintPart) != 0;
        }
        protected override void Paint(Graphics graphics, Rectangle clipBounds, Rectangle cellBounds, int rowIndex, DataGridViewElementStates elementState, object value, object formattedValue, string errorText, DataGridViewCellStyle cellStyle, DataGridViewAdvancedBorderStyle advancedBorderStyle, DataGridViewPaintParts paintParts)
        {
            // the base Paint implementation paints the check box
            base.Paint(graphics, clipBounds, cellBounds, rowIndex, elementState, value, formattedValue, errorText, cellStyle, advancedBorderStyle, paintParts);

            // Get the check box bounds: they are the content bounds
            Rectangle contentBounds = this.GetContentBounds(rowIndex);

            if(this.Style.ForeColor == Color.Red)
                graphics.DrawRectangle(new Pen(this.Style.ForeColor, 2.0f), 
                    new Rectangle(  cellBounds.X + contentBounds.X - 2, 
                                    cellBounds.Y + 2, 
                                    contentBounds.Width + 4, 
                                    contentBounds.Height + 4));
        }

    }
}
