---
type: API
title: Grid
description: Constants, RaycastGrid: every method, the hit bytes, the grid walk and the picture, step by step, and what it refuses.
resource: stubs/
tags: [raycast, grid, dda, c]
status: draft
generated: { by: claude-fable/5.2, at: 2026-11-13T10:00:00Z }
sources:
  - id: stubs
    resource: stubs/
    title: raycast.stub.php, RaycastGrid.stub.php
  - id: core
    resource: src/core.h
    title: The casting core
---

# Overview

One class, `RaycastGrid`, and eight constants.[^stubs] `src/core.{h,c}` is the arithmetic with no Zend in it; `src/RaycastGrid.c` checks arguments and owns the strings.[^core]

# The grid

`width` x `height` cells, one byte each, rows top first. 0 is open; 1..255 is a wall and the byte is its id. x grows east, y grows south, in cells: (0.5, 0.5) is the middle of the first cell. Sides are 1..`RAYCAST_MAX_SIDE` (65535). The object keeps a reference to the cells string; it is immutable.

# Constants

| Constant | Value | Meaning |
|---|---|---|
| `RAYCAST_HIT_BYTES` | 22 | one hit record |
| `RAYCAST_FACE_WEST` | 0 | the ray was travelling east |
| `RAYCAST_FACE_EAST` | 1 | west |
| `RAYCAST_FACE_NORTH` | 2 | south |
| `RAYCAST_FACE_SOUTH` | 3 | north |
| `RAYCAST_MAX_SIDE` | 65535 | a grid's side, a texture's side, a fan's columns |
| `RAYCAST_MAX_VIEW` | 16384 | a view's side |
| `RAYCAST_LIMIT` | 2^30 | the largest magnitude of a position |

`RAYCAST_MAX_LINE` (2^24, the tallest wall line before it is cut to the view) is internal to `core.h`.

# Hit bytes

22 bytes, little-endian: `distance` (float64), `along` (float64), `x` (uint16), `y` (uint16), `wall` (uint8), `face` (uint8). PHP: `unpack('edistance/ealong/vx/vy/Cwall/Cface', $record)`. A miss is 22 zero bytes; `wall === 0` tells it.

* `distance`: the multiple of the ray's vector at the face. A unit vector gives cells along the ray. In a fan with a unit direction and a plane at right angles, it is the distance along the direction, so a flat wall has one distance and a picture has no fisheye.
* `along`: where on the face, 0 <= along < 1, measured toward +y on a west or east face and toward +x on a north or south one.

# The walk (`raycast_march`)

From (x, y) along (dx, dy), at most `reach` multiples:

1. Outside the grid (`x < 0`, `y < 0`, `x >= width`, `y >= height`): miss.
2. `delta_x = dx == 0 ? INF : fabs(1 / dx)`, the same for y. Both infinite: miss.
3. `cell = floor(x), floor(y)`; `step = dx < 0 ? -1 : 1`, the same for y.
4. `next_x = dx < 0 ? (x - cell_x) * delta_x : (cell_x + 1 - x) * delta_x`, INF when `delta_x` is; the same for y.
5. Loop. If `next_x < next_y`: `distance = next_x`, `next_x += delta_x`, `cell_x += step_x`; else the same in y. **A tie steps in y.**
6. `distance > reach`, or the cell is outside the grid: miss.
7. The cell's byte is 0: back to 5.
8. Hit. `where = crossed x ? y + distance * dy : x + distance * dx`; `along = where - floor(where)`. Face: crossed x and stepping east is WEST, west is EAST; crossed y and stepping south is NORTH, north is SOUTH.

The cell the ray starts in is never looked at. An eye exactly on a boundary is in the cell at or after it; a ray going back across that boundary crosses it at distance 0. The loop ends within width + height steps.

# `fan` and the column ray

Column `i` of `n` looks along `dir + plane * across`, `across = (2i + 1) / n - 1`: through the centre of its column. A column whose ray is (0, 0) is a miss.

# `view`

RGBA8, top row first. `horizon = height / 2` (whole division): rows above it are the ceiling, the rest the floor. For each column that hits:

1. `tall = distance > 0 ? height / distance : 2^24`, held to 2^24; `line = (int) tall`.
2. `start = horizon - line / 2` (whole division); rows `max(start, 0)` to `min(start + line, height)` exclusive.
3. `dim = shade` on a north or south face, else 255.
4. A colour: red, green and blue each `c * dim / 255` (whole division), alpha as given.
5. A texture: `texel_x = (int) (along * texture_width)`, held to the last column, and mirrored (`width - 1 - texel_x`) on an east or north face so the texture reads left to right as the face is seen. Row `r` reads texture row `(r - start) * texture_height / line` (64-bit whole division). Red, green and blue are dimmed as in 4.

Walls are `array<int, int|array{string, int, int}>` keyed by wall id 1..255: a colour `0xRRGGBBAA` or `[RGBA8 bytes, width, height]`. The whole table is checked before a ray is cast. A ray that reaches a wall with no entry is `ValueError("Wall N has no colour or texture.")`; no string is answered.

# Refusals

All `ValueError`, before any ray:

* construct: a side outside 1..65535; cells not width * height bytes. A second `__construct` is `Error`.
* a position not finite or past ±2^30; a vector not finite; a ray or direction of (0, 0); a reach not above 0 (INF is allowed, NAN is not).
* `fan`: columns outside 1..65535. `view`: a side outside 1..16384; ceiling or floor outside 0..0xFFFFFFFF; shade outside 0..255; a wall key that is not an int 1..255; a wall that is not a colour or a three-item list of string, int, int; a texture side outside 1..65535; a texture whose bytes are not width * height * 4.

# Why no angles

`cast`, `fan` and `view` take vectors. `cos`, `sin` and `tan` are not correctly rounded and differ between C libraries, so an angle taken in C would let two builds, or C and PHP, differ in the last bit. The caller works the vectors once.

[^stubs]: `stubs/raycast.stub.php`, `stubs/RaycastGrid.stub.php`.
[^core]: `src/core.h` documents each function next to its declaration.

## viewInto

`viewInto(x, y, dirX, dirY, planeX, planeY, width, height, walls, ceiling, floor, address, stride, shade = 255, reach = INF): void` writes `view()`'s picture into caller memory: the first pixel at `address`, rows `stride` bytes apart, `stride >= width * 4`. Bytes of a row past `width * 4` are not touched. The same bytes as `view()` for the same input (`raycast_view()` is `raycast_view_rows()` at `stride = width * 4`).

It refuses what `view()` refuses, plus `address == 0` and a `stride` under `width * 4`, all before writing. A wall with no entry is found by `raycast_view_missing()`, a pass of the rays alone, so nothing is half painted; `view()` keeps its single pass. The address is not checked beyond non-zero. Tests take an address from ext-fb and skip without it.

A last argument, `order`, says where a pixel's four bytes go: `RAYCAST_ORDER_RGBA` (0, the default), `_BGRA` (1), `_ARGB` (2), `_ABGR` (3). `view()` is always RGBA.
