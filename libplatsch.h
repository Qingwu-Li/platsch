#ifndef __LIBPLATSCH_H__
#define __LIBPLATSCH_H__

#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "libplatsch.h"
#include <unistd.h>



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

static const struct platsch_format platsch_formats[] = {
	{ DRM_FORMAT_RGB565, 16, "RGB565" }, /* default */
	{ DRM_FORMAT_XRGB8888, 32, "XRGB8888" },
};

ssize_t readfull(int fd, void *buf, size_t count);
struct modeset_dev *init(void);
void draw(struct modeset_dev *dev, const char *dir, const char *base);
void finish(void);
void redirect_stdfd(void);
void update_display(struct modeset_dev *dev);

#endif