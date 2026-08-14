#ifndef SWAN_SYNTAX_H
#define SWAN_SYNTAX_H

#include "swan.h"

extern struct editorSyntax HLDB[];
#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight(void);

#endif /* SWAN_SYNTAX_H */
