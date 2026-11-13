#ifndef PHP_RAYCAST_H
#define PHP_RAYCAST_H

extern zend_module_entry raycast_module_entry;
#define phpext_raycast_ptr &raycast_module_entry

#define PHP_RAYCAST_VERSION "0.11.0"

#if defined(ZTS) && defined(COMPILE_DL_RAYCAST)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif
