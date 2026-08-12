# Kilo

A small terminal-based text editor written in C.

## Build

Compile the editor with:

```sh
gcc -o kilo kilo.c -std=c99 -Wall -Wextra -pedantic
```

## Run

Open a file with:

```sh
./kilo path/to/file.txt
```

If no file is provided, Kilo opens with an empty buffer.

## Controls

- `Ctrl-Q`: quit
- `Arrow keys`: move the cursor
- `Home` / `End`: go to beginning/end of line
- `Page Up` / `Page Down`: scroll through the file

## Notes

This is a minimal demo editor inspired by the "Build Your Own Text Editor" tutorial.
