<div align="center" id="header">

<h1>ext-raycast</h1>

<h3>Rays through a grid of cells in C, for PHP 8.4+</h3>

[![CI](https://img.shields.io/github/actions/workflow/status/php-io-extensions/raycast/ci.yml?style=flat-square&label=CI)](https://github.com/php-io-extensions/raycast/actions/workflows/ci.yml)
[![Packagist](https://img.shields.io/packagist/v/php-io-extensions/raycast?style=flat-square)](https://packagist.org/packages/php-io-extensions/raycast)
[![PHP](https://img.shields.io/packagist/dependency-v/php-io-extensions/raycast/php?style=flat-square&label=PHP)](https://packagist.org/packages/php-io-extensions/raycast)
[![License](https://img.shields.io/github/license/php-io-extensions/raycast?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-linux_%7C_macos-blue?style=flat-square&logo=linux&logoColor=white)](#requirements)
[![OKF](https://img.shields.io/badge/OKF-0.2-6f42c1?style=flat-square&label=knowledge%20bundle)](.okf/index.md)
[![PIE](https://img.shields.io/badge/PIE-installable-4f5b93?style=flat-square&logo=php&logoColor=white)](https://github.com/php/pie)

</div>

A map made of square cells is the oldest way to build a world a player can walk through: a maze, a dungeon, a floor plan. To show it from inside, or to ask whether one thing in it can see another, you send a ray out across the cells and find the first wall it meets. Send one ray for every column of the screen, draw each wall taller the nearer it is, and the flat map becomes a first-person view. That is ray casting, the technique behind the first-person games of the early 1990s.

In PHP that is a loop inside a loop. Every ray steps from cell to cell with a handful of float operations a step, and a picture then writes a line of pixels for every column, four bytes at a time. A 640 by 400 view is a few hundred rays and a quarter of a million pixel writes through the interpreter, every frame.

<b>The only problem?</b> PHP has nothing that walks a grid for you and stops there. The loop is yours to write and yours to make fast.

<b>ext-raycast</b> runs that loop in C. It has no dependency, no window, no game rules and no opinions: a grid of bytes goes in, hit records or RGBA8 pixels come out, and the same input answers the same bytes on every platform.

## Basic usage

Make a grid, cast a ray, read what it hit:

```php
// A room 5 cells square: walls all round (1), a door in the east wall (2).
$grid = new RaycastGrid(5, 5,
    "\1\1\1\1\1".
    "\1\0\0\0\1".
    "\1\0\0\0\2".
    "\1\0\0\0\1".
    "\1\1\1\1\1");

// One ray from the middle of the room, looking east.
$hit = unpack('edistance/ealong/vx/vy/Cwall/Cface', $grid->cast(2.5, 2.5, 1.0, 0.0));
```

```console
[distance] => 1.5   // cells from the eye to the wall
[along]    => 0.5   // halfway along the face
[x] => 4, [y] => 2  // the cell
[wall]     => 2     // the door
[face]     => 0     // RAYCAST_FACE_WEST: its west side
```

A cell is one byte: 0 is open, 1 to 255 is a wall and the byte is its id. x grows east and y grows south, in cells, so (2.5, 2.5) is the middle of cell (2, 2).

### Examples

#### See the room

`view()` casts one ray for each column and answers the picture as RGBA8 bytes, top row first. Give each wall id a colour:

```php
// The camera looks east. Its plane is 0.66 long: about 66 degrees from edge to edge.
$pixels = $grid->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.66, 320, 200,
    [1 => 0x808890FF, 2 => 0x9A6A3CFF],   // walls by id, 0xRRGGBBAA
    0x20242CFF,                           // ceiling
    0x3A3A3AFF,                           // floor
    170);                                 // north and south faces at 170/255 brightness

strlen($pixels); // 256000: 320 * 200 * 4
```

The bytes go into anything that takes RGBA8: an image encoder, a texture upload, a pixel buffer.

#### Turn and choose a field of view

The methods take vectors, not angles, so work them once from the angle you want. The direction is a unit vector; the plane is at right angles to it and `tan(fov / 2)` long:

```php
$turn = M_PI / 6;                // 30 degrees toward south
$half = tan(deg2rad(60) / 2);    // a 60 degree field of view

$fan = $grid->fan(2.5, 2.5, cos($turn), sin($turn), -sin($turn) * $half, cos($turn) * $half, 5);

foreach (str_split($fan, RAYCAST_HIT_BYTES) as $column => $record) {
    $hit = unpack('edistance/ealong/vx/vy/Cwall/Cface', $record);
    printf("%d: wall %d at (%d, %d), %.3f away\n", $column, $hit['wall'], $hit['x'], $hit['y'], $hit['distance']);
}
```

```console
0: wall 2 at (4, 2), 1.367 away
1: wall 2 at (4, 2), 1.528 away
2: wall 1 at (4, 3), 1.732 away
3: wall 1 at (4, 3), 1.999 away
4: wall 1 at (3, 4), 1.667 away
```

`fan()` answers the hits without drawing them, for when you draw the walls yourself or only need to know what is in view.

#### Paint a wall with a texture

A wall entry can be a picture in place of a colour: its RGBA8 bytes, its width and its height. It is stretched over each face and sampled nearest:

```php
$brick = [$rgba8, 64, 64];   // 64 * 64 * 4 bytes, top row first

$pixels = $grid->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.66, 320, 200, [1 => $brick, 2 => 0x9A6A3CFF], 0x20242CFF, 0x3A3A3AFF);
```

#### Ask whether something is in sight

A reach stops a ray short. A ray that hits nothing within it answers 22 zero bytes, so `wall` is 0:

```php
$dx = $target_x - $x;
$dy = $target_y - $y;

// The vector runs from the eye to the target, so a reach of 1 is exactly the target.
$blocked = unpack('Cwall', $grid->cast($x, $y, $dx, $dy, 1.0), 20)['wall'] !== 0;
```

## Requirements

- PHP 8.4 or newer, NTS or ZTS, with `phpize` and `php-config`.
- A C compiler. No library to install.
- Linux or macOS.

```bash
# Debian, Ubuntu, Raspberry Pi OS
sudo apt install php-dev build-essential

# macOS: Xcode Command Line Tools, and Homebrew's PHP ships phpize and php-config
xcode-select --install
```

## Installation

### With PHP PIE (recommended)

```bash
pie install php-io-extensions/raycast
```

### Manually (install script)

```bash
git clone https://github.com/php-io-extensions/raycast.git
cd raycast
./install-macos.sh            # Homebrew php@8.4 and php@8.4-zts, or pass PHP binaries
./install-debian-trixie.sh    # Debian, Ubuntu, Raspberry Pi OS
```

### Manually (phpize)

```bash
phpize
./configure --enable-raycast
make -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)"
sudo make install
echo 'extension=raycast' | sudo tee "$(php -r 'echo PHP_CONFIG_FILE_SCAN_DIR;')/30-raycast.ini"
```

### Verify

```bash
php --ri raycast
```

```console
raycast

raycast support => enabled
Version => 0.11.0
```

## API

Everything a method refuses is a `ValueError` that names what was wrong, raised before a ray is cast.

### Constants

| Constant | Value | Meaning |
|---|---|---|
| `RAYCAST_HIT_BYTES` | 22 | the size of one hit record |
| `RAYCAST_FACE_WEST`, `_EAST`, `_NORTH`, `_SOUTH` | 0, 1, 2, 3 | the side of the wall cell a ray struck |
| `RAYCAST_MAX_SIDE` | 65535 | the longest side of a grid or a texture, and the most columns in a fan |
| `RAYCAST_MAX_VIEW` | 16384 | the longest side of a view |
| `RAYCAST_LIMIT` | 2³⁰ | the largest magnitude of a position |

### Hits

A hit is 22 bytes, little-endian: `distance` (float64), `along` (float64), `x` (uint16), `y` (uint16), `wall` (uint8), `face` (uint8). Read one with `unpack('edistance/ealong/vx/vy/Cwall/Cface', $record)`. A ray that hit nothing is 22 zero bytes.

- `distance` is the multiple of the ray's vector at the wall's face. With a unit vector it is in cells. In a fan it is measured along the camera's direction, not along each ray, so a flat wall has one distance and a picture has no fisheye bulge.
- `along` is where on the face the ray landed, from 0 up to but not 1, measured toward +y on a west or east face and toward +x on a north or south one.

### Rays

A ray walks from cell to cell and stops at the first wall it enters. The cell it starts in is not looked at. A ray that starts outside the grid, leaves it, or passes its reach hits nothing. Where a ray passes exactly through a corner it steps in y first.

### RaycastGrid

#### `__construct(int $width, int $height, string $cells)`

Both sides 1 to `RAYCAST_MAX_SIDE`; `$cells` holds `width * height` bytes, rows top first. The grid does not change afterwards.

#### `width(): int`, `height(): int`, `cells(): string`

What it was made with.

#### `cast(float $x, float $y, float $dx, float $dy, float $reach = INF): string`

One ray from (`$x`, `$y`) along (`$dx`, `$dy`): one hit record. The vector is not (0, 0); the reach is above 0 and counts in multiples of the vector.

#### `fan(float $x, float $y, float $dirX, float $dirY, float $planeX, float $planeY, int $columns, float $reach = INF): string`

One ray a column, left to right: `$columns` records, 1 to `RAYCAST_MAX_SIDE`. Column `i` looks along `dir + plane * ((2i + 1) / columns - 1)`, through the centre of its column.

#### `view(float $x, float $y, float $dirX, float $dirY, float $planeX, float $planeY, int $width, int $height, array $walls, int $ceiling, int $floor, int $shade = 255, float $reach = INF): string`

What the fan sees as RGBA8, `$width` by `$height` (each 1 to `RAYCAST_MAX_VIEW`), top row first. The ceiling fills the rows above the middle, the floor the rest. A column that hits a wall gets a line `(int) ($height / distance)` pixels tall, centred.

`$walls` is keyed by wall id, 1 to 255. A value is a colour `0xRRGGBBAA` or a texture `[RGBA8 bytes, width, height]`. The whole table is checked first; a ray that reaches a wall with no entry is a `ValueError` and no picture is answered. `$shade`, 0 to 255, multiplies the red, green and blue of north and south faces by `$shade / 255`, which is what tells two walls apart at a corner; 255 leaves them as they are.

---

#### `viewInto(float $x, float $y, float $dirX, float $dirY, float $planeX, float $planeY, int $width, int $height, array $walls, int $ceiling, int $floor, int $address, int $stride, int $shade = 255, float $reach = INF, int $order = RAYCAST_ORDER_RGBA): void`

`view()` written straight into memory you own, with no string built in between: a framebuffer another extension holds, say. The first pixel goes at `$address` and each row starts `$stride` bytes after the one above, so a `$stride` above `$width * 4` places the picture inside a wider surface and leaves the rest of each row alone. Each pixel is four bytes in `$order`: `RAYCAST_ORDER_RGBA` (red first, the default), `RAYCAST_ORDER_BGRA`, `RAYCAST_ORDER_ARGB` or `RAYCAST_ORDER_ABGR`, so the picture can go into a surface that keeps its bytes another way.

Everything `view()` refuses is refused here before a byte is written, a wall with no entry included. The memory is yours to answer for: `$stride * ($height - 1) + $width * 4` writable bytes for the length of the call. The address is not checked beyond being non-zero, so a wrong one crashes the process.

## Extensions you might use with this one

* [ext-fb](https://github.com/php-io-extensions/fb): a pixel store in any layout. `FbBuffer::blitRgba8()` takes a view's bytes as they are.
* [ext-rasterize](https://github.com/php-io-extensions/rasterize): shapes as coverage spans, for a map, a crosshair or a gauge drawn over the view.

## AI-assisted development

This codebase was built with LLM assistance under human direction. Prompts, reviews, and architectural decisions were owned by a professional software engineer — the model accelerates drafting and exploration; it does not replace judgment on APIs, dependency direction, or what ships.

## Agentic Documentation

This repository carries an [Open Knowledge Format](https://github.com/GoogleCloudPlatform/knowledge-catalog/blob/main/okf/SPEC.md) bundle at [`.okf/`](.okf/): Markdown with YAML frontmatter, readable by you and loadable by a coding agent. It records what the README doesn't: the grid walk and the picture step by step, what every method refuses, why the methods take vectors and not angles, why the build turns off fused multiply-add and how to check it, the scratch build and both installers, and a log of every change that altered behaviour.

Start at [`.okf/index.md`](.okf/index.md). [`AGENTS.md`](AGENTS.md) is the short rule sheet an agent reads before touching the C, the stubs, or packaging; each rule points into the bundle for the reason behind it.

## Resources

* [`src/core.h`](src/core.h): the hit bytes and the casting core, documented next to the code.
* John Amanatides and Andrew Woo, *A Fast Voxel Traversal Algorithm for Ray Tracing* (1987): the grid walk.
* [PIE](https://github.com/php/pie): the PHP Foundation's extension installer.
* [`gen_stub.php`](https://github.com/php/php-src/blob/master/build/gen_stub.php): the php-src stub format. [`stubs/`](stubs) holds the API declarations; each `*_arginfo.h` is generated from its stub.
* [Open Knowledge Format](https://github.com/GoogleCloudPlatform/knowledge-catalog/blob/main/okf/SPEC.md): the spec the [`.okf/`](.okf/) bundle follows.

## Running the test suite

The suite is Pest v4 and needs no hardware or image files: the hit format, each face, corners and boundaries, reaches, fans, pictures pixel by pixel with colours and textures, and every refusal with its message. Build the module in the clone, then point `php` at it:

```bash
git clone https://github.com/php-io-extensions/raycast.git
cd raycast
phpize && ./configure --enable-raycast && make -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)"
composer install
php -n -d extension="$PWD/modules/raycast.so" vendor/bin/pest
```

```console
Tests:    95 passed (224 assertions)
```

`-n` skips your `php.ini`, so the suite tests the clone and not an installed `raycast.so`; without it, a PHP that already loads raycast warns `Module "raycast" is already loaded` and keeps the installed one. A module only loads in the PHP whose `phpize` built it: for Homebrew's ZTS build, run `phpize` and pass `--with-php-config` from `/opt/homebrew/opt/php@8.4-zts/bin`.

## Contributing

Open an issue first for anything beyond a typo. Pull requests change a stub in `stubs/` and the C under `src/` together, regenerate the `*_arginfo.h` files with gen_stub, and add a Pest test for the behaviour. A change to the arithmetic changes the bytes every caller gets, so it needs its reason in the pull request and the expected records or pixels worked by hand in the test. CI must be green on every cell of the matrix. Read [`AGENTS.md`](AGENTS.md) for the rules the code follows.

## Security

ext-raycast reads only the numbers and strings it is given and writes only the strings it returns. See [SECURITY.md](SECURITY.md) for what it guards against and how to report a vulnerability.

## License

MIT. See [LICENSE](LICENSE).
