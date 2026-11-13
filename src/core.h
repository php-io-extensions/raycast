/*
 * The casting core: rays through a grid of cells, answered as hit records or
 * as a picture of what a camera sees. Plain C; nothing here knows PHP.
 *
 * A grid is width x height cells, one byte each, rows top first. 0 is open;
 * 1..255 is a wall and the byte is its id. x grows east, y grows south.
 *
 * A hit is 22 bytes, little-endian: distance (float64), along (float64), x
 * (uint16), y (uint16), wall (uint8), face (uint8). A miss is 22 zero bytes.
 *
 * The arithmetic is IEEE double with + - * /, fabs and floor only, in a fixed
 * order, built with -ffp-contract=off: every platform gets the same bits, and
 * so does any other implementation that keeps the same order.
 */

#ifndef PHPRAYCAST_CORE_H
#define PHPRAYCAST_CORE_H

#include <stddef.h>
#include <stdint.h>

#define RAYCAST_HIT_BYTES 22
#define RAYCAST_FACE_WEST 0    /* the ray was travelling east */
#define RAYCAST_FACE_EAST 1    /* west */
#define RAYCAST_FACE_NORTH 2   /* south */
#define RAYCAST_FACE_SOUTH 3   /* north */
#define RAYCAST_MAX_SIDE 65535     /* a grid's side, and a fan's columns */
#define RAYCAST_MAX_VIEW 16384     /* a view's side */

/* The order of a pixel's four bytes in memory a view is written into. */
#define RAYCAST_ORDER_RGBA 0
#define RAYCAST_ORDER_BGRA 1
#define RAYCAST_ORDER_ARGB 2
#define RAYCAST_ORDER_ABGR 3
#define RAYCAST_ORDERS 4
#define RAYCAST_LIMIT 1073741824   /* 2^30: a position past it is refused */
#define RAYCAST_MAX_LINE 16777216  /* 2^24: the tallest wall line a view draws, before it is cut to the view */
#define RAYCAST_WALLS 256

typedef struct {
	int width, height;
	const uint8_t *cells;   /* width * height bytes */
} raycast_grid;

typedef struct {
	double distance;   /* the multiple of the ray's vector at the face */
	double along;      /* where on the face, 0 <= along < 1, measured toward +y (west, east) or +x (north, south) */
	int x, y;          /* the cell */
	int wall;          /* its byte, 1..255 */
	int face;          /* RAYCAST_FACE_* */
} raycast_hit;

typedef struct {
	double x, y;               /* the eye, in cells */
	double dir_x, dir_y;       /* where it looks */
	double plane_x, plane_y;   /* half the screen's width, as a vector */
} raycast_camera;

#define RAYCAST_WALL_NONE 0
#define RAYCAST_WALL_COLOUR 1
#define RAYCAST_WALL_TEXTURE 2

typedef struct {
	int kind;                /* RAYCAST_WALL_* */
	uint32_t colour;         /* 0xRRGGBBAA */
	const uint8_t *pixels;   /* RGBA8, width * height * 4 bytes, top row first */
	int width, height;
} raycast_wall;

/* One ray from (x, y) along (dx, dy), at most `reach` multiples of it: 1 and `hit` filled, or 0 for a miss. */
int raycast_march(const raycast_grid *grid, double x, double y, double dx, double dy, double reach, raycast_hit *hit);

/* A hit as its 22 bytes; NULL writes a miss. */
void raycast_pack(const raycast_hit *hit, uint8_t *out);

/* One ray a column, left to right: `columns` records into `out`. */
void raycast_fan(const raycast_grid *grid, const raycast_camera *camera, int columns, double reach, uint8_t *out);

/*
 * What the camera sees as RGBA8, width x height, top row first, into `out`
 * (width * height * 4 bytes). `walls` has RAYCAST_WALLS entries, by wall id.
 * Answers 0, or the id of a wall a ray hit that has no entry: `out` is then
 * not a picture.
 */
int raycast_view(const raycast_grid *grid, const raycast_camera *camera, int width, int height, double reach,
	const raycast_wall *walls, uint32_t ceiling, uint32_t floor, int shade, uint8_t *out);

/*
 * raycast_view() into rows `stride` bytes apart (stride >= width * 4): the
 * picture inside a larger surface. The bytes of a row past width * 4 are not
 * touched. `order` is a RAYCAST_ORDER_*: where a pixel's four bytes go.
 */
int raycast_view_rows(const raycast_grid *grid, const raycast_camera *camera, int width, int height, double reach,
	const raycast_wall *walls, uint32_t ceiling, uint32_t floor, int shade, uint8_t *out, size_t stride, int order);

/* The id of the first wall a view's rays hit that has no entry, or 0: asked before painting where nothing may be half written. */
int raycast_view_missing(const raycast_grid *grid, const raycast_camera *camera, int width, double reach, const raycast_wall *walls);

#endif
