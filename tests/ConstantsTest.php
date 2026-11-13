<?php

declare(strict_types=1);

it('defines the record size, the limits and the faces', function (): void {
    expect(RAYCAST_HIT_BYTES)->toBe(22)
        ->and(RAYCAST_MAX_SIDE)->toBe(65535)
        ->and(RAYCAST_MAX_VIEW)->toBe(16384)
        ->and(RAYCAST_LIMIT)->toBe(2 ** 30)
        ->and([RAYCAST_FACE_WEST, RAYCAST_FACE_EAST, RAYCAST_FACE_NORTH, RAYCAST_FACE_SOUTH])->toBe([0, 1, 2, 3])
        ->and([RAYCAST_ORDER_RGBA, RAYCAST_ORDER_BGRA, RAYCAST_ORDER_ARGB, RAYCAST_ORDER_ABGR])->toBe([0, 1, 2, 3]);
});
