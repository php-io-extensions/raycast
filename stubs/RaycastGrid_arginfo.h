/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f6cf94cd41989c494e442189ffd77f830ecdcdbf */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_RaycastGrid___construct, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cells, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_RaycastGrid_width, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_RaycastGrid_height arginfo_class_RaycastGrid_width

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_RaycastGrid_cells, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_RaycastGrid_cast, 0, 4, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dx, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dy, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reach, IS_DOUBLE, 0, "INF")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_RaycastGrid_fan, 0, 7, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dirX, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dirY, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, planeX, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, planeY, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, columns, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reach, IS_DOUBLE, 0, "INF")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_RaycastGrid_view, 0, 11, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dirX, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dirY, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, planeX, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, planeY, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, walls, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, ceiling, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, floor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, shade, IS_LONG, 0, "255")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reach, IS_DOUBLE, 0, "INF")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_RaycastGrid_viewInto, 0, 13, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dirX, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, dirY, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, planeX, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, planeY, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, walls, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, ceiling, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, floor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, stride, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, shade, IS_LONG, 0, "255")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reach, IS_DOUBLE, 0, "INF")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, order, IS_LONG, 0, "RAYCAST_ORDER_RGBA")
ZEND_END_ARG_INFO()

ZEND_METHOD(RaycastGrid, __construct);
ZEND_METHOD(RaycastGrid, width);
ZEND_METHOD(RaycastGrid, height);
ZEND_METHOD(RaycastGrid, cells);
ZEND_METHOD(RaycastGrid, cast);
ZEND_METHOD(RaycastGrid, fan);
ZEND_METHOD(RaycastGrid, view);
ZEND_METHOD(RaycastGrid, viewInto);

static const zend_function_entry class_RaycastGrid_methods[] = {
	ZEND_ME(RaycastGrid, __construct, arginfo_class_RaycastGrid___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, width, arginfo_class_RaycastGrid_width, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, height, arginfo_class_RaycastGrid_height, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, cells, arginfo_class_RaycastGrid_cells, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, cast, arginfo_class_RaycastGrid_cast, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, fan, arginfo_class_RaycastGrid_fan, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, view, arginfo_class_RaycastGrid_view, ZEND_ACC_PUBLIC)
	ZEND_ME(RaycastGrid, viewInto, arginfo_class_RaycastGrid_viewInto, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_RaycastGrid(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RaycastGrid", class_RaycastGrid_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}
