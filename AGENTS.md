# Agent guidance — php-io-extensions/raycast

1. **Read [`.okf/index.md`](.okf/index.md) first** before changing the API, the C, or packaging. Open only the concepts the change touches.
2. **A grid and rays, nothing else.** `RaycastGrid` answers hit records and RGBA8 pictures as strings, and `viewInto()` writes a picture into memory the caller names by address. No pixel stores of its own, no windows, no game rules, no dependency, no framework names. See [`.okf/api/grid.md`](.okf/api/grid.md).
3. **Plain C on the Zend API.** No Zephir, no FFI, no system library. `src/core.{h,c}` is plain C with no Zend in it and writes only into buffers the class hands it; `src/RaycastGrid.c` is the class over it; `src/raycast.c` is the module.
4. **The stubs are the declaration.** Edit `stubs/*.stub.php`, regenerate the matching `*_arginfo.h` with gen_stub, commit both. Never hand-edit arginfo. See [`.okf/runbooks/build.md`](.okf/runbooks/build.md).
5. **The hit bytes are a contract.** 22 bytes, little-endian `distance` and `along` (float64), `x` and `y` (uint16), `wall` and `face` (uint8); a miss is 22 zero bytes. Consumers parse them as they are.
6. **The arithmetic is a contract.** IEEE double with `+ − × ÷`, `fabs` and `floor` in the order [`.okf/api/grid.md`](.okf/api/grid.md) writes down, and whole-number division for a picture's rows and shading, so the same input answers the same bytes on every platform and from a PHP implementation of the same steps. A change to it changes every caller's output: it needs its reason, and its tests work the expected records and pixels by hand.
7. **`-ffp-contract=off` stays in `config.m4`.** Without it the compiler fuses multiply-adds in `core.c` and the bytes move. Check with `objdump -d src/.libs/core.o | grep -c fmadd`: 0.
8. **No trigonometry in C.** The methods take vectors, not angles: `cos`, `sin` and `tan` differ between C libraries in their last bit, so the caller works them once and every implementation is handed the same numbers.
9. **Refuse, don't guess.** A non-finite number, a position past `RAYCAST_LIMIT`, a vector of (0, 0), a reach not above 0, a size out of range or a malformed wall table is a `ValueError` naming what was wrong, before any ray is cast. A ray that reaches a wall with no entry in a view's table is a `ValueError` too, and no picture is answered.
10. **NTS and ZTS.** No module globals. Every change builds with `-Wall -Wextra` without a warning from `src/` and passes Pest under NTS and ZTS on macOS and Linux.
11. **Tests are Pest v4 and hardware-free.** Expected hits and pixels come from the grid walk, worked by hand.
12. **Durable facts go in `.okf`.** Update the matching concept, bump its `generated.at`, append `.okf/log.md`. The bundle documents the package, never a session.
13. **Version**: `PHP_RAYCAST_VERSION` moves only past a published tag; tags drive Packagist.
