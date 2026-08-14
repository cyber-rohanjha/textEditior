#include "swan.h"
#include "syntax.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*** filetypes ***/

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typdef", "static", "enum", "class", "case",

    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL
};

char *PY_HL_extensions[] = {".py", NULL};
char *PY_HL_keywords[] = {
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is",
    "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
    "while", "with", "yield", "self",

    "print|", "len|", "range|", "input|", "open|", "int|", "str|",
    "float|", "list|", "dict|", "set|", "tuple|", "bool|", "super|",
    "enumerate|", "zip|", "map|", "filter|", "sorted|", "sum|", "min|",
    "max|", "abs|", "isinstance|", NULL
};

char *ASM_X86_HL_extensions[] = {".asm", ".nasm", NULL};
char *ASM_X86_HL_keywords[] = {
    "mov", "add", "sub", "mul", "imul", "div", "idiv", "inc", "dec",
    "neg", "and", "or", "xor", "not", "shl", "shr", "sal", "sar", "rol",
    "ror", "cmp", "test", "jmp", "je", "jne", "jz", "jnz", "jg", "jge",
    "jl", "jle", "ja", "jae", "jb", "jbe", "call", "ret", "push", "pop",
    "lea", "nop", "int", "syscall", "loop", "cwd", "cdq", "cqo", "cbw",
    "cwde", "movzx", "movsx", "pushf", "popf", "cli", "sti", "hlt", "in",
    "out", "xchg", "cmpxchg", "rep", "movs", "stos", "lods", "scas",
    "cmps",

    "section|", "global|", "extern|", "db|", "dw|", "dd|", "dq|",
    "resb|", "resw|", "resd|", "resq|", "equ|", "times|", "bits|",
    "org|", "align|", "struc|", "endstruc|",

    "rax;", "rbx;", "rcx;", "rdx;", "rsi;", "rdi;", "rbp;", "rsp;",
    "r8;", "r9;", "r10;", "r11;", "r12;", "r13;", "r14;", "r15;",
    "eax;", "ebx;", "ecx;", "edx;", "esi;", "edi;", "ebp;", "esp;",
    "ax;", "bx;", "cx;", "dx;", "si;", "di;", "bp;", "sp;",
    "al;", "bl;", "cl;", "dl;", "ah;", "bh;", "ch;", "dh;", NULL
};

char *ASM_ARM_HL_extensions[] = {".s", ".S", NULL};
char *ASM_ARM_HL_keywords[] = {
    "mov", "mvn", "add", "adc", "sub", "sbc", "rsb", "mul", "mla", "and",
    "orr", "eor", "bic", "cmp", "cmn", "tst", "teq", "b", "bl", "bx",
    "blx", "ldr", "str", "ldrb", "strb", "ldrh", "strh", "ldm", "stm",
    "push", "pop", "swi", "svc", "nop", "lsl", "lsr", "asr", "ror",
    "adr",

    ".text|", ".data|", ".bss|", ".global|", ".align|", ".word|",
    ".byte|", ".asciz|", ".ascii|", ".equ|", ".section|", ".thumb|",
    ".arm|", ".end|",

    "r0;", "r1;", "r2;", "r3;", "r4;", "r5;", "r6;", "r7;", "r8;",
    "r9;", "r10;", "r11;", "r12;", "r13;", "r14;", "r15;",
    "sp;", "lr;", "pc;", "fp;", "ip;", "cpsr;", "spsr;", NULL
};

struct editorSyntax HLDB[] = {
    {
        "c",
        C_HL_extensions,
        C_HL_keywords,
        "//", "/*", "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "python",
        PY_HL_extensions,
        PY_HL_keywords,
        "#", "\"\"\"", "\"\"\"",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "asm-x86",
        ASM_X86_HL_extensions,
        ASM_X86_HL_keywords,
        ";", NULL, NULL,
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "asm-arm",
        ASM_ARM_HL_extensions,
        ASM_ARM_HL_keywords,
        "@", NULL, NULL,
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
};

/*** syntax highlighting ***/

int is_separator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorUpdateSyntax(erow *row) {
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);

    if (E.syntax == NULL) return;

    char **keywords = E.syntax->keywords;

    char *scs = E.syntax->signleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    int scs_len = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_sep = 1;
    int in_string = 0;
    int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);

    int i = 0;
    while (i < row->rsize) {
        char c = row->render[i];
        unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

        if (scs_len && !in_string && !in_comment) {
            if (!strncmp(&row->render[i], scs, scs_len)) {
                memset(&row->hl[i], HL_COMMENT, row->rsize - i);
                break;
            }
        }

        if (mcs_len && mce_len && !in_string) {
            if (in_comment) {
                row->hl[i] = HL_MLCOMMENT;
                if (!strncmp(&row->render[i], mce, mce_len)) {
                    memset(&row->hl[i], HL_MLCOMMENT, mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_sep = 1;
                    continue;
                } else {
                    i++;
                    continue;
                }
            } else if (!strncmp(&row->render[i], mcs, mcs_len)) {
                memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
            if (in_string) {
                row->hl[i] = HL_STRING;
                if (c == '\\' && i + 1 < row->rsize) {
                    row->hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string) in_string = 0;
                i++;
                prev_sep = 1;
                continue;
            } else {
                if (c == '"' || c == '\'') {
                    in_string = c;
                    row->hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
            if (((isdigit(c)) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER)) {
                row->hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }

        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen - 1] == '|';
                int kw3 = keywords[j][klen - 1] == ';';
                if (kw2 || kw3) klen--;

                if (!strncmp(&row->render[i], keywords[j], klen) && is_separator(row->render[i + klen])) {
                    memset(&row->hl[i], kw3 ? HL_KEYWORD3 : (kw2 ? HL_KEYWORD2 : HL_KEYWORD1), klen);
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(c);
        i++;
    }

    int changed = (row->hl_open_comment != in_comment);
    row->hl_open_comment = in_comment;
    if (changed && row->idx + 1 < E.numrows) {
        editorUpdateSyntax(&E.row[row->idx + 1]);
    }
}

int editorSyntaxToColor(int hl) {
    switch (hl) {
        case HL_COMMENT:
        case HL_MLCOMMENT: return 36;
        case HL_KEYWORD1: return 33;
        case HL_KEYWORD2: return 32;
        case HL_KEYWORD3: return 94;
        case HL_STRING: return 35;
        case HL_NUMBER: return 31;
        case HL_MATCH: return 34;
        default: return 37;
    }
}

void editorSelectSyntaxHighlight(void) {
    E.syntax = NULL;
    if (E.filename == NULL) return;

    char *ext = strrchr(E.filename, '.');

    for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
        struct editorSyntax *s = &HLDB[j];
        unsigned int i = 0;
        while (s->filematch[i]) {
            int is_ext = (s->filematch[i][0] == '.');
            if ((is_ext && ext && !strcmp(ext, s->filematch[i])) || (!is_ext && strstr(E.filename, s->filematch[i]))) {
                E.syntax = s;

                int filerow;
                for (filerow = 0; filerow < E.numrows; filerow++) {
                    editorUpdateSyntax(&E.row[filerow]);
                }
                return;
            }
            i++;
        }
    }
}
