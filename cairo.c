/*
 * Copyright (C) 2023 Pengutronix, Marco Felsch <m.felsch@pengutronix.de>
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <cairo.h>

#include "platsch.h"

static struct cairo_ctx {
	struct modeset_dev *dev;
	const char *dir;
	const char *base;
	char filename[128];
} ctx;

struct import_backend {
	int (*detect)(char *filename, size_t filename_sz);
	int (*import_picture)(cairo_t *cr, const char *filename);
};

static int png_import_backend_detect(char *filename, size_t filename_sz)
{
	struct stat s;
	int ret;

	ret = snprintf(filename, filename_sz, "%s/%s.%s", ctx.dir, ctx.base, "png");
	if (ret >= filename_sz) {
		error("Failed to fit filename into buffer\n");
		return -EINVAL;
	}

	return stat(filename, &s);
}

static const char *image_format_to_string(cairo_format_t format)
{
	switch (format) {
	case CAIRO_FORMAT_ARGB32:
		return "ARGB32";
	case CAIRO_FORMAT_RGB24:
		return "RGB24";
	case CAIRO_FORMAT_A8:
		return "A8";
	case CAIRO_FORMAT_A1:
		return "A1";
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 2, 0)
	case CAIRO_FORMAT_RGB16_565:
		return "RGB16_565";
#endif
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
	case CAIRO_FORMAT_RGB30:
		return "RGB30";
#endif
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 17, 2)
	case CAIRO_FORMAT_RGB96F:
		return "RGB96F";
	case CAIRO_FORMAT_RGBA128F:
		return "RGB128F";
#endif
	case CAIRO_FORMAT_INVALID:
	defaul:
		return "invalid";
	}
}

static int png_import_backend_import_picture(cairo_t *cr, const char *filename)
{
	int image_width, image_height, surface_width, surface_height;
	cairo_format_t image_fmt, surface_fmt;
	cairo_surface_t *image, *surface;
	cairo_status_t status;
	cairo_format_t format;
	int ret = 0;

	image = cairo_image_surface_create_from_png(filename);
	status = cairo_surface_status(image);
	if (status != CAIRO_STATUS_SUCCESS) {
		error("Failed to create cairo surface (%s)\n", cairo_status_to_string(status));
		return -EINVAL;
	}

	surface = cairo_get_target(cr);
	image_fmt = cairo_image_surface_get_format(image);
	surface_fmt = cairo_image_surface_get_format(surface);
	if (image_fmt != surface_fmt) {
		error("Error image format (%s) does not match the surface format (%s)\n",
		      image_format_to_string(image_fmt), image_format_to_string(surface_fmt));
		ret = -EINVAL;
		goto out;
	}

	image_width = cairo_image_surface_get_width(image);
	image_height = cairo_image_surface_get_height(image);
	surface_width = cairo_image_surface_get_width(surface);
	surface_height = cairo_image_surface_get_height(surface);

	cairo_scale(cr, (double) surface_width/image_width, (double) surface_height/image_height);
	cairo_set_source_surface(cr, image, 0, 0);
	cairo_paint(cr);

out:
	cairo_surface_destroy(image);

	return ret;
}

static int bin_import_backend_detect(char *filename, size_t filename_sz)
{
	struct modeset_dev *dev = ctx.dev;
	struct stat s;
	int ret;

	ret = snprintf(filename, filename_sz, "%s/%s-%ux%u-%s.bin",
		       ctx.dir, ctx.base, dev->width, dev->height, dev->format->name);
	if (ret >= filename_sz) {
		error("Failed to fit filename into buffer\n");
		return -EINVAL;
	}

	return stat(filename, &s);
}

static int bin_import_backend_import_picture(cairo_t *cr, const char *filename)
{
	cairo_surface_t *surface = cairo_get_target(cr);
	struct modeset_dev *dev = ctx.dev;
	unsigned char *image_buf;
	ssize_t size;
	int fd_src;
	int ret;

	cairo_surface_flush(surface);
	image_buf = cairo_image_surface_get_data(surface);

	fd_src = open(filename, O_RDONLY | O_CLOEXEC);
	if (fd_src < 0) {
		error("Failed to open %s: %m\n", filename);
		return -EINVAL;
	}

	size = readfull(fd_src, image_buf, dev->size);
	if (size < dev->size) {
		if (size < 0)
			error("Failed to read from %s: %m\n", filename);
		else
			error("Could only read %zd/%u bytes from %s\n",
			      size, dev->size, filename);

		close(fd_src);
		return -EINVAL;
	}

	cairo_surface_mark_dirty(surface);

	ret = close(fd_src);
	if (ret < 0)
		error("Failed to close image file\n");

	return ret;
}

static const struct import_backend supported_backends[] = {
	{
		.detect = bin_import_backend_detect,
		.import_picture = bin_import_backend_import_picture,
	}, {
		.detect = png_import_backend_detect,
		.import_picture = png_import_backend_import_picture,
	},
	{ /*sentinel */ }
};

static int cairo_import_picture(cairo_t *cr)
{
	const struct import_backend *backend;
	char filename[128];
	int ret;

	for (backend = supported_backends; backend; backend++)
	{
		ret = backend->detect(filename, sizeof(filename));
		if (!ret)
			break;
	}

	if (!backend) {
		debug("No suitable import backend found found\n");
		return -EINVAL;
	}

	return backend->import_picture(cr, filename);
}

/*
 * TODO: add config file support since env variables are not
 * suitable for text options
 */
static void cairo_draw_text(cairo_t *cr)
{
	unsigned int xpos, ypos, size;
	const char *env;
	char *text;
	int ret;

	/* platsch_overlay_text="(x,y):foo-bar" */
	env = getenv("platsch_overlay_text");
	if (!env)
		return;

	ret = sscanf(env, "(%u,%u,%u):%ms", &xpos, &ypos, &size, &text);
	if (ret < 3 || ret == EOF) {
		error("Error while parsing platsch_overlay_text: %s\n", env);
		return;
	}

	debug("text:%s xpos:%u ypos:%u size:%u\n", text, xpos, ypos, size);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, (double) size);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_move_to(cr, xpos, ypos);
	cairo_show_text(cr, text);

	free(text);
}

static uint32_t convert_to_cairo_format(uint32_t format)
{
	switch (format) {
	case DRM_FORMAT_RGB565:
		return CAIRO_FORMAT_RGB16_565;
	case DRM_FORMAT_XRGB8888:
		return CAIRO_FORMAT_ARGB32;
	}
}

static cairo_t *cairo_init(struct modeset_dev *dev, const char *dir, const char *base)
{
	cairo_surface_t *surface;
	cairo_status_t status;
	cairo_t *cr;

	surface = cairo_image_surface_create_for_data(dev->map,
			convert_to_cairo_format(dev->format->format),
			dev->width, dev->height, dev->stride);
	status = cairo_surface_status(surface);
	if (status != CAIRO_STATUS_SUCCESS) {
		error("Failed to create cairo surface (%s)\n", cairo_status_to_string(status));
		return NULL;
	}

	cr = cairo_create(surface);
	status = cairo_status(cr);
	if (status != CAIRO_STATUS_SUCCESS) {
		error("Failed to create cairo surface (%s)\n", cairo_status_to_string(status));
		cairo_surface_destroy(surface);
		return NULL;
	}

	/* Limit drawing according dev size */
	cairo_rectangle(cr, 0, 0, dev->width, dev->height);
	cairo_clip(cr);

	ctx.dev = dev;
	ctx.dir = dir;
	ctx.base = base;

	return cr;
}

static void cairo_deinit(cairo_t *cr)
{
	cairo_surface_t *surface = cairo_get_target(cr);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

int cairo_draw_buffer(struct modeset_dev *dev, const char *dir, const char *base)
{
	cairo_t *cr;
	int ret;

	cr = cairo_init(dev, dir, base);
	if (!cr)
		return -EINVAL;

	ret = cairo_import_picture(cr);
	if (ret)
		goto out;

	cairo_draw_text(cr);
out:
	cairo_deinit(cr);

	return ret;
}
