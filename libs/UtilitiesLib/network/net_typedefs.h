#ifndef NET_TYPEDEFS_H
#define NET_TYPEDEFS_H

typedef struct Packet Packet;
typedef struct NetLink NetLink;
typedef struct NetLinkList NetLinkList;

/***************************************************************
 * NetLink callbacks
 */

/**********************************************
 * NetLinkTransmitCallback
 *	This networking layer (netio + packet) is mostly designed to
 *	perform interactive network I/O processing.  The default behavior
 *	is to wait on all sockets for some input before giving up control
 *	to the rest of the application to operate on the inputs and any
 *	other processing.  
 *
 *	However, somethings it is desirable to allow the application to 
 *	write as fast as possible to outgoing links, as is the case for 
 *	file transfers.  In this case, waiting solely on incoming data
 *	is not desirable.  If this callback is specified on a NetLink,
 *	then the NetLinkList also waits on some buffer space to become 
 *	available for outgoing data on the socket.  It then calls
 *	this callback to allow the user to send data on the outgoing link.
 *
 *	Note that this option only makes sense on a TCP connection.
 *
 *	Note that setting this callback on a NetLinkList basically
 *	asks it to perfer sending over receving.
 */
typedef int NetLinkTransmitCallback(NetLink* link);

/**********************************************
 * NetPacketCallback
 *	This callback is used to process individual packets that have
 *	been received on a particular link.  This function is only passed
 *	to NetLinkMonitorXXX().
 */
typedef int NetPacketCallback(Packet *pak, int cmd, NetLink *link);
typedef void NetPacketSentCallback(NetLink* link, Packet* pak, int realPakSize, int result);

/*
 * NetLink callbacks
 ***************************************************************/

/***************************************************************
 * NetLinkList callbacks
 *	These callbacks are used to signal certain events so that they
 *	may be handled by higher level app logic.
 */

/**********************************************
 * NetLinkAllocCallback
 *	When a new incoming connection is found by a NetLinkList, it
 *	will create and initialize a new link, attaching the reqested
 *	amount of memory as user data.  It then calls this callback
 *	function, if available to let the user perform any additional
 *	initailizations for the link or the user data.
 *
 *	Note that this is the proper location to place any link customization
 *	code.
 */
typedef int NetLinkAllocCallback(NetLink* link);

/**********************************************
 * NetLinkDestroyCallback
 *	After a link has been disconnected, this callback is called
 *	first before the link is actually destroyed.  The user may
 *	perform any cleanup required by the attached user data.  This
 *	is also a good place to gather any link statistics.
 */
typedef int NetLinkDestroyCallback(NetLink* link);

/**********************************************
 * NetLinkCallback
 *	Used to iterate a NetLink collection.
 */
typedef void NetLinkCallback(NetLink* link);
/*
 * NetLinkList callbacks
 ***************************************************************/


/**********************************************
* PacketUserDataDestroyCallback
*	After a packet has been sent and is ready to be destroyed,
*	this callback is used to destroy anything in the packet's userdata
*	field.
*/
typedef void PacketUserDataDestroyCallback(Packet* pak);


#endif