# Log

## 2026-11-13

* `viewInto()` takes a byte order, `RAYCAST_ORDER_RGBA`, `_BGRA`, `_ARGB` or `_ABGR`, so a picture goes in place into a surface that keeps its bytes another way. 119 passed on NTS and ZTS. [grid](api/grid.md)
* `RaycastGrid::viewInto()`: `view()`'s picture written into caller memory at an address, rows a stride apart, so a picture lands in another extension's framebuffer with no string in between. `raycast_view_rows()` and `raycast_view_missing()` in `core.c`. 114 passed on PHP 8.4.27 NTS and ZTS; `tests/ViewIntoTest.php` also under guard malloc. [grid](api/grid.md)

## 2026-11-12

* Bundle created with ext-raycast 0.11.0: [grid](api/grid.md) (`RaycastGrid`, 8 constants), [build](runbooks/build.md). `PHP_RAYCAST_VERSION` 0.11.0, `branch-alias` `dev-main` is `0.11.x-dev`, on the Venusian 0.11 line. Built with `-Wall -Wextra` with no warning from `src/`, `fmadd` count 0, installed and 95 passed on Homebrew PHP 8.4.27 NTS and ZTS (macOS arm64). Not yet built on Linux.
