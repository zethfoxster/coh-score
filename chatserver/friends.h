#ifndef _FRIENDS_H
#define _FRIENDS_H

void friendStatus(User *user,char *status);
void friendRequest(User *user,char *friend_name);
void friendRemove(User *user,char *friend_name);
void friendList(User *user);

void userInvisible(User *user);
void userVisible(User *user);

void userFriendHide(User *user);
void userFriendUnHide(User *user);

void userTellHide(User *user);
void userTellUnHide(User *user);

void userGmailFriendOnlySet(User *user);
void userGmailFriendOnlyUnset(User *user);

void userClearMessageHash(User *user);
void friendRequestSendAll(User *user);

#endif
