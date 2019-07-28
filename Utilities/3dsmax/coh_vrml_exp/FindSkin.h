#ifndef __FINDSKIN__H
#define __FINDSKIN__H

#include "max.h"
#include "modstack.h"
#include "phyexp.h"
#include "ISkin.h"

Modifier* FindPhysiqueModifier (INode* nodePtr)
{
	// Get object from node. Abort if no object.
	if (!nodePtr) return NULL;

	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr ) return NULL;
	
	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
			// Yes -> Cast.
			IDerivedObject* DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);				
			
		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;

		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			// Get current modifier.
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			// Is this Physique ?
			if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
			{
				// is this the correct Physique Modifier based on index?
				return ModifierPtr;
			}
			ModStackIndex++;
		}

		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	// Not found.
	return NULL;
}


Modifier* FindSkinModifier (INode* node)
{
	if (!node) return NULL;
	
	// Get object from node. Abort if no object.
	Object* pObj = node->GetObjectRef();
	

	if (!pObj) return NULL;

	// Is derived object ?
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject*) pObj;

		// Iterate over all entries of the modifier stack.
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get current modifier.
			Modifier* mod = pDerObj->GetModifier(Idx);

			// Is this Skin ?
			if (mod->ClassID() == SKIN_CLASSID )
			{
				// Yes -> Exit.
				return mod;
			}

			// Next modifier stack entry.
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}

	// Not found.
	return NULL;
}

#endif // __FINDSKIN__H