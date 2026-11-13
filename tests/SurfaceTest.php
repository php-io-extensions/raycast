<?php

declare(strict_types=1);

it('declares exactly the constants and the class of the stubs', function (): void {
    $constants = array_keys(get_defined_constants(true)['raycast']);
    sort($constants);

    expect($constants)->toBe(['RAYCAST_FACE_EAST', 'RAYCAST_FACE_NORTH', 'RAYCAST_FACE_SOUTH', 'RAYCAST_FACE_WEST', 'RAYCAST_HIT_BYTES', 'RAYCAST_LIMIT', 'RAYCAST_MAX_SIDE', 'RAYCAST_MAX_VIEW', 'RAYCAST_ORDER_ABGR', 'RAYCAST_ORDER_ARGB', 'RAYCAST_ORDER_BGRA', 'RAYCAST_ORDER_RGBA'])
        ->and((new ReflectionExtension('raycast'))->getClassNames())->toBe(['RaycastGrid'])
        ->and((new ReflectionExtension('raycast'))->getFunctions())->toBe([]);

    $methods = array_map(fn (ReflectionMethod $method): string => $method->getName(), (new ReflectionClass(RaycastGrid::class))->getMethods());
    sort($methods);

    expect($methods)->toBe(['__construct', 'cast', 'cells', 'fan', 'height', 'view', 'viewInto', 'width']);
});

it('reports its version', function (): void {
    expect(phpversion('raycast'))->toBe('0.11.0');
});

it('declares a final class that cannot be cloned or serialized', function (): void {
    $grid = new RaycastGrid(1, 1, "\0");

    expect((new ReflectionClass(RaycastGrid::class))->isFinal())->toBeTrue()
        ->and(fn () => clone $grid)->toThrow(Error::class)
        ->and(fn () => serialize($grid))->toThrow(Exception::class);
});

it('refuses a second construction', function (): void {
    $grid = new RaycastGrid(2, 1, "\0\1");

    expect(fn () => $grid->__construct(1, 1, "\0"))->toThrow(Error::class, 'already constructed')
        ->and([$grid->width(), $grid->height(), $grid->cells()])->toBe([2, 1, "\0\1"]);
});
