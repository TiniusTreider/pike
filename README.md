# pike

UCI compatible chess engine written entirely in POSIX C23 with no dependencies.

## usage

`pike` runs UCI mode.

## build

Run `make` in project root on Unix systems to compile, and `make clean` to remove build files and compiled binary. Pike is not yet Windows-compatible.

## UCI

Pike has full UCI support and some extra commands for debugging or ease-of-use.


`go perft <depth>` runs a perft divide on the current position with the given depth.

`position kiwipete` buildt-in position, good for debugging move generation errors.

`d` prints the current position with ANSI-colored Unicode. Make sure your terminal supports both.

