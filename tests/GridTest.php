<?php

declare(strict_types=1);

it('holds its sides and its cells', function (): void {
    $grid = new RaycastGrid(3, 2, "\1\0\2\0\0\xFF");

    expect($grid->width())->toBe(3)
        ->and($grid->height())->toBe(2)
        ->and($grid->cells())->toBe("\1\0\2\0\0\xFF");
});

it('refuses a side outside 1..65535', function (int $width, int $height): void {
    expect(fn () => new RaycastGrid($width, $height, ''))->toThrow(ValueError::class, 'has a side outside 1..65535');
})->with([[0, 1], [1, 0], [-1, 1], [65536, 1], [1, 65536]]);

it('refuses cells that are not one byte a cell', function (string $cells): void {
    expect(fn () => new RaycastGrid(2, 2, $cells))->toThrow(ValueError::class, 'holds 4 cells, one byte each; got '.strlen($cells).' bytes');
})->with(['', "\0\0\0", "\0\0\0\0\0"]);

it('keeps its cells when the string it was made from changes', function (): void {
    $cells = "\0\1";
    $grid = new RaycastGrid(2, 1, $cells);
    $cells[0] = "\1";

    expect($grid->cells())->toBe("\0\1")
        ->and(hit($grid->cast(0.5, 0.5, 1.0, 0.0)))->toBe([0.5, 0.5, 1, 0, 1, RAYCAST_FACE_WEST]);
});
