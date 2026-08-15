#ifndef SWAN_INPUT_H
#define SWAN_INPUT_H

char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
void editorProcessKeypress(void);

#endif /* SWAN_INPUT_H */
