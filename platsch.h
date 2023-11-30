#ifndef __PLATSCH_H__
#define __PLATSCH_H__

#include <stdint.h>
#include <xf86drmMode.h>

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

#ifdef HAVE_CAIRO
int cairo_draw_buffer(struct modeset_dev *dev, const char *dir, const char *base);
#else
static inline int cairo_draw_buffer(struct modeset_dev *dev, const char *dir, const char *base)
{
	return -ENOTSUP;
}
#endif /* HAVE_CAIRO */

#endif /* __PLATSCH_H__ */
