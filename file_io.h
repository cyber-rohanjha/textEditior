#ifndef SWAN_FILE_IO_H
#define SWAN_FILE_IO_H

char *editorRowToString(int *buflen);
void editorOpen(char *filename);
void editorSave(void);

#endif /* SWAN_FILE_IO_H */
