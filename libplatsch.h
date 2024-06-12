#ifndef __LIBPLATSCH_H__
#define __LIBPLATSCH_H__

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#define platsch_debug(fmt, ...) printf("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define platsch_error(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#define PLATSCH_MAX_FILENAME_LENGTH 128
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

struct modeset_dev *platsch_init(void);
void platsch_draw(struct modeset_dev *dev, const char *dir, const char *base);
void platsch_drmDropMaster(void);
void platsch_setup_display(struct modeset_dev *dev);

#endif
