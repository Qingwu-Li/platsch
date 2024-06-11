#ifndef __LIBPLATSCH_H__
#define __LIBPLATSCH_H__

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "libplatsch.h"


#define debug(fmt, ...) printf("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define error(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

struct platsch_format {
	uint32_t format;
	uint32_t bpp;
	const char *name;
};

struct modeset_dev {
	struct modeset_dev *next;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t size;
	const struct platsch_format *format;
	uint32_t handle;
	void *map;
	bool setmode;
	drmModeModeInfo mode;
	uint32_t fb_id;
	uint32_t conn_id;
	uint32_t crtc_id;
};

ssize_t readfull(int fd, void *buf, size_t count);
struct modeset_dev * init(void);

int draw(struct modeset_dev *dev, const char *dir, const char *base);
int finish(void);
int update_display(struct modeset_dev *dev);

#ifndef HAVE_CAIRO
static inline int cairo_draw_buffer(struct modeset_dev *dev, const char *dir, const char *base)
{
	printf("cairo_draw_buffer do nothing %s %s\n", dir, base);
	return -ENOTSUP;
}
#else

#include <cairo.h>
int cairo_draw_buffer(struct modeset_dev *dev, const char *dir, const char *base);
cairo_t *cairo_init(struct modeset_dev *dev, const char *dir, const char *base);

#endif /* HAVE_CAIRO */

#endif /* __LIBPLATSCH_H__ */
