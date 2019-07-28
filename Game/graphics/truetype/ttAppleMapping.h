/****************************************************************************************
 * "Apple Roman" to Unicode mapping
 *	
 *	This mapping is needed so it is possible to request for the correct glyphs from fonts
 *	that only have "Apple Roman" encoding character maps, such as "RedCircle".
 *
 *	It looks like most of the characters in "Apple Roman" matches their Unicode counterparts
 *	without any mapping.  This mapping may not be needed.
 *
 */
#ifndef TTAPPLEMAPPING_H
#define TTAPPLEMAPPING_H

void initAppleAndUnicodeMap();
unsigned short appleToUnicode(char appleCharacter);
char unicodeToApple(unsigned short unicodeCharacter);

#endif