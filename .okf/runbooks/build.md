---
type: Runbook
title: Build, install, test
description: Scratch build with gen_stub, the two installers, Pest.
resource: install-macos.sh
tags: [build, linux, macos, pest]
status: draft
generated: { by: claude-fable/5.2, at: 2026-11-12T00:00:00Z }
sources:
  - id: mac
    resource: install-macos.sh
    title: install-macos.sh
  - id: debian
    resource: install-debian-trixie.sh
    title: install-debian-trixie.sh
---

# Overview

No system library needed: php-dev and a C compiler.

Dev loop: copy `config.m4 php_raycast.h src stubs` to a scratch dir, `phpize`, `php build/gen_stub.php stubs`, `./configure --enable-raycast --with-php-config=… CFLAGS="-g -O2 -Wall -Wextra"`, `make`. Copy `stubs/*_arginfo.h` back after a stub edit. Run Pest with `-d extension=<scratch>/modules/raycast.so`. No warning from `src/` is acceptable; the linker's `-undefined suppress is deprecated` on macOS is PHP's own.

macOS (`install-macos.sh`): builds in a temp copy for php@8.4 and php@8.4-zts, ad-hoc signs, writes `30-raycast.ini`.[^mac]

Linux (`install-debian-trixie.sh`): builds in place, installs `raycast.so`, writes `30-raycast.ini`, removes build output.[^debian] Not yet run for this extension.

`config.m4` passes `-ffp-contract=off`; a build without it may fuse the multiply-adds of the grid walk (`y + distance * dy`, `dir + plane * across`) and answer different bytes from a PHP implementation of the same arithmetic. Check with `objdump -d src/.libs/core.o | grep -c fmadd`: 0 on arm64.

To test a consumer without the ext, run with a `PHP_INI_SCAN_DIR` that omits `30-raycast.ini`; `php -n` drops every other conf.d extension too.

[^mac]: Same script as ext-rasterize's, names changed.
[^debian]: Same script as ext-rasterize's, names changed.
