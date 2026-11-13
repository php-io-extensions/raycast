<?php

declare(strict_types=1);

/*
 * Every expected picture is worked by hand: the ceiling above the middle row, the floor from it
 * down, and in a column that hits, a line (int) (height / distance) tall starting at
 * height / 2 - line / 2 (both whole divisions).
 */

const RED = 0xFF0000FF;
const GREEN = 0x00FF00FF;

/** A 2x2 texture: a b over c d. */
function texture(): array
{
    return [hex2bin('aa0000ff'.'bb0000ff'.'cc0000ff'.'dd0000ff'), 2, 2];
}

it('draws the ceiling, a centred wall line and the floor', function (): void {
    // Distance 1.5, height 4: the line is (int) (4 / 1.5) = 2 tall, from row 2 - 1 = 1.
    expect(picture(room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [1 => RED], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['ff0000ff'], ['ff0000ff'], ['222222ff']]);
});

it('puts the middle row with the floor when the height is odd', function (): void {
    // Height 5: horizon 2, line (int) (5 / 1.5) = 3 from row 2 - 1 = 1.
    expect(picture(room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 5, [1 => RED], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['ff0000ff'], ['ff0000ff'], ['ff0000ff'], ['222222ff']]);
});

it('cuts a line taller than the view to the view', function (): void {
    // Distance 0.5: the line is 8 tall from row -2.
    expect(picture(room()->view(3.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [1 => RED], CEILING, FLOOR), 1))
        ->toBe([['ff0000ff'], ['ff0000ff'], ['ff0000ff'], ['ff0000ff']]);
});

it('fills the column with the wall when the eye is on its face', function (): void {
    // Distance 0: the line is as tall as a line gets.
    expect(picture(room()->view(1.0, 2.5, -1.0, 0.0, 0.0, 0.5, 1, 3, [1 => RED], CEILING, FLOOR), 1))
        ->toBe([['ff0000ff'], ['ff0000ff'], ['ff0000ff']]);
});

it('draws only the ceiling and the floor where nothing is hit', function (): void {
    expect(picture(room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [1 => RED], CEILING, FLOOR, 255, 1.0), 1))
        ->toBe([['111111ff'], ['111111ff'], ['222222ff'], ['222222ff']])
        ->and(picture(room()->view(-1.0, 2.5, 1.0, 0.0, 0.0, 0.5, 2, 2, [], CEILING, FLOOR), 2))
        ->toBe([['111111ff', '111111ff'], ['222222ff', '222222ff']]);
});

it('draws no line for a wall too far to be one pixel tall', function (): void {
    // Height 1, distance 1.5: the line is 0 tall, the horizon is row 0, so the one row is floor.
    expect(picture(room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 1, [1 => RED], CEILING, FLOOR), 1))->toBe([['222222ff']]);
});

it('writes rows of columns, each column its own wall', function (): void {
    // From (2.5, 3.0) the rays (1, -0.25) and (1, 0.25) reach x = 4 at y = 2.625 (wall 1) and y = 3.375 (wall 2), both at distance 1.5.
    expect(picture(room()->view(2.5, 3.0, 1.0, 0.0, 0.0, 0.5, 2, 2, [1 => RED, 2 => GREEN], CEILING, FLOOR), 2))
        ->toBe([['111111ff', '111111ff'], ['ff0000ff', '00ff00ff']]);
});

it('darkens the north and south faces by the shade, and leaves alpha', function (): void {
    // Looking south at a north face: 255 * 128 / 255 = 128.
    expect(picture(room()->view(2.5, 2.5, 0.0, 1.0, -0.5, 0.0, 1, 4, [1 => 0xFF8040C0], CEILING, FLOOR, 128), 1))
        ->toBe([['111111ff'], ['804020c0'], ['804020c0'], ['222222ff']])
        ->and(picture(room()->view(2.5, 2.5, 0.0, -1.0, 0.5, 0.0, 1, 4, [1 => RED], CEILING, FLOOR, 0), 1))
        ->toBe([['111111ff'], ['000000ff'], ['000000ff'], ['222222ff']])
        ->and(picture(room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [1 => RED], CEILING, FLOOR, 128), 1))
        ->toBe([['111111ff'], ['ff0000ff'], ['ff0000ff'], ['222222ff']]);
});

it('samples a texture\'s column under the hit, top to bottom down the line', function (): void {
    // Wall 2 from the west: along 0.25 is texture column 0 (a over c), along 0.75 column 1 (b over d). Line 2 from row 1.
    expect(picture(room()->view(2.5, 3.25, 1.0, 0.0, 0.0, 0.5, 1, 4, [2 => texture()], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['aa0000ff'], ['cc0000ff'], ['222222ff']])
        ->and(picture(room()->view(2.5, 3.75, 1.0, 0.0, 0.0, 0.5, 1, 4, [2 => texture()], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['bb0000ff'], ['dd0000ff'], ['222222ff']]);
});

it('stretches a texture over a line of any height', function (): void {
    // Height 8: line (int) (8 / 1.5) = 5 from row 4 - 2 = 2; row r reads texture row (r - 2) * 2 / 5 = 0, 0, 0, 1, 1.
    expect(picture(room()->view(2.5, 3.25, 1.0, 0.0, 0.0, 0.5, 1, 8, [2 => texture()], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['111111ff'], ['aa0000ff'], ['aa0000ff'], ['aa0000ff'], ['cc0000ff'], ['cc0000ff'], ['222222ff']]);
});

it('reads a texture left to right as its face is seen: east and north faces run the other way', function (): void {
    $east = new RaycastGrid(3, 1, "\2\0\0");    // seen from the east, travelling west
    $north = new RaycastGrid(1, 3, "\0\0\2");   // seen from the north, travelling south
    $south = new RaycastGrid(1, 3, "\2\0\0");   // seen from the south, travelling north

    // along 0.25 on each: column 0 as stored, column 1 where the face runs the other way.
    expect(picture($east->view(2.5, 0.25, -1.0, 0.0, 0.0, -0.5, 1, 4, [2 => texture()], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['bb0000ff'], ['dd0000ff'], ['222222ff']])
        ->and(picture($north->view(0.25, 0.5, 0.0, 1.0, -0.5, 0.0, 1, 4, [2 => texture()], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['bb0000ff'], ['dd0000ff'], ['222222ff']])
        ->and(picture($south->view(0.25, 2.5, 0.0, -1.0, 0.5, 0.0, 1, 4, [2 => texture()], CEILING, FLOOR), 1))
        ->toBe([['111111ff'], ['aa0000ff'], ['cc0000ff'], ['222222ff']]);
});

it('darkens a texture on a north face too', function (): void {
    $north = new RaycastGrid(1, 3, "\0\0\2");

    // 0xbb * 128 / 255 = 93 = 0x5d; 0xdd * 128 / 255 = 110 = 0x6e.
    expect(picture($north->view(0.25, 0.5, 0.0, 1.0, -0.5, 0.0, 1, 4, [2 => texture()], CEILING, FLOOR, 128), 1))
        ->toBe([['111111ff'], ['5d0000ff'], ['6e0000ff'], ['222222ff']]);
});

it('refuses a ray that hits a wall with no entry', function (): void {
    expect(fn () => room()->view(2.5, 3.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [1 => RED], CEILING, FLOOR))->toThrow(ValueError::class, 'Wall 2 has no colour or texture.');
});

it('refuses walls that are not keyed 1..255, before a ray is cast', function (array $walls): void {
    expect(fn () => room()->view(-1.0, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, $walls, CEILING, FLOOR))->toThrow(ValueError::class, 'The walls are keyed by wall id, 1..255');
})->with([[[0 => RED]], [[256 => RED]], [['one' => RED]], [[-1 => RED]]]);

it('refuses a wall that is not a colour or a texture, before a ray is cast', function (mixed $wall, string $message): void {
    expect(fn () => room()->view(-1.0, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [7 => $wall], CEILING, FLOOR))->toThrow(ValueError::class, $message);
})->with([
    [-1, 'Wall 7 is a colour 0xRRGGBBAA'],
    [0x100000000, 'Wall 7 is a colour 0xRRGGBBAA'],
    ['red', 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [1.5, 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [[], 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [['abcd', 1], 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [['bytes' => 'abcd', 'width' => 1, 'height' => 1], 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [[1, 1, 1], 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [['abcd', 1.0, 1], 'Wall 7 is a colour or [RGBA8 bytes, width, height]'],
    [['abcd', 0, 1], 'Wall 7\'s texture 0x1 has a side outside 1..65535'],
    [['abcd', 1, 65536], 'Wall 7\'s texture 1x65536 has a side outside 1..65535'],
    [['abc', 1, 1], 'Wall 7\'s texture 1x1 holds 4 bytes of RGBA8; got 3'],
    [['abcdefgh', 1, 1], 'Wall 7\'s texture 1x1 holds 4 bytes of RGBA8; got 8'],
]);

it('refuses a view with a side outside 1..16384', function (int $width, int $height): void {
    expect(fn () => room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, $width, $height, [1 => RED], CEILING, FLOOR))->toThrow(ValueError::class, 'must be 1..16384');
})->with([[0, 4], [4, 0], [-1, 4], [16385, 4], [4, 16385]]);

it('refuses a ceiling, a floor or a shade out of range', function (int $ceiling, int $floor, int $shade, string $message): void {
    expect(fn () => room()->view(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 1, 4, [1 => RED], $ceiling, $floor, $shade))->toThrow(ValueError::class, $message);
})->with([
    [-1, FLOOR, 255, 'The ceiling is a colour 0xRRGGBBAA'],
    [CEILING, 0x100000000, 255, 'The floor is a colour 0xRRGGBBAA'],
    [CEILING, FLOOR, 256, 'must be 0..255'],
    [CEILING, FLOOR, -1, 'must be 0..255'],
]);

it('answers the same picture for the same call', function (): void {
    $walls = [1 => RED, 2 => texture(), 3 => GREEN];
    $view = fn (): string => room()->view(1.7, 2.3, 0.6, 0.8, -0.528, 0.396, 64, 48, $walls, CEILING, FLOOR, 170);

    expect(strlen($view()))->toBe(64 * 48 * 4)
        ->and($view())->toBe($view());
});
