#ifndef SWAN_OUTPUT_H
#define SWAN_OUTPUT_H

#include "abuf.h"

void editorScroll(void);
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen(void);
void editorSetStatusMessage(const char *fmt, ...);

#endif /* SWAN_OUTPUT_H */
