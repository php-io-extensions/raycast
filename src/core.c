#include "core.h"

#include <math.h>
#include <string.h>

/*
 * The grid walk (Amanatides and Woo): from the eye's cell, step to whichever
 * cell boundary the ray reaches first, x or y, and look at the cell entered.
 * The cell the ray starts in is never looked at. A tie steps in y.
 */
int raycast_march(const raycast_grid *grid, double x, double y, double dx, double dy, double reach, raycast_hit *hit)
{
	if (!(x >= 0.0) || !(y >= 0.0) || !(x < (double) grid->width) || !(y < (double) grid->height)) {
		return 0;
	}

	/* How far along the ray one whole cell is, in x and in y. */
	double delta_x = dx == 0.0 ? INFINITY : fabs(1.0 / dx);
	double delta_y = dy == 0.0 ? INFINITY : fabs(1.0 / dy);
	if (isinf(delta_x) && isinf(delta_y)) {
		return 0;
	}

	int cell_x = (int) floor(x);
	int cell_y = (int) floor(y);
	int step_x = dx < 0.0 ? -1 : 1;
	int step_y = dy < 0.0 ? -1 : 1;

	/* How far along the ray the next boundary is, in x and in y. */
	double next_x = INFINITY;
	double next_y = INFINITY;
	if (!isinf(delta_x)) {
		next_x = dx < 0.0 ? (x - (double) cell_x) * delta_x : ((double) (cell_x + 1) - x) * delta_x;
	}
	if (!isinf(delta_y)) {
		next_y = dy < 0.0 ? (y - (double) cell_y) * delta_y : ((double) (cell_y + 1) - y) * delta_y;
	}

	for (;;) {
		double distance;
		int crossed_x;

		if (next_x < next_y) {
			distance = next_x;
			next_x += delta_x;
			cell_x += step_x;
			crossed_x = 1;
		} else {
			distance = next_y;
			next_y += delta_y;
			cell_y += step_y;
			crossed_x = 0;
		}

		if (distance > reach || cell_x < 0 || cell_y < 0 || cell_x >= grid->width || cell_y >= grid->height) {
			return 0;
		}

		uint8_t wall = grid->cells[(size_t) cell_y * (size_t) grid->width + (size_t) cell_x];
		if (wall == 0) {
			continue;
		}

		double where = crossed_x ? y + distance * dy : x + distance * dx;

		hit->distance = distance;
		hit->along = where - floor(where);
		hit->x = cell_x;
		hit->y = cell_y;
		hit->wall = wall;
		if (crossed_x) {
			hit->face = step_x > 0 ? RAYCAST_FACE_WEST : RAYCAST_FACE_EAST;
		} else {
			hit->face = step_y > 0 ? RAYCAST_FACE_NORTH : RAYCAST_FACE_SOUTH;
		}

		return 1;
	}
}

static void raycast_put_double(double value, uint8_t *out)
{
	uint64_t bits;

	memcpy(&bits, &value, sizeof(bits));
	for (int i = 0; i < 8; i++) {
		out[i] = (uint8_t) (bits >> (8 * i));
	}
}

void raycast_pack(const raycast_hit *hit, uint8_t *out)
{
	if (hit == NULL) {
		memset(out, 0, RAYCAST_HIT_BYTES);
		return;
	}
	raycast_put_double(hit->distance, out);
	raycast_put_double(hit->along, out + 8);
	out[16] = (uint8_t) (hit->x & 0xFF);
	out[17] = (uint8_t) ((hit->x >> 8) & 0xFF);
	out[18] = (uint8_t) (hit->y & 0xFF);
	out[19] = (uint8_t) ((hit->y >> 8) & 0xFF);
	out[20] = (uint8_t) hit->wall;
	out[21] = (uint8_t) hit->face;
}

/* Column `column` of `columns` looks through its own centre: the direction plus the plane times -1..1. */
static int raycast_column(const raycast_grid *grid, const raycast_camera *camera, int column, int columns, double reach, raycast_hit *hit)
{
	double across = (double) (2 * column + 1) / (double) columns - 1.0;

	return raycast_march(grid, camera->x, camera->y,
		camera->dir_x + camera->plane_x * across, camera->dir_y + camera->plane_y * across, reach, hit);
}

void raycast_fan(const raycast_grid *grid, const raycast_camera *camera, int columns, double reach, uint8_t *out)
{
	for (int column = 0; column < columns; column++) {
		raycast_hit hit;

		raycast_pack(raycast_column(grid, camera, column, columns, reach, &hit) ? &hit : NULL, out + (size_t) column * RAYCAST_HIT_BYTES);
	}
}

/* Where red, green, blue and alpha go in a pixel's four bytes, by RAYCAST_ORDER_*. */
static const uint8_t raycast_orders[RAYCAST_ORDERS][4] = {
	{0, 1, 2, 3},   /* RGBA */
	{2, 1, 0, 3},   /* BGRA */
	{1, 2, 3, 0},   /* ARGB */
	{3, 2, 1, 0},   /* ABGR */
};

static void raycast_put_pixel(uint8_t *out, const uint8_t *at, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	out[at[0]] = red;
	out[at[1]] = green;
	out[at[2]] = blue;
	out[at[3]] = alpha;
}

int raycast_view(const raycast_grid *grid, const raycast_camera *camera, int width, int height, double reach,
	const raycast_wall *walls, uint32_t ceiling, uint32_t floor, int shade, uint8_t *out)
{
	return raycast_view_rows(grid, camera, width, height, reach, walls, ceiling, floor, shade, out, (size_t) width * 4, RAYCAST_ORDER_RGBA);
}

int raycast_view_missing(const raycast_grid *grid, const raycast_camera *camera, int width, double reach, const raycast_wall *walls)
{
	for (int column = 0; column < width; column++) {
		raycast_hit hit;

		if (raycast_column(grid, camera, column, width, reach, &hit) && walls[hit.wall].kind == RAYCAST_WALL_NONE) {
			return hit.wall;
		}
	}

	return 0;
}

int raycast_view_rows(const raycast_grid *grid, const raycast_camera *camera, int width, int height, double reach,
	const raycast_wall *walls, uint32_t ceiling, uint32_t floor, int shade, uint8_t *out, size_t stride, int order)
{
	int horizon = height / 2;
	const uint8_t *at = raycast_orders[order];

	/* The ceiling above the horizon, the floor from it down. */
	for (int row = 0; row < height; row++) {
		uint32_t colour = row < horizon ? ceiling : floor;
		uint8_t *pixel = out + (size_t) row * stride;

		for (int column = 0; column < width; column++, pixel += 4) {
			raycast_put_pixel(pixel, at, (uint8_t) (colour >> 24), (uint8_t) (colour >> 16), (uint8_t) (colour >> 8), (uint8_t) colour);
		}
	}

	for (int column = 0; column < width; column++) {
		raycast_hit hit;

		if (!raycast_column(grid, camera, column, width, reach, &hit)) {
			continue;
		}

		const raycast_wall *wall = &walls[hit.wall];
		if (wall->kind == RAYCAST_WALL_NONE) {
			return hit.wall;
		}

		/* The wall's line is as tall as the view at distance 1, centred on the horizon. */
		double tall = hit.distance > 0.0 ? (double) height / hit.distance : (double) RAYCAST_MAX_LINE;
		if (!(tall < (double) RAYCAST_MAX_LINE)) {
			tall = (double) RAYCAST_MAX_LINE;
		}
		int line = (int) tall;
		int start = horizon - line / 2;
		int from = start < 0 ? 0 : start;
		int to = start + line > height ? height : start + line;

		/* The north and south faces are darkened: what tells one wall from the next at a corner. */
		int dim = (hit.face == RAYCAST_FACE_NORTH || hit.face == RAYCAST_FACE_SOUTH) ? shade : 255;

		if (wall->kind == RAYCAST_WALL_COLOUR) {
			uint8_t pixel[4];

			raycast_put_pixel(pixel, at,
				(uint8_t) ((uint8_t) (wall->colour >> 24) * dim / 255),
				(uint8_t) ((uint8_t) (wall->colour >> 16) * dim / 255),
				(uint8_t) ((uint8_t) (wall->colour >> 8) * dim / 255),
				(uint8_t) wall->colour);
			for (int row = from; row < to; row++) {
				memcpy(out + (size_t) row * stride + (size_t) column * 4, pixel, 4);
			}
			continue;
		}

		/* The texture's column under the hit, read left to right as the face is seen from outside. */
		int texel_x = (int) (hit.along * (double) wall->width);
		if (texel_x > wall->width - 1) {
			texel_x = wall->width - 1;
		}
		if (hit.face == RAYCAST_FACE_EAST || hit.face == RAYCAST_FACE_NORTH) {
			texel_x = wall->width - 1 - texel_x;
		}

		for (int row = from; row < to; row++) {
			int texel_y = (int) ((int64_t) (row - start) * (int64_t) wall->height / (int64_t) line);
			const uint8_t *texel = wall->pixels + ((size_t) texel_y * (size_t) wall->width + (size_t) texel_x) * 4;
			uint8_t *pixel = out + (size_t) row * stride + (size_t) column * 4;

			raycast_put_pixel(pixel, at, (uint8_t) (texel[0] * dim / 255), (uint8_t) (texel[1] * dim / 255), (uint8_t) (texel[2] * dim / 255), texel[3]);
		}
	}

	return 0;
}
