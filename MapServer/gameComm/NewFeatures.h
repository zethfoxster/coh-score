#ifndef NEW_FEATURES_H
#define NEW_FEATURES_H

typedef struct NewFeature
{
	int id;
	const char * pchDescription;
	const char * pchCommand;
} NewFeature;

typedef struct NewFeatureList
{
	const NewFeature ** list;
} NewFeatureList;

extern SHARED_MEMORY NewFeatureList g_NewFeatureList;
typedef struct ClientLink ClientLink;

int load_NewFeatures(SHARED_MEMORY_PARAM NewFeatureList * pNewFeatureList, const char * pchDefName);
const char * runNewFeatureCommand(ClientLink * client, int id);

#endif // NEW_FEATURES_H