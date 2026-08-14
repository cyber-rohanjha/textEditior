#include "swan.h"
#include "editor_ops.h"
#include "row.h"

#include <string.h>
#include <stdlib.h>

/*** editor operations ***/

void editorInsertChar(int c) {
    if (E.cy == E.numrows) {
        editorInsertRow(E.numrows, "", 0);
    }
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void editorInsertNewline(void) {
    if (E.cx == 0) {
        editorInsertRow(E.cy, "", 0);
        E.cy++;
        E.cx = 0;
        return;
    }
    
    erow *row = &E.row[E.cy];

    /* auto-indent: measure the current line's leading whitespace and
        carry it into the newline, so one doesn't have to re-tab/re-space
        every line by hand */
    int indent_len = 0;
    while (indent_len < row->size && (row->chars[indent_len] == ' ' || row->chars[indent_len] == '\t')) {
        indent_len++;
    }

    /* don't indent past the split point if one hits 'Enter' inside the leading whitespace itself */
    if (indent_len > E.cx) indent_len = E.cx;

    int tail_len = row->size - E.cx;
    char *newline_buf = malloc(indent_len + tail_len);
    memcpy(newline_buf, row->chars, indent_len);
    memcpy(newline_buf + indent_len, &row->chars[E.cx], tail_len);

    editorInsertRow(E.cy + 1, newline_buf, indent_len + tail_len);
    free(newline_buf);

    /* editorInsertRow may have re-allocated E.row, so re-fetch the pointer
        before truncating the original line */
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);

    E.cy++;
    E.cx = indent_len;
}

void editorDelChar(void) {
    if (E.cy == E.numrows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow *row = &E.row[E.cy];
    if (E.cx > 0) {
        editorRowDelChar(row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size;
        editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}
