<?php

/** @generate-class-entries */

/**
 * Rays through a grid of cells. A grid is width x height cells, one byte each,
 * rows top first: 0 is open, 1..255 is a wall and the byte is its id. x grows
 * east and y grows south, in cells; (0.5, 0.5) is the middle of the first cell.
 *
 * A ray walks from cell to cell and stops at the first wall it enters. The
 * cell it starts in is not looked at; a ray that starts outside the grid, or
 * leaves it, or goes past its reach, misses.
 *
 * A hit is RAYCAST_HIT_BYTES (22) bytes, little-endian: distance (float64),
 * along (float64), x (uint16), y (uint16), wall (uint8), face (uint8). The
 * distance is the multiple of the ray's vector at the face. along is where
 * on the face, 0 <= along < 1, measured toward +y on a west or east face and
 * toward +x on a north or south one. A miss is 22 zero bytes.
 *
 * A position is finite with magnitude at most RAYCAST_LIMIT; a vector is
 * finite; a reach is above 0 and may be INF. Anything else is ValueError.
 *
 * @not-serializable
 */
final class RaycastGrid
{
    /** Both sides 1..RAYCAST_MAX_SIDE; $cells holds width * height bytes. */
    public function __construct(int $width, int $height, string $cells) {}

    public function width(): int {}

    public function height(): int {}

    public function cells(): string {}

    /** One ray from ($x, $y) along ($dx, $dy), not both 0: one hit record. With a unit vector the distance is in cells. */
    public function cast(float $x, float $y, float $dx, float $dy, float $reach = INF): string {}

    /**
     * One ray a column, left to right: $columns hit records. Column i looks along
     * dir + plane * ((2i + 1) / columns - 1). With a unit direction and a plane at
     * right angles to it, a distance is measured along the direction, not along
     * the ray, so a flat wall has one distance across the fan.
     *
     * @param int $columns 1..RAYCAST_MAX_SIDE
     */
    public function fan(float $x, float $y, float $dirX, float $dirY, float $planeX, float $planeY, int $columns, float $reach = INF): string {}

    /**
     * What the fan sees as RGBA8 pixels, $width x $height, top row first: $ceiling
     * above the middle row, $floor from it down, and in each column that hits a
     * wall a line of that wall, (int) ($height / distance) pixels tall, centred.
     *
     * A wall is one colour or a texture stretched over each face, sampled
     * nearest. The red, green and blue of a north or south face are multiplied
     * by $shade / 255.
     *
     * @param int $width 1..RAYCAST_MAX_VIEW
     * @param int $height 1..RAYCAST_MAX_VIEW
     * @param array $walls By wall id 1..255: a colour 0xRRGGBBAA, or [RGBA8 bytes, width, height] with both sides 1..RAYCAST_MAX_SIDE. Every entry is checked before a ray is cast; a ray that hits a wall with no entry is ValueError.
     * @param int $ceiling 0xRRGGBBAA
     * @param int $floor 0xRRGGBBAA
     * @param int $shade 0..255; 255 leaves every face as it is.
     */
    public function view(float $x, float $y, float $dirX, float $dirY, float $planeX, float $planeY, int $width, int $height, array $walls, int $ceiling, int $floor, int $shade = 255, float $reach = INF): string {}

    /**
     * view() written straight into memory the caller owns, nothing built in
     * between: $width x $height pixels of four bytes each, in $order (a
     * RAYCAST_ORDER_*: red first, or as a window's surface keeps them), top
     * row first, the first at $address and each row $stride bytes after the
     * one above. A
     * $stride above $width * 4 places the picture inside a wider surface; the
     * rest of each row is not touched.
     *
     * Everything view() refuses is refused here before a byte is written, a
     * wall with no entry included. The caller answers for the memory: it is
     * $stride * ($height - 1) + $width * 4 bytes, writable, for the length of
     * the call. The address is not checked beyond being non-zero.
     *
     * @param int $address the first pixel's first byte
     * @param int $stride at least $width * 4
     */
    public function viewInto(float $x, float $y, float $dirX, float $dirY, float $planeX, float $planeY, int $width, int $height, array $walls, int $ceiling, int $floor, int $address, int $stride, int $shade = 255, float $reach = INF, int $order = RAYCAST_ORDER_RGBA): void {}
}
