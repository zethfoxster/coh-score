

Title: About our Ogg/Vorbis setup

Creator: Martin Simpson


The "./include" directory is a concatenation of "./src/ogg/include" and "./src/vorbis/include" directories.

The "src.zip" file is the ogg/vorbis source tree.  Unzip it in-place (should create a "./src" directory).

The changes made to the default distribution are as follows:

- Allocation functions call our own allocators.
  They are called "cryptic_ogg_*" (i.e. cryptic_ogg_malloc, _free, etc) and are in ogg_decode.c in the game project.
  This change can be found in "./src/ogg/include/ogg/os_types.h"

- The projects ogg_static, vorbis_static, and vorbisfile_static output lib and pdb files to "./lib".

- The project files are now set up to build for the xbox target as well (Ben zeigler, 4/6/06). You may need to change the default target before recompiling