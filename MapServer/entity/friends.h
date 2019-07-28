#ifndef FRIENDS_H
#define FRIENDS_H

void sendFriendsList( Entity *e, Packet *pak, int send_all );
void addFriend( Entity *e, char *name );
void removeFriend( Entity *e, char *name );
void displayFriends( Entity *e );
int isFriend( Entity * e, int dbid );

void updateAllFriendLists( void );
void updateFriendList( Entity *e);
void updateFriendTimer( void );

#endif