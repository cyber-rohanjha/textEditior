#ifndef SWAN_TERMINAL_H
#define SWAN_TERMINAL_H

void die(const char *s);
void disableRawMode(void);
void enableRawMode(void);
int editorReadKey(void);
int getCursorPosition(int *rows, int *clos);
int getWindowSize(int *rows, int *cols);

#endif /* SWAN_TERMINAL_H */
