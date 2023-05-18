# Experimental Text Editor (WIP)



## Building
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
| Ctrl+ScrollWheel | Zoom in/out |
| Ctrl+0 | Reset zoom | 

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

## Credits


## Known Issues
 - Builing with `-Doptimize=ReleaseFast` causes the compiler to hang while analyzing `src/fs.zig`. Not sure why, but it's most likely a compiler bug due to regression in the C backend.
 - Crashing when editing files. This is probably due to a buffer overrun caused by the Zig->C handoff in `src/fs.zig` and `Editor_OpenFile` (null terminator woes I'm sure). Will be fixed in a rewrite.
