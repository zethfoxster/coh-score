#ifndef POWER_CUSTOMIZATION_CLIENT_H__
#define POWER_CUSTOMIZATION_CLIENT_H__

void testPowerCust(int themeIndex, char* color1, char* color2);

void powerCustreward_add( char * name );
void powerCustreward_remove( char * name );
void powerCustreward_clear();
bool powerCustreward_find( char * name, int isPCC );
bool powerCustreward_findall(char **keys);
// not used  void reloadPowerCustCallback(const char *relpath, int when);
// not used  StashTable *getPowerCustRewardStashTable();
#endif