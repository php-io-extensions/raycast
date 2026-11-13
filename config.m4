PHP_ARG_ENABLE([raycast],
  [whether to enable raycast support],
  [AS_HELP_STRING([--enable-raycast], [Enable the raycast grid casting extension])],
  [no])

if test "$PHP_RAYCAST" != "no"; then
  dnl -ffp-contract=off: no fused multiply-add, so every platform rounds each step the same way.
  PHP_NEW_EXTENSION([raycast],
    [src/raycast.c src/core.c src/RaycastGrid.c],
    [$ext_shared],, [-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 -ffp-contract=off])
  PHP_ADD_BUILD_DIR([$ext_builddir/src])
fi
