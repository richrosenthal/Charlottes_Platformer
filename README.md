# Charlotte's Platformer

A small, playable Game Boy platformer starter built with
[GBDK-2020](https://gbdk.org/). It includes a tile-based level, a two-tile player
sprite, acceleration, gravity, jumping, and solid platform collisions.

## Play

- D-pad left/right: move
- A: jump

## Build

Install or extract GBDK-2020, then either put `lcc` on your `PATH` or point
`GBDK_HOME` at the extracted GBDK directory:

```sh
make GBDK_HOME=/path/to/gbdk
```

The ROM is written to `build/charlottes_platformer.gb`.

To launch it with an installed emulator:

```sh
make run GBDK_HOME=/path/to/gbdk EMULATOR=sameboy
```

Use `make clean` to remove generated build output.

## Project layout

```text
.
├── Makefile       Build and emulator targets
└── src/
    └── main.c     Level, graphics, input, physics, and game loop
```

The graphics are defined in C so the starter has no asset-generation dependency.
As the game grows, PNG art can be converted with GBDK's `png2asset` utility.
