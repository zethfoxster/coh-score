#ifndef UIREWARDCHOICE_H
#define UIREWARDCHOICE_H

int rewardChoiceWindow(void);

void setRewardDesc( char *desc );
void setRewardDisableChooseNothing( bool flag );
void addRewardChoice(int visible, int disabled, char *txt);
void clearRewardChoices(void);

#endif