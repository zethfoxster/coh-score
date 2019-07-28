#ifndef _IGNORE_H
#define _IGNORE_H

void ignore(User *user,char *ignore_name);
void ignoreSpammer(User *user,char *ignore_name);
void ignoreAuth(User *user, char *authId );
void ignoreAuthSpammer(User *user, char *authId );

void unIgnore(User *user,char *ignore_name);
void unIgnoreAuth(User *user, char *authId);
void unSpamMe( User * user );

void ignoreList(User *user);
void ignoreListUpdate(User *user);
void notifyIgnoredBy(User * user);
int  isIgnoring(User *user, User *ignoree);
void ignoreRefreshList(User *user);

void setSpammerThreshold(User *user, char *seconds );
void setSpammerMultiplier(User *user, char *seconds );
void setSpammerDuration(User *user, char *seconds );

extern int ignore_spammer_threshold;
extern int ignore_spammer_multiplier;
extern int ignore_spammer_duration;

#endif
