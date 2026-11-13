/*
 * raycast: rays through a grid of cells in C. RaycastGrid answers what a ray
 * hits, what a fan of rays hits, and the picture a camera sees.
 */

#include "runtime.h"
#include "ext/standard/info.h"
#include "../stubs/raycast_arginfo.h"

PHP_MINIT_FUNCTION(raycast)
{
	(void) type;

	register_raycast_symbols(module_number);

	phpraycast_register_RaycastGrid();

	return SUCCESS;
}

PHP_RINIT_FUNCTION(raycast)
{
	(void) type;
	(void) module_number;
#if defined(COMPILE_DL_RAYCAST) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}

PHP_MINFO_FUNCTION(raycast)
{
	(void) zend_module;

	php_info_print_table_start();
	php_info_print_table_row(2, "raycast support", "enabled");
	php_info_print_table_row(2, "Version", PHP_RAYCAST_VERSION);
	php_info_print_table_end();
}

zend_module_entry raycast_module_entry = {
	STANDARD_MODULE_HEADER,
	"raycast",
	NULL,
	PHP_MINIT(raycast),
	NULL,
	PHP_RINIT(raycast),
	NULL,
	PHP_MINFO(raycast),
	PHP_RAYCAST_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_RAYCAST
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(raycast)
#endif
