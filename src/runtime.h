/*
 * What the class source shares with the module: the Zend object that holds a
 * grid's cells, and its class entry.
 */

#ifndef PHPRAYCAST_RUNTIME_H
#define PHPRAYCAST_RUNTIME_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_raycast.h"
#include "core.h"

typedef struct {
	raycast_grid grid;
	zend_string *cells;   /* what grid.cells points into; NULL until __construct has run */
	zend_object std;
} phpraycast_grid;

static zend_always_inline phpraycast_grid *phpraycast_grid_from(zend_object *obj)
{
	return (phpraycast_grid *) ((char *) obj - XtOffsetOf(phpraycast_grid, std));
}

extern zend_class_entry *phpraycast_ce_RaycastGrid;

void phpraycast_register_RaycastGrid(void);

#endif
