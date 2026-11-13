#include "runtime.h"
#include "zend_exceptions.h"
#include "../stubs/RaycastGrid_arginfo.h"

#include <math.h>

zend_class_entry *phpraycast_ce_RaycastGrid;

static zend_object_handlers phpraycast_grid_handlers;

static zend_object *phpraycast_grid_create(zend_class_entry *ce)
{
	phpraycast_grid *self = zend_object_alloc(sizeof(phpraycast_grid), ce);

	zend_object_std_init(&self->std, ce);
	object_properties_init(&self->std, ce);

	return &self->std;
}

static void phpraycast_grid_free(zend_object *object)
{
	phpraycast_grid *self = phpraycast_grid_from(object);

	if (self->cells != NULL) {
		zend_string_release(self->cells);
	}
	zend_object_std_dtor(object);
}

void phpraycast_register_RaycastGrid(void)
{
	phpraycast_ce_RaycastGrid = register_class_RaycastGrid();
	phpraycast_ce_RaycastGrid->create_object = phpraycast_grid_create;

	memcpy(&phpraycast_grid_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	phpraycast_grid_handlers.offset = XtOffsetOf(phpraycast_grid, std);
	phpraycast_grid_handlers.free_obj = phpraycast_grid_free;
	phpraycast_grid_handlers.clone_obj = NULL;
	phpraycast_grid_handlers.compare = zend_objects_not_comparable;
	phpraycast_ce_RaycastGrid->default_object_handlers = &phpraycast_grid_handlers;
}

#define PHPRAYCAST_THIS(self) \
	phpraycast_grid *self = phpraycast_grid_from(Z_OBJ_P(ZEND_THIS)); \
	if (UNEXPECTED(self->cells == NULL)) { \
		zend_throw_error(NULL, "RaycastGrid has not been constructed"); \
		RETURN_THROWS(); \
	}

/* A finite number within RAYCAST_LIMIT; otherwise a ValueError naming it. */
static bool phpraycast_position(double value, const char *what)
{
	if (!isfinite(value)) {
		zend_value_error("%s is not a finite number.", what);
		return false;
	}
	if (fabs(value) > (double) RAYCAST_LIMIT) {
		zend_value_error("%s %.17g is past ±%d.", what, value, RAYCAST_LIMIT);
		return false;
	}

	return true;
}

/* Two finite numbers; otherwise a ValueError naming the vector. */
static bool phpraycast_vector(double x, double y, const char *what)
{
	if (!isfinite(x) || !isfinite(y)) {
		zend_value_error("%s is not two finite numbers.", what);
		return false;
	}

	return true;
}

/* Two finite numbers, not both 0; otherwise a ValueError naming the vector. */
static bool phpraycast_direction(double x, double y, const char *what)
{
	if (!phpraycast_vector(x, y, what)) {
		return false;
	}
	if (x == 0.0 && y == 0.0) {
		zend_value_error("%s is (0, 0): a ray needs somewhere to go.", what);
		return false;
	}

	return true;
}

/* Above 0, INF allowed; otherwise a ValueError. */
static bool phpraycast_reach(double reach)
{
	if (!(reach > 0.0)) {
		zend_value_error("The reach is above 0, or INF.");
		return false;
	}

	return true;
}

/* 0..0xFFFFFFFF; otherwise a ValueError naming the colour. */
static bool phpraycast_colour(zend_long value, const char *what)
{
	if (value < 0 || value > (zend_long) 0xFFFFFFFF) {
		zend_value_error("%s is a colour 0xRRGGBBAA.", what);
		return false;
	}

	return true;
}

ZEND_METHOD(RaycastGrid, __construct)
{
	zend_long width, height;
	zend_string *cells;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_LONG(width)
		Z_PARAM_LONG(height)
		Z_PARAM_STR(cells)
	ZEND_PARSE_PARAMETERS_END();

	phpraycast_grid *self = phpraycast_grid_from(Z_OBJ_P(ZEND_THIS));
	if (self->cells != NULL) {
		zend_throw_error(NULL, "RaycastGrid is already constructed");
		RETURN_THROWS();
	}
	if (width < 1 || height < 1 || width > RAYCAST_MAX_SIDE || height > RAYCAST_MAX_SIDE) {
		zend_value_error("A grid " ZEND_LONG_FMT "x" ZEND_LONG_FMT " has a side outside 1..%d.", width, height, RAYCAST_MAX_SIDE);
		RETURN_THROWS();
	}
	if (ZSTR_LEN(cells) != (size_t) width * (size_t) height) {
		zend_value_error("A grid " ZEND_LONG_FMT "x" ZEND_LONG_FMT " holds " ZEND_LONG_FMT " cells, one byte each; got %zu bytes.",
			width, height, width * height, ZSTR_LEN(cells));
		RETURN_THROWS();
	}

	self->cells = zend_string_copy(cells);
	self->grid.width = (int) width;
	self->grid.height = (int) height;
	self->grid.cells = (const uint8_t *) ZSTR_VAL(self->cells);
}

ZEND_METHOD(RaycastGrid, width)
{
	ZEND_PARSE_PARAMETERS_NONE();
	PHPRAYCAST_THIS(self);

	RETURN_LONG(self->grid.width);
}

ZEND_METHOD(RaycastGrid, height)
{
	ZEND_PARSE_PARAMETERS_NONE();
	PHPRAYCAST_THIS(self);

	RETURN_LONG(self->grid.height);
}

ZEND_METHOD(RaycastGrid, cells)
{
	ZEND_PARSE_PARAMETERS_NONE();
	PHPRAYCAST_THIS(self);

	RETURN_STR_COPY(self->cells);
}

ZEND_METHOD(RaycastGrid, cast)
{
	double x, y, dx, dy, reach = INFINITY;

	ZEND_PARSE_PARAMETERS_START(4, 5)
		Z_PARAM_DOUBLE(x)
		Z_PARAM_DOUBLE(y)
		Z_PARAM_DOUBLE(dx)
		Z_PARAM_DOUBLE(dy)
		Z_PARAM_OPTIONAL
		Z_PARAM_DOUBLE(reach)
	ZEND_PARSE_PARAMETERS_END();
	PHPRAYCAST_THIS(self);

	if (!phpraycast_position(x, "x") || !phpraycast_position(y, "y") || !phpraycast_direction(dx, dy, "The ray") || !phpraycast_reach(reach)) {
		RETURN_THROWS();
	}

	zend_string *out = zend_string_alloc(RAYCAST_HIT_BYTES, 0);
	raycast_hit hit;

	raycast_pack(raycast_march(&self->grid, x, y, dx, dy, reach, &hit) ? &hit : NULL, (uint8_t *) ZSTR_VAL(out));
	ZSTR_VAL(out)[RAYCAST_HIT_BYTES] = '\0';

	RETURN_NEW_STR(out);
}

/* The eye, where it looks and its plane, each checked; false with a ValueError. */
static bool phpraycast_camera(const raycast_camera *camera)
{
	return phpraycast_position(camera->x, "x") && phpraycast_position(camera->y, "y")
		&& phpraycast_direction(camera->dir_x, camera->dir_y, "The direction")
		&& phpraycast_vector(camera->plane_x, camera->plane_y, "The plane");
}

ZEND_METHOD(RaycastGrid, fan)
{
	raycast_camera camera;
	zend_long columns;
	double reach = INFINITY;

	ZEND_PARSE_PARAMETERS_START(7, 8)
		Z_PARAM_DOUBLE(camera.x)
		Z_PARAM_DOUBLE(camera.y)
		Z_PARAM_DOUBLE(camera.dir_x)
		Z_PARAM_DOUBLE(camera.dir_y)
		Z_PARAM_DOUBLE(camera.plane_x)
		Z_PARAM_DOUBLE(camera.plane_y)
		Z_PARAM_LONG(columns)
		Z_PARAM_OPTIONAL
		Z_PARAM_DOUBLE(reach)
	ZEND_PARSE_PARAMETERS_END();
	PHPRAYCAST_THIS(self);

	if (!phpraycast_camera(&camera) || !phpraycast_reach(reach)) {
		RETURN_THROWS();
	}
	if (columns < 1 || columns > RAYCAST_MAX_SIDE) {
		zend_argument_value_error(7, "must be 1..%d", RAYCAST_MAX_SIDE);
		RETURN_THROWS();
	}

	zend_string *out = zend_string_alloc((size_t) columns * RAYCAST_HIT_BYTES, 0);

	raycast_fan(&self->grid, &camera, (int) columns, reach, (uint8_t *) ZSTR_VAL(out));
	ZSTR_VAL(out)[ZSTR_LEN(out)] = '\0';

	RETURN_NEW_STR(out);
}

/* A texture entry [RGBA8 bytes, width, height] into `wall`; false with a ValueError naming the wall. */
static bool phpraycast_texture(HashTable *entry, zend_ulong id, raycast_wall *wall)
{
	zval *bytes, *width, *height;

	if (!zend_array_is_list(entry) || zend_hash_num_elements(entry) != 3) {
		zend_value_error("Wall " ZEND_ULONG_FMT " is a colour or [RGBA8 bytes, width, height].", id);
		return false;
	}
	bytes = zend_hash_index_find(entry, 0);
	width = zend_hash_index_find(entry, 1);
	height = zend_hash_index_find(entry, 2);
	ZVAL_DEREF(bytes);
	ZVAL_DEREF(width);
	ZVAL_DEREF(height);
	if (Z_TYPE_P(bytes) != IS_STRING || Z_TYPE_P(width) != IS_LONG || Z_TYPE_P(height) != IS_LONG) {
		zend_value_error("Wall " ZEND_ULONG_FMT " is a colour or [RGBA8 bytes, width, height].", id);
		return false;
	}
	if (Z_LVAL_P(width) < 1 || Z_LVAL_P(height) < 1 || Z_LVAL_P(width) > RAYCAST_MAX_SIDE || Z_LVAL_P(height) > RAYCAST_MAX_SIDE) {
		zend_value_error("Wall " ZEND_ULONG_FMT "'s texture " ZEND_LONG_FMT "x" ZEND_LONG_FMT " has a side outside 1..%d.",
			id, Z_LVAL_P(width), Z_LVAL_P(height), RAYCAST_MAX_SIDE);
		return false;
	}
	if (Z_STRLEN_P(bytes) != (size_t) Z_LVAL_P(width) * (size_t) Z_LVAL_P(height) * 4) {
		zend_value_error("Wall " ZEND_ULONG_FMT "'s texture " ZEND_LONG_FMT "x" ZEND_LONG_FMT " holds " ZEND_LONG_FMT " bytes of RGBA8; got %zu.",
			id, Z_LVAL_P(width), Z_LVAL_P(height), Z_LVAL_P(width) * Z_LVAL_P(height) * 4, Z_STRLEN_P(bytes));
		return false;
	}

	wall->kind = RAYCAST_WALL_TEXTURE;
	wall->pixels = (const uint8_t *) Z_STRVAL_P(bytes);
	wall->width = (int) Z_LVAL_P(width);
	wall->height = (int) Z_LVAL_P(height);

	return true;
}

/* The walls by id into `walls` (RAYCAST_WALLS entries, cleared first); false with a ValueError. */
static bool phpraycast_walls(HashTable *list, raycast_wall *walls)
{
	zend_ulong id;
	zend_string *name;
	zval *entry;

	memset(walls, 0, RAYCAST_WALLS * sizeof(raycast_wall));
	ZEND_HASH_FOREACH_KEY_VAL(list, id, name, entry) {
		if (name != NULL || id < 1 || id >= RAYCAST_WALLS) {
			zend_value_error("The walls are keyed by wall id, 1..%d.", RAYCAST_WALLS - 1);
			return false;
		}
		ZVAL_DEREF(entry);
		if (Z_TYPE_P(entry) == IS_LONG) {
			char what[32];

			snprintf(what, sizeof(what), "Wall " ZEND_ULONG_FMT, id);
			if (!phpraycast_colour(Z_LVAL_P(entry), what)) {
				return false;
			}
			walls[id].kind = RAYCAST_WALL_COLOUR;
			walls[id].colour = (uint32_t) Z_LVAL_P(entry);
		} else if (Z_TYPE_P(entry) == IS_ARRAY) {
			if (!phpraycast_texture(Z_ARRVAL_P(entry), id, &walls[id])) {
				return false;
			}
		} else {
			zend_value_error("Wall " ZEND_ULONG_FMT " is a colour or [RGBA8 bytes, width, height].", id);
			return false;
		}
	} ZEND_HASH_FOREACH_END();

	return true;
}

ZEND_METHOD(RaycastGrid, view)
{
	raycast_camera camera;
	zend_long width, height, ceiling, floor, shade = 255;
	HashTable *list;
	double reach = INFINITY;

	ZEND_PARSE_PARAMETERS_START(11, 13)
		Z_PARAM_DOUBLE(camera.x)
		Z_PARAM_DOUBLE(camera.y)
		Z_PARAM_DOUBLE(camera.dir_x)
		Z_PARAM_DOUBLE(camera.dir_y)
		Z_PARAM_DOUBLE(camera.plane_x)
		Z_PARAM_DOUBLE(camera.plane_y)
		Z_PARAM_LONG(width)
		Z_PARAM_LONG(height)
		Z_PARAM_ARRAY_HT(list)
		Z_PARAM_LONG(ceiling)
		Z_PARAM_LONG(floor)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(shade)
		Z_PARAM_DOUBLE(reach)
	ZEND_PARSE_PARAMETERS_END();
	PHPRAYCAST_THIS(self);

	if (!phpraycast_camera(&camera) || !phpraycast_reach(reach)) {
		RETURN_THROWS();
	}
	if (width < 1 || width > RAYCAST_MAX_VIEW) {
		zend_argument_value_error(7, "must be 1..%d", RAYCAST_MAX_VIEW);
		RETURN_THROWS();
	}
	if (height < 1 || height > RAYCAST_MAX_VIEW) {
		zend_argument_value_error(8, "must be 1..%d", RAYCAST_MAX_VIEW);
		RETURN_THROWS();
	}
	if (!phpraycast_colour(ceiling, "The ceiling") || !phpraycast_colour(floor, "The floor")) {
		RETURN_THROWS();
	}
	if (shade < 0 || shade > 255) {
		zend_argument_value_error(12, "must be 0..255");
		RETURN_THROWS();
	}

	raycast_wall walls[RAYCAST_WALLS];
	if (!phpraycast_walls(list, walls)) {
		RETURN_THROWS();
	}

	zend_string *out = zend_string_alloc((size_t) width * (size_t) height * 4, 0);
	int missing = raycast_view(&self->grid, &camera, (int) width, (int) height, reach, walls,
		(uint32_t) ceiling, (uint32_t) floor, (int) shade, (uint8_t *) ZSTR_VAL(out));

	if (missing != 0) {
		zend_string_release(out);
		zend_value_error("Wall %d has no colour or texture.", missing);
		RETURN_THROWS();
	}
	ZSTR_VAL(out)[ZSTR_LEN(out)] = '\0';

	RETURN_NEW_STR(out);
}

ZEND_METHOD(RaycastGrid, viewInto)
{
	raycast_camera camera;
	zend_long width, height, ceiling, floor, address, stride, shade = 255, order = RAYCAST_ORDER_RGBA;
	HashTable *list;
	double reach = INFINITY;

	ZEND_PARSE_PARAMETERS_START(13, 16)
		Z_PARAM_DOUBLE(camera.x)
		Z_PARAM_DOUBLE(camera.y)
		Z_PARAM_DOUBLE(camera.dir_x)
		Z_PARAM_DOUBLE(camera.dir_y)
		Z_PARAM_DOUBLE(camera.plane_x)
		Z_PARAM_DOUBLE(camera.plane_y)
		Z_PARAM_LONG(width)
		Z_PARAM_LONG(height)
		Z_PARAM_ARRAY_HT(list)
		Z_PARAM_LONG(ceiling)
		Z_PARAM_LONG(floor)
		Z_PARAM_LONG(address)
		Z_PARAM_LONG(stride)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(shade)
		Z_PARAM_DOUBLE(reach)
		Z_PARAM_LONG(order)
	ZEND_PARSE_PARAMETERS_END();
	PHPRAYCAST_THIS(self);

	if (!phpraycast_camera(&camera) || !phpraycast_reach(reach)) {
		RETURN_THROWS();
	}
	if (width < 1 || width > RAYCAST_MAX_VIEW) {
		zend_argument_value_error(7, "must be 1..%d", RAYCAST_MAX_VIEW);
		RETURN_THROWS();
	}
	if (height < 1 || height > RAYCAST_MAX_VIEW) {
		zend_argument_value_error(8, "must be 1..%d", RAYCAST_MAX_VIEW);
		RETURN_THROWS();
	}
	if (!phpraycast_colour(ceiling, "The ceiling") || !phpraycast_colour(floor, "The floor")) {
		RETURN_THROWS();
	}
	if (shade < 0 || shade > 255) {
		zend_argument_value_error(14, "must be 0..255");
		RETURN_THROWS();
	}

	raycast_wall walls[RAYCAST_WALLS];
	if (!phpraycast_walls(list, walls)) {
		RETURN_THROWS();
	}

	if (order < 0 || order >= RAYCAST_ORDERS) {
		zend_argument_value_error(16, "must be a RAYCAST_ORDER_* constant");
		RETURN_THROWS();
	}
	if (address == 0) {
		zend_argument_value_error(12, "must be the address of the first pixel, 0 given");
		RETURN_THROWS();
	}
	if (stride < width * 4) {
		zend_argument_value_error(13, "must be at least width * 4 (" ZEND_LONG_FMT "), " ZEND_LONG_FMT " given", width * 4, stride);
		RETURN_THROWS();
	}

	/* Asked first: a wall with no entry is refused before a byte of the caller's memory is written. */
	int missing = raycast_view_missing(&self->grid, &camera, (int) width, reach, walls);
	if (missing != 0) {
		zend_value_error("Wall %d has no colour or texture.", missing);
		RETURN_THROWS();
	}

	raycast_view_rows(&self->grid, &camera, (int) width, (int) height, reach, walls,
		(uint32_t) ceiling, (uint32_t) floor, (int) shade, (uint8_t *) (uintptr_t) address, (size_t) stride, (int) order);
}
