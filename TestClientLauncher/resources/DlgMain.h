

void DlgMainDoDialog(void);

int addLineToEnd(int nIDDlgItem, char *fmt, ...);
void updateLine(int nIDDlgItem, int index, char *fmt, ...);

void setStatusBar(int elem, const char *fmt, ...);

void enableRB2(void);
char *makeParamString(void);
