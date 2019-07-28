/* Ripped right out of VC CRT library.
 * Added ability to pass in a context to be passed to the compare callback function.
 *
 */

#ifndef CONTEXTQSORT_H
#define CONTEXTQSORT_H

void __cdecl contextQsort (
						   void *base,
						   size_t num,
						   size_t width,
						   const void* context,
						   int (__cdecl *comp)(const void* item1, const void* item2, const void* context)
						   );

#endif