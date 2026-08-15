#ifndef SWAN_ABUF_H
#define SWAN_ABUF_H

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif /* SWAN_ABUF_H */
