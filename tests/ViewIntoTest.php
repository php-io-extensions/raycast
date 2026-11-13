<?php

declare(strict_types=1);

/*
 * viewInto() writes view()'s picture into memory the caller owns. The memory here is an ext-fb
 * buffer's, which is only where the tests get an address from: ext-raycast knows nothing of it.
 */

/** An RGBA8 surface to write into: [the buffer that owns the bytes, their address, bytes a row]. */
function surface(int $width, int $height, int $fill = 0x77777777): array
{
    $buffer = new FbBuffer(new FbFormat(FB_LAYOUT_RGBA8888, channelOrder: FB_CHANNELS_RGBA), $width, $height);
    $buffer->fill($fill);

    return [$buffer, $buffer->pointer(), $width * 4];
}

const NEEDS_FB = 'ext-fb is not loaded in this PHP: it is where these tests get an address from.';

$walls = [1 => 0xFF0000FF, 2 => [hex2bin('aa0000ff'.'bb0000ff'.'cc0000ff'.'dd0000ff'), 2, 2], 3 => 0x00FF00FF];

it('writes the picture view() answers', function (int $width, int $height, float $dirX, float $dirY) use ($walls): void {
    [$buffer, $address, $stride] = surface($width, $height);
    $camera = [2.5, 2.5, $dirX, $dirY, -$dirY * 0.66, $dirX * 0.66];

    room()->viewInto(...$camera, ...[$width, $height, $walls, 0x111111FF, 0x222222FF, $address, $stride, 180]);

    expect(bin2hex($buffer->bytes()))->toBe(bin2hex(room()->view(...$camera, ...[$width, $height, $walls, 0x111111FF, 0x222222FF, 180])));
})->with([[1, 4], [16, 12], [64, 40], [33, 17]])->with([[1.0, 0.0], [0.0, 1.0], [-0.6, 0.8], [0.8, -0.6]])->skip(! class_exists(FbBuffer::class), NEEDS_FB);

it('places the picture inside a wider surface and touches nothing else', function () use ($walls): void {
    [$buffer, $address, $stride] = surface(20, 10);
    $camera = [2.5, 2.5, 1.0, 0.0, 0.0, 0.66];

    // 8x4 with its top-left at (5, 3)
    room()->viewInto(...$camera, ...[8, 4, $walls, 0x111111FF, 0x222222FF, $address + 3 * $stride + 5 * 4, $stride]);

    $picture = str_split(room()->view(...$camera, ...[8, 4, $walls, 0x111111FF, 0x222222FF]), 8 * 4);
    foreach (str_split($buffer->bytes(), $stride) as $y => $row) {
        $inside = $y >= 3 && $y < 7;
        expect(bin2hex(substr($row, 5 * 4, 8 * 4)))->toBe(bin2hex($inside ? $picture[$y - 3] : str_repeat("\x77", 32)), "row {$y}")
            ->and(bin2hex(substr($row, 0, 20)))->toBe(str_repeat('77', 20), "row {$y}, left")
            ->and(bin2hex(substr($row, 13 * 4)))->toBe(str_repeat('77', 28), "row {$y}, right");
    }
})->skip(! class_exists(FbBuffer::class), NEEDS_FB);

it('refuses what view() refuses, with nothing written', function () use ($walls): void {
    [$buffer, $address, $stride] = surface(8, 4);
    $blank = $buffer->bytes();
    $camera = [2.5, 2.5, 1.0, 0.0, 0.0, 0.66];

    expect(fn () => room()->viewInto(...$camera, ...[8, 4, [1 => 0xFF0000FF], 0x111111FF, 0x222222FF, $address, $stride]))
        ->toThrow(ValueError::class, 'Wall 2 has no colour or texture.')   // the east wall holds wall 2
        ->and(fn () => room()->viewInto(...$camera, ...[0, 4, $walls, 0x111111FF, 0x222222FF, $address, $stride]))->toThrow(ValueError::class)
        ->and(fn () => room()->viewInto(...$camera, ...[8, 4, $walls, 0x111111FF, 0x222222FF, $address, $stride, 256]))->toThrow(ValueError::class, 'must be 0..255')
        ->and(fn () => room()->viewInto(NAN, 2.5, 1.0, 0.0, 0.0, 0.66, 8, 4, $walls, 0x111111FF, 0x222222FF, $address, $stride))->toThrow(ValueError::class)
        ->and($buffer->bytes())->toBe($blank);
})->skip(! class_exists(FbBuffer::class), NEEDS_FB);

it('refuses no address and rows closer than the picture is wide', function () use ($walls): void {
    [, $address, $stride] = surface(8, 4);
    $camera = [2.5, 2.5, 1.0, 0.0, 0.0, 0.66];

    expect(fn () => room()->viewInto(...$camera, ...[8, 4, $walls, 0x111111FF, 0x222222FF, 0, $stride]))->toThrow(ValueError::class, 'must be the address of the first pixel')
        ->and(fn () => room()->viewInto(...$camera, ...[8, 4, $walls, 0x111111FF, 0x222222FF, $address, 31]))->toThrow(ValueError::class, 'must be at least width * 4 (32), 31 given');
})->skip(! class_exists(FbBuffer::class), NEEDS_FB);

it('writes a pixel\'s four bytes in the order asked', function (int $order, array $from) use ($walls): void {
    [$buffer, $address, $stride] = surface(16, 12);
    $camera = [2.5, 3.25, 1.0, 0.0, 0.0, 0.66];

    room()->viewInto(...$camera, ...[16, 12, $walls, 0x112233FF, 0x445566AA, $address, $stride, 180, INF, $order]);

    // each pixel of the red-first picture, its bytes moved to where the order puts them
    $pixels = str_split(room()->view(...$camera, ...[16, 12, $walls, 0x112233FF, 0x445566AA, 180]), 4);
    $expected = implode('', array_map(fn (string $p): string => $p[$from[0]].$p[$from[1]].$p[$from[2]].$p[$from[3]], $pixels));

    expect(bin2hex($buffer->bytes()))->toBe(bin2hex($expected));
})->with([
    'RGBA' => [RAYCAST_ORDER_RGBA, [0, 1, 2, 3]],
    'BGRA' => [RAYCAST_ORDER_BGRA, [2, 1, 0, 3]],
    'ARGB' => [RAYCAST_ORDER_ARGB, [3, 0, 1, 2]],
    'ABGR' => [RAYCAST_ORDER_ABGR, [3, 2, 1, 0]],
])->skip(! class_exists(FbBuffer::class), NEEDS_FB);

it('refuses an order it does not have', function () use ($walls): void {
    [, $address, $stride] = surface(8, 4);

    expect(fn () => room()->viewInto(2.5, 2.5, 1.0, 0.0, 0.0, 0.66, 8, 4, $walls, 0x111111FF, 0x222222FF, $address, $stride, 255, INF, 4))
        ->toThrow(ValueError::class, 'must be a RAYCAST_ORDER_* constant');
})->skip(! class_exists(FbBuffer::class), NEEDS_FB);
