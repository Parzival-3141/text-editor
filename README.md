# Experimental Text Editor (WIP)
This project is what happens when you ask, *"What if my text editor had a camera?"*.

[text-editor-demo.webm](https://github.com/Parzival-3141/text-editor/assets/29632054/3ecd6ae9-8374-4e35-91cb-2a00d3a0b2ba)

## Features
 - Basic text editing
 - Camera that smoothly follows the cursor
 - File Browser that displays directories in 2D space
 - [SDF text rendering](https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf)

Though this project is mainly in C, it uses Zig as it's compiler/build system. I really love this feature of Zig, and it allows doing some neat stuff *(check out the [filesystem](src/fs.zig) [code](src/fs.h))*!

**Note:** I'm still in the exploratory phase of development right now, so the code is pretty messy. The project will definitely be rewritten in the future (probably in Zig!).

## Building
```sh
git clone --recursive https://github.com/Parzival-3141/text-editor.git
```
You'll need the self-hosted Zig compiler.
```sh
zig version
0.11.0-dev.3180+b754068fb
```
Then just call `zig build run` in the project root.

## Controls
| | |
| --- | --- |
| Ctrl+ScrollWheel | Zoom in/out | Zoom in/out |
| Ctrl+0 | Reset zoom | Reset zoom |

### File Browser 
| Input | Action |
| --- | --- |
| Arrow Keys | Moves the cursor |
| Escape | Goes up a directory |
| Enter | Opens the directory or file under the cursor |

### Text Editing
| Input | Action |
| --- | --- |
| Arrow Keys | Moves the cursor |
| Ctrl+Arrow Keys | Moves the cursor by word |
| Escape | Closes the text box |
| Enter | Newline |
| Backspace | Remove character to the left |
| Delete | Remove character to the right |
| Home | Move cursor to line start |
| End | Move cursor to line end |

## Built With
*Dependencies are included in the repo and are built from source, so you shouldn't have any issues.*
 - [SDL2](https://github.com/libsdl-org/SDL)
 - [Freetype](https://github.com/hexops/freetype)
 - [glad](https://github.com/Dav1dde/glad)
 - [cglm](https://github.com/recp/cglm)
 - Icons - https://fonts.google.com/icons
 - [Hack font](https://github.com/source-foundry/Hack)

## Known Issues
 - Builing with `-Doptimize=ReleaseFast` causes the compiler to hang while analyzing `src/fs.zig`. Not sure why, but it's most likely a compiler bug due to regression in the C backend.
