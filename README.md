# swan

A terminal text editor written from scratch in C - raw-mode terminal handling, incremental search and replace, multi-language syntax highlighting, and a clean vim-style exit, all with zero external dependencies.

> Built on top of the [kilo](https://viewsourcecode.org/snaptoken/kilo/) tutorial, then substantially extended: split into a ten modules, given syntax highlighting for three additional languages, and fixed to properly restore the terminal on exit. See [Origin & what's original](#origin--whats-original) for the honest breakdown.

## Features

- **Raw-mode terminal editing** - full control over input, no line-buffering, built directly on `termios`
- **Incremental search** (`Ctrl-F`) - live-updating match highlighting as you type, arrow keys to step forward/backward through results
- **Interactive search-and-replace** (`Ctrl-R`) - vim-style `y`/`n`/`a`/`q` prompt per match
- **Auto-indent** - new lines inherit the leading whitespace of the line they were split from
- **Syntax highlighting** for:
  - C
  - Python
  - x86-64 assembly (NASM syntax) - including a dedicated register color class
  - ARM assembly (GNU/UAL directives) - same register-aware highlighting
- **Vim-style clean exit** - uses the terminal's alternate screen buffer so your shell's scrollback is exactly as you left it after quitting, with a fallback clear-and-home for terminals that don't honor the alt-screen escape sequence
- **Dirty-buffer protection** - won't let you quit with unsaved changes without confirming
- **Zero dependencies** - no ncurses, no external libraries; just libc and raw ANSI escape sequences

## Platform support

Linux only, for now. Raw-mode terminal handling relies on POSIX `termios` and `ioctl(TIOCGWINSZ)`, which are broadly POSIX-standard, but this project has only been built and tested on Linux - no macOS or BSD testing has been done, and the Makefile assumes a Linux toolchain. It would likely need at least some verification (and possibly minor tweaks) to run cleanly elsewhere.

## Build & run

Requires only a C99 compiler and `make`. No libraries to install.

```bash
git clone https://github.com/cyber-rohanjha/textEditior
cd textEditior
make
./swan somefile.c
```

Run with no filename to open a blank buffer:

```bash
./swan
```

`make clean` removes build artifacts.

### Running `swan` from any directory

By default `./swan` only works while you're inside the project directory, since `./` is a relative path. To use `swan` like any other installed command, from anywhere on your system:

```bash
chmod +x swan          # make sure the binary has execute permission
sudo cp swan /usr/local/bin/swan
```

`/usr/local/bin` is already on most Linux distributions' `PATH` by default, so after that you can just run:

```bash
swan somefile.c
```

from any directory. Alternatively, if you'd rather not copy the binary into a system directory, add the project folder itself to your `PATH` instead (e.g. in `~/.bashrc` or `~/.zshrc`):

```bash
export PATH="$PATH:/path/to/textEditior"
```

Either way, `chmod +x` alone only grants the file permission to execute - it's `PATH` (or copying somewhere already on it) that actually makes the bare `swan` command resolve from any working directory.

## Keybindings

| Key | Action |
|---|---|
| `Ctrl-S` | Save |
| `Ctrl-Q` | Quit (press 3 times if there are unsaved changes) |
| `Ctrl-F` | Search (arrows to step through matches, Enter/Esc to exit) |
| `Ctrl-R` | Search and replace (`y`/`n`/`a`/`q` per match) |
| Arrow keys | Move cursor |
| `Page Up` / `Page Down` | Scroll by a screen |
| `Home` / `End` | Jump to start/end of line |
| `Backspace` / `Delete` | Delete character |
| `Enter` | New line (auto-indented) |

## Architecture

The editor is split into focused modules instead of one monolithic file, so navigating and extending the codebase doesn't mean scrolling through 1000+ lines to find the right function.

| File | Responsibility |
|---|---|
| `swan.h` | Shared types (`erow`, `editorConfig`, `editorSyntax`), enums, and the single `extern` global editor state |
| `main.c` | Owns the one true definition of the global state, `initEditor`, and the main loop |
| `terminal.c/h` | Raw mode setup/teardown, key reading, cursor position, window size, alt-screen buffer handling |
| `syntax.c/h` | The filetype database (`HLDB`) and the highlighter that walks each row and classifies characters |
| `row.c/h` | Low-level row operations: insert/delete/update, cursor-column ↔ render-column conversion |
| `editor_ops.c/h` | Higher-level editing operations: insert character, insert newline (with auto-indent), delete character |
| `file_io.c/h` | Opening files, saving, and serializing rows back to a single buffer |
| `find.c/h` | Incremental search and interactive search-and-replace |
| `abuf.c/h` | Small append-buffer helper used to batch terminal writes into a single `write()` call per frame |
| `output.c/h` | Screen rendering: scrolling, drawing rows/status bar/message bar, refreshing the screen |
| `input.c/h` | Reading and dispatching keypresses, the generic line-prompt used by save/search/replace |

Each `.c` file only includes the headers it actually needs - there's no circular dependency, everything funnels through `swan.h` for shared types.

## Syntax highlighting details

Highlighting uses three keyword classes rather than the stock two:

- **Instructions/keywords** (plain entries in the keyword list)
- **Types/builtins/directives** (suffixed with `|` - e.g. `"int|"`, `"section|"`)
- **Registers** (suffixed with `;`, used by the two assembly highlighters - e.g. `"rax;"`, `"r0;"`)

A couple of known simplifications, worth knowing if you extend this further:

- Python triple-quoted strings (`"""..."""`) are highlighted via the multi-line-comment machinery rather than as real strings, so docstrings render in the comment color, not the string color.
- ARM comments are highlighted as `@`-prefixed only (classic ARM/UAL style); GNU `as` also accepts `//` and `/* */`, which aren't currently recognized.

## How AI was used on this project

The core editor - following the kilo tutorial through to a complete, working single-file text editor - was written by hand, without AI assistance.

From there, [Claude](https://claude.ai) was used as a pair-programming/refactoring collaborator for the work that came after: splitting the single ~1000-line file into the ten-module architecture described above, implementing the Python/x86/ARM syntax highlighters, adding auto-indent and interactive search-and-replace, and diagnosing and fixing the terminal-restore and buffer-growth bugs. In each case the workflow was the same: describe the intended change or paste the current source, review the generated diff, rebuild, and test the actual behavior before accepting it - not "generate a text editor," but "help restructure and extend one that already exists and already works."

## Origin & what's original

The base editor - raw-mode terminal handling, the append-buffer rendering trick, basic row editing, and C syntax highlighting - follows [antirez's kilo](https://github.com/antirez/kilo) via the [Snaptoken tutorial](https://viewsourcecode.org/snaptoken/kilo/). Everything past that base is original work on this project:

- Split the single ~1000-line `kilo.c` into the ten-module architecture described above
- Added a third highlight class and wrote highlighters for Python, x86-64 (NASM), and ARM assembly from scratch
- Added auto-indent
- Added interactive search-and-replace
- Diagnosed and fixed the terminal-restore bug (the tutorial's version leaves the terminal in an inconsistent state on exit) by implementing alternate-screen-buffer handling with a clear-screen fallback for terminals that don't support it
- Fixed two pre-existing bugs from the base implementation: a buffer-growth no-op in the line-prompt input handler that caused a one-byte heap overflow past ~127 characters, and (during the module split) corrected a file-open bug that silently dropped the first line of any opened file

## Known limitations / future work

- Linux only - not tested on macOS or BSD (see [Platform support](#platform-support))
- Search-and-replace searches forward from the cursor to end-of-file only; it does not wrap around to the beginning
- No undo/redo yet
- No multi-buffer/tab support - one file per editor instance
- Single-line ARM comment support only (`@`, not `//` or `/* */`)

Planned next:

- [ ] Undo/redo via an operation log
- [ ] Multiple buffers
- [ ] Config file for tab width, colors, etc.
- [ ] Line numbers in the gutter

## License

This project is licensed under the BSD 2-Clause License - see [`LICENSE`](LICENSE) for full terms.

```
Portions Copyright (c) 2016, Salvatore Sanfilippo <antirez@gmail.com>
Protions Copyright (c) 2026, Rohan Jha <https://github.com/cyber-rohanjha>

## Acknowledgments

- [antirez](https://github.com/antirez) for the original [kilo](https://github.com/antirez/kilo) editor
- [Snaptoken](https://viewsourcecode.org/snaptoken/kilo/) for the "Build Your Own Text Editor" tutorial this project is built on
