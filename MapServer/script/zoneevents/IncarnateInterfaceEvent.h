#include "scriptutil.h"
#include "logcomm.h"

enum IncarnateInterfaceSlaveOrder
{
	IncarnateInterfaceSlaveOrder_None,
	IncarnateInterfaceSlaveOrder_Activate, // tell the slave to change from inactive to active
	IncarnateInterfaceSlaveOrder_Deactivate, // tell the slave to change from active to inactive
	IncarnateInterfaceSlaveOrder_Reactivate, // tell the slave: active->inactive->active
};