#include "swan.h"
#include "find.h"
#include "row.h"
#include "input.h"
#include "output.h"
#include "terminal.h"

#include <stdlib.h>
#include <string.h>

/*** find ***/

void editorFindCallback(char *query, int key) {
    static int last_match = -1;
    static int direction = 1;

    static int saved_hl_line;
    static char *saved_hl = NULL;

    if (saved_hl) {
        memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
        free(saved_hl);
        saved_hl = NULL;
    }

    if (key == '\r' || key == '\x1b') {
        last_match = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1) direction = 1;
    int current = last_match;
    int i;
    for (i = 0; i < E.numrows; i++) {
        current += direction;
        if (current == -1) current = E.numrows - 1;
        else if (current == E.numrows) current = 0;

        erow *row = &E.row[current];
        char *match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = editorRowRxToCx(row, match - row->render);
            E.rowoff = E.numrows;

            saved_hl_line = current;
            saved_hl = malloc(row->rsize);
            memcpy(saved_hl, row->hl, row->rsize);
            memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }
}

void editorFind(void) {
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)", editorFindCallback);

    if (query) {
        free(query);
    } else {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}

static void editorReplaceOne(erow *row, int match_rx, const char *query, const char *replacement) {
    int cx_start = editorRowRxToCx(row, match_rx);
    int qlen = (int)strlen(query);
    int rlen = (int)strlen(replacement);

    int newsize = row->size - qlen + rlen;
    char *newchars = malloc(newsize + 1);
    memcpy(newchars, row->chars, cx_start);
    memcpy(newchars + cx_start, replacement, rlen);
    memcpy(newchars + cx_start + rlen, &row->chars[cx_start + qlen], row->size - cx_start - qlen);

    newchars[newsize] = '\0';

    free(row->chars);
    row->chars = newchars;
    row->size = newsize;
    editorUpdateRow(row);
    E.dirty++;
}

void editorFindAndReplace(void) {
    char *query = editorPrompt("Replace: %s (ECS to cancel)", NULL);
    if (!query) return;
    if (query[0] == '\0') {
        free(query);
        return;
    }

    char *replacement = editorPrompt("Replace with: %s (ESC to cancel)", NULL);
    if (!replacement) {
        free(query);
        return;
    }

    int qlen = (int)strlen(query);
    int replace_all = 0;
    int count = 0;

    int cur_row = E.cy;
    int search_rx = editorRowCxToRx(&E.row[E.cy], E.cx);

    while (cur_row < E.numrows) {
        erow *row = &E.row[cur_row];
        char *match = strstr(row->render + search_rx, query);

        if (!match) {
            cur_row++;
            search_rx = 0;
            continue;
        }

        int match_rx = (int)(match - row->render);

        E.cy = cur_row;
        E.cx = editorRowRxToCx(row, match_rx);
        E.rowoff = E.numrows;

        unsigned char *saved_hl = malloc(row->rsize);
        memcpy(saved_hl, row->hl, row->rsize);
        memset(&row->hl[match_rx], HL_MATCH, qlen);

        int do_replace = replace_all;
        int quit = 0;

        if (!replace_all) {
            editorSetStatusMessage("Replace with \"%s\"? y/n/a(ll)/q(quit)", replacement);
            editorRefreshScreen();
            int key = editorReadKey();
            if (key = 'y') {
                do_replace = 1;
            } else if (key == 'a') {
                do_replace = 1;
                replace_all = 1;
            } else if (key == 'q' || key == '\x1b') {
                quit = 1;
            }
        }

        memcpy(row->hl, saved_hl, row->rsize);
        free(saved_hl);

        if (quit) break;

        if (do_replace) {
            editorReplaceOne(row, match_rx, query, replacement);
            count++;
            search_rx = match_rx + (int)strlen(replacement);
        } else {
            search_rx = match_rx + qlen;
        }
    }

    editorSetStatusMessage("Replaced %d occurrence%s", count, count == 1 ? "" : "s");

    free(query);
    free(replacement);
}