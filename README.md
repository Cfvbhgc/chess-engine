# Rook

A basic UCI chess engine written in C++17.

## Features

- Array-based board with bitboard occupancy helpers
- Full legal move generation (castling, en passant, promotions)
- Alpha-beta search with quiescence
- Piece-square table evaluation
- UCI protocol support

## Build

```
mkdir build && cd build
cmake ..
make
```

Or directly:

```
g++ -std=c++17 -O2 src/*.cpp -Isrc -o rook
```

## Usage

Run `./rook` (or `./build/rook`) and communicate via UCI:

```
uci
isready
position startpos moves e2e4 e7e5
go depth 5
quit
```

Works with any UCI-compatible GUI (Arena, Cute Chess, etc).

## Search

Default depth is 5 plies. The engine adjusts depth based on available time when `wtime`/`btime` are provided. Quiescence search handles tactical positions at leaf nodes.
