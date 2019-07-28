#include "uiUtilMenu.h"
#include "uiNPCCreationNLM.h"
#include "SuperAssert.h"
#include "earray.h"

static NonLinearMenu *NLM_NPCCreation = NULL;
extern NonLinearMenuElement NPC_costume_NLME;
extern NonLinearMenuElement NPC_profile_NLME;
NonLinearMenu * NLM_NPCCreationMenu()
{
	if (!NLM_NPCCreation)
	{
		NLM_NPCCreation = NLM_allocNonLinearMenu();
		if (!NLM_NPCCreation)	return NULL;
		NLM_pushNewNLMElement(NLM_NPCCreation, &NPC_costume_NLME);
		NLM_pushNewNLMElement(NLM_NPCCreation, &NPC_profile_NLME);
	}
	return NLM_NPCCreation;
}