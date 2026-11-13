<?php

declare(strict_types=1);

/*
 * Every expected record is worked by hand from the walk: from the eye's cell, step to whichever
 * boundary the ray reaches first and look at the cell entered.
 */

it('names the face a ray strikes, by the way it was travelling', function (float $dx, float $dy, array $expected): void {
    expect(hit(room()->cast(2.5, 2.5, $dx, $dy)))->toBe($expected);
})->with([
    'east' => [1.0, 0.0, [1.5, 0.5, 4, 2, 1, RAYCAST_FACE_WEST]],
    'west' => [-1.0, 0.0, [1.5, 0.5, 0, 2, 1, RAYCAST_FACE_EAST]],
    'south' => [0.0, 1.0, [1.5, 0.5, 2, 4, 1, RAYCAST_FACE_NORTH]],
    'north' => [0.0, -1.0, [1.5, 0.5, 2, 0, 1, RAYCAST_FACE_SOUTH]],
]);

it('answers the wall\'s own byte and where on its face, toward +y or +x', function (): void {
    expect(hit(room()->cast(2.5, 3.25, 1.0, 0.0)))->toBe([1.5, 0.25, 4, 3, 2, RAYCAST_FACE_WEST])
        ->and(hit(room()->cast(2.5, 3.75, 1.0, 0.0)))->toBe([1.5, 0.75, 4, 3, 2, RAYCAST_FACE_WEST])
        ->and(hit(room()->cast(1.25, 2.5, 0.0, 1.0)))->toBe([1.5, 0.25, 1, 4, 3, RAYCAST_FACE_NORTH]);
});

it('measures the distance in multiples of the vector', function (): void {
    expect(hit(room()->cast(2.5, 2.5, 2.0, 0.0)))->toBe([0.75, 0.5, 4, 2, 1, RAYCAST_FACE_WEST])
        ->and(hit(room()->cast(2.5, 2.5, 0.5, 0.0)))->toBe([3.0, 0.5, 4, 2, 1, RAYCAST_FACE_WEST]);
});

it('steps in y when both boundaries are as near', function (): void {
    // Down the diagonal from (1.5, 1.5): every crossing is a corner. y first each time: (1,2), (2,2), (2,3), (3,3), then (3,4), a wall.
    expect(hit(room()->cast(1.5, 1.5, 1.0, 1.0)))->toBe([2.5, 0.0, 3, 4, 1, RAYCAST_FACE_NORTH]);
});

it('stops at its reach: a wall at the reach is hit, one past it is missed', function (): void {
    expect(hit(room()->cast(2.5, 2.5, 1.0, 0.0, 1.5)))->toBe([1.5, 0.5, 4, 2, 1, RAYCAST_FACE_WEST])
        ->and(room()->cast(2.5, 2.5, 1.0, 0.0, 1.4))->toBe(MISS)
        ->and(hit(room()->cast(2.5, 2.5, 1.0, 0.0, INF)))->toBe([1.5, 0.5, 4, 2, 1, RAYCAST_FACE_WEST]);
});

it('does not look at the cell it starts in', function (): void {
    $grid = new RaycastGrid(3, 1, "\1\0\1");

    expect(hit($grid->cast(0.5, 0.5, 1.0, 0.0)))->toBe([1.5, 0.5, 2, 0, 1, RAYCAST_FACE_WEST]);
});

it('misses from outside the grid, and when it leaves the grid', function (): void {
    $open = new RaycastGrid(3, 3, str_repeat("\0", 9));

    expect(room()->cast(-0.5, 2.5, 1.0, 0.0))->toBe(MISS)
        ->and(room()->cast(5.0, 2.5, -1.0, 0.0))->toBe(MISS)
        ->and(room()->cast(2.5, -0.001, 0.0, 1.0))->toBe(MISS)
        ->and(room()->cast(2.5, 5.0, 0.0, -1.0))->toBe(MISS)
        ->and($open->cast(1.5, 1.5, 1.0, 0.0))->toBe(MISS)
        ->and($open->cast(1.5, 1.5, -0.3, 0.7))->toBe(MISS);
});

it('starts on a boundary in the cell at or after it', function (): void {
    // x = 3.0 is in cell 3; westward, the boundary under the eye is crossed at distance 0 into cell 2.
    expect(hit(room()->cast(3.0, 2.5, 1.0, 0.0)))->toBe([1.0, 0.5, 4, 2, 1, RAYCAST_FACE_WEST])
        ->and(hit(room()->cast(3.0, 2.5, -1.0, 0.0)))->toBe([2.0, 0.5, 0, 2, 1, RAYCAST_FACE_EAST])
        ->and(hit(room()->cast(1.0, 2.5, -1.0, 0.0)))->toBe([0.0, 0.5, 0, 2, 1, RAYCAST_FACE_EAST]);
});

it('refuses a position that is not finite or is past the limit', function (float $x, float $y, string $message): void {
    expect(fn () => room()->cast($x, $y, 1.0, 0.0))->toThrow(ValueError::class, $message);
})->with([
    [NAN, 0.5, 'x is not a finite number'],
    [0.5, INF, 'y is not a finite number'],
    [-INF, 0.5, 'x is not a finite number'],
    [2.0 ** 30 + 1, 0.5, 'is past ±1073741824'],
    [0.5, -(2.0 ** 30) - 1, 'is past ±1073741824'],
]);

it('refuses a ray with nowhere to go', function (float $dx, float $dy, string $message): void {
    expect(fn () => room()->cast(2.5, 2.5, $dx, $dy))->toThrow(ValueError::class, $message);
})->with([
    [0.0, 0.0, 'The ray is (0, 0)'],
    [NAN, 1.0, 'The ray is not two finite numbers'],
    [1.0, INF, 'The ray is not two finite numbers'],
]);

it('refuses a reach that is not above 0', function (float $reach): void {
    expect(fn () => room()->cast(2.5, 2.5, 1.0, 0.0, $reach))->toThrow(ValueError::class, 'The reach is above 0, or INF');
})->with([0.0, -1.0, NAN, -INF]);
