# Security Policy

## Supported versions

ext-raycast is pre-1.0. No 0.x release receives security fixes or advisories; fixes land in
the next release line. Security support starts with 1.0.

| Version | Security fixes |
|---------|----------------|
| < 1.0   | No             |

## Reporting a vulnerability

Please don't open a public issue for a security problem.

Report it privately through GitHub: the **Report a vulnerability** button on the
[php-io-extensions/raycast](https://github.com/php-io-extensions/raycast) repository's
**Security** tab. If that isn't available, email **info@projectsaturnstudios.com**.

Include what you found, the affected version, your platform and PHP build (NTS or ZTS), the
grid's size and cells and the call that triggers it, and steps to reproduce. Reports are read
and weighed for the release line in development; before 1.0 there is no response-time
commitment.

## Security model

ext-raycast is arithmetic in plain C: it opens no files, makes no network requests, links no
library and holds no state between calls beyond each grid's cells. It reads the numbers and
strings it is given and writes only the strings it returns.

- **Input.** Every number is checked before a ray is cast: a position is finite with magnitude
  at most `RAYCAST_LIMIT` (2³⁰), a vector is finite and a direction is not (0, 0), a reach is
  above 0. A view's wall table is checked whole first: keys 1..255, a colour in 0..0xFFFFFFFF,
  a texture whose byte count equals its width × height × 4. Anything else is a `ValueError`.
- **Bounds.** A grid's sides are 1..65535 and its cells are exactly width × height bytes, so
  every cell a ray enters is inside the string and every hit's `x` and `y` fit their uint16
  fields. A ray that starts outside the grid or steps out of it stops there. A walk takes at
  most width + height steps. A view's sides are 1..16384; a wall line is cut to the view before
  a pixel is written, and a texture row and column are held inside the texture.
- **Memory.** A grid keeps a reference to its cells string. `cast()`, `fan()` and `view()`
  allocate only the string they return, from PHP's allocator, so `memory_limit` bounds a large
  view. Textures are read in place for the length of the call.

A report is in scope when ext-raycast itself reads or writes memory it should not, leaks, or
crashes on any argument: a cell read outside the grid, a pixel written outside the view, a
texel read outside a texture, an allocation whose size can wrap.
