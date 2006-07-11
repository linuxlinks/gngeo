
#ifndef _GLPROC_H_
#define _GLPROC_H_

#ifndef GP2X

#include "SDL_opengl.h"

void (*pglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (*pglClear)(GLbitfield mask);

void (*pglEnable)(GLenum cap);
void (*pglViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
void (*pglTexParameteri)(GLenum target, GLenum pname, GLint param);
void (*pglPixelStorei)(GLenum pname, GLint param);
void (*pglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (*pglBegin)(GLenum mode);
void (*pglEnd)();
void (*pglTexCoord2f)(GLfloat s, GLfloat t);
void (*pglVertex2f)(GLfloat s, GLfloat t);

#endif
#endif
