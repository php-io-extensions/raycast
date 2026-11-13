<?php

declare(strict_types=1);

if (! extension_loaded('raycast')) {
    throw new RuntimeException('The raycast extension is not loaded; run pest with -d extension=/path/to/raycast.so');
}

/**
 * The room every test casts in: walls all round, wall 2 in the east wall on row 3, wall 3 in the south wall on column 1.
 *
 *     1 1 1 1 1
 *     1 . . . 1
 *     1 . . . 1
 *     1 . . . 2
 *     1 3 1 1 1
 */
function room(): RaycastGrid
{
    return new RaycastGrid(5, 5, "\1\1\1\1\1"."\1\0\0\0\1"."\1\0\0\0\1"."\1\0\0\0\2"."\1\3\1\1\1");
}

/**
 * Hit records as [distance, along, x, y, wall, face] lists.
 *
 * @return list<array{float, float, int, int, int, int}>
 */
function hits(string $bytes): array
{
    expect(strlen($bytes) % RAYCAST_HIT_BYTES)->toBe(0);

    return array_map(fn (string $hit): array => array_values(unpack('edistance/ealong/vx/vy/Cwall/Cface', $hit)), str_split($bytes, RAYCAST_HIT_BYTES));
}

/** One hit record as [distance, along, x, y, wall, face]. */
function hit(string $bytes): array
{
    expect(strlen($bytes))->toBe(RAYCAST_HIT_BYTES);

    return hits($bytes)[0];
}

const MISS = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

/**
 * RGBA8 bytes as rows of pixels, each pixel eight hex digits.
 *
 * @return list<list<string>>
 */
function picture(string $rgba8, int $width): array
{
    return array_map(fn (string $row): array => str_split(bin2hex($row), 8), str_split($rgba8, $width * 4));
}

const CEILING = 0x111111FF;
const FLOOR = 0x222222FF;
