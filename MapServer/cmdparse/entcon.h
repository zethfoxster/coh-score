/* File entcon
 *	Provide a way for entities to receive and process commands directly from clients.
 *
 *	Allows clients to issue commands to entities.  This is mostly created for debugging purposes.
 *	The function extracts the entity ID and command, then passes the parameters, if any, to the
 *	correct command handler.
 *
 *	Command format:
 *		entcontrol <entID> <command> [params]
 *		The command parameters should be deliminated by space.  All parameter types are "valid"
 *		as long as the command handler accepts them.
 *
 */
#ifndef ENTCON_H
#define ENTCON_H

typedef struct ClientLink ClientLink;
typedef struct Entity Entity;

typedef void (*CommandHandler)(Entity* ent, char* params);

int entconSetCommandAccess(char* commandName, int accesslevel);
// add command handler enabling and disabling functions?

void entControl(ClientLink* client, const char* command);

#endif