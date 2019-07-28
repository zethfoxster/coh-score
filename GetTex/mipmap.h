#ifndef _MIPMAP_H
#define _MIPMAP_H

//GLint GETTEX_applyFade(GLint width, GLint height, GLenum format, GLenum type, const void *data,F32 close_mix,F32 far_mix );
int GETTEX_applyFade(unsigned char *dds_raw, F32 close_mix,F32 far_mix );

// End mkproto
#endif
