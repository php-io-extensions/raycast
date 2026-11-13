<?php

declare(strict_types=1);

it('casts one ray a column through the column\'s centre, left to right', function (): void {
    // Two columns: dir + plane * -0.5 and dir + plane * 0.5, so the rays are (1, -0.25) and (1, 0.25).
    $fan = room()->fan(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 2);

    expect(hits($fan))->toBe([
        [1.5, 0.125, 4, 2, 1, RAYCAST_FACE_WEST],
        [1.5, 0.875, 4, 2, 1, RAYCAST_FACE_WEST],
    ])
        ->and(substr($fan, 0, RAYCAST_HIT_BYTES))->toBe(room()->cast(2.5, 2.5, 1.0, -0.25))
        ->and(substr($fan, RAYCAST_HIT_BYTES))->toBe(room()->cast(2.5, 2.5, 1.0, 0.25));
});

it('gives a flat wall one distance across the fan', function (): void {
    $distances = array_column(hits(room()->fan(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 9)), 0);

    expect(array_unique($distances))->toBe([1.5]);
});

it('looks straight ahead through a single column', function (): void {
    expect(room()->fan(2.5, 3.25, 1.0, 0.0, 0.0, 0.5, 1))->toBe(room()->cast(2.5, 3.25, 1.0, 0.0));
});

it('writes a miss for a column that hits nothing, and for one whose ray vanishes', function (): void {
    // Reach 1.4 stops short of the wall. A plane equal to the direction makes the leftmost of two columns' ray (0.5, 0); of one column, with the plane -dir... none vanish here, so use dir (1, 0) and plane (-2, 0): columns at -0.5 and 0.5 give (2, 0) and (0, 0).
    expect(room()->fan(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 3, 1.4))->toBe(str_repeat(MISS, 3))
        ->and(hits(room()->fan(2.5, 2.5, 1.0, 0.0, -2.0, 0.0, 2)))->toBe([
            [0.75, 0.5, 4, 2, 1, RAYCAST_FACE_WEST],
            [0.0, 0.0, 0, 0, 0, 0],
        ]);
});

it('answers as many records as columns', function (int $columns): void {
    expect(strlen(room()->fan(2.5, 2.5, 1.0, 0.0, 0.0, 0.66, $columns)))->toBe($columns * RAYCAST_HIT_BYTES);
})->with([1, 2, 320, 65535]);

it('refuses a count of columns outside 1..65535', function (int $columns): void {
    expect(fn () => room()->fan(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, $columns))->toThrow(ValueError::class, 'must be 1..65535');
})->with([0, -1, 65536]);

it('refuses a camera that is not finite, or looks nowhere', function (array $camera, string $message): void {
    expect(fn () => room()->fan(...[...$camera, 4]))->toThrow(ValueError::class, $message);
})->with([
    [[NAN, 2.5, 1.0, 0.0, 0.0, 0.5], 'x is not a finite number'],
    [[2.5, 2.0 ** 31, 1.0, 0.0, 0.0, 0.5], 'is past ±1073741824'],
    [[2.5, 2.5, 0.0, 0.0, 0.0, 0.5], 'The direction is (0, 0)'],
    [[2.5, 2.5, INF, 0.0, 0.0, 0.5], 'The direction is not two finite numbers'],
    [[2.5, 2.5, 1.0, 0.0, NAN, 0.5], 'The plane is not two finite numbers'],
]);

it('refuses a reach that is not above 0', function (): void {
    expect(fn () => room()->fan(2.5, 2.5, 1.0, 0.0, 0.0, 0.5, 4, 0.0))->toThrow(ValueError::class, 'The reach is above 0, or INF');
});
