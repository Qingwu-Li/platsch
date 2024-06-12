
#include <cairo.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "libplatsch.h"
#include "spinner_conf.h"

typedef struct spinner {
	cairo_format_t format;
	cairo_surface_t *background_surface;
	cairo_surface_t *symbol_surface;
	cairo_t *disp_cr;
	int display_height;
	int display_width;
	int symbol_frames;
	int symbol_height;
	int symbol_width;
	int symbol_x;
	int symbol_y;
	float rotate_step;
	struct modeset_dev *dev;
	struct spinner *next;
} spinner_t;

void draw_background(cairo_t *cr, spinner_t *data)
{
	cairo_set_source_surface(cr, data->background_surface, -data->symbol_x, -data->symbol_y);
	cairo_paint(cr);
	cairo_save(cr);
}

void on_draw_sequence_animation(cairo_t *cr, spinner_t *data)
{
	static int frame;

	int x = -data->symbol_width / 2;
	int y = -data->symbol_height / 2;

	int width = cairo_image_surface_get_width(data->symbol_surface);
	int height = cairo_image_surface_get_height(data->symbol_surface);

	if (width > height)
		x = -data->symbol_width / 2 - frame * data->symbol_width;
	else
		y = -data->symbol_height / 2 - frame * data->symbol_height;

	draw_background(cr, data);
	cairo_translate(cr, data->symbol_width / 2, data->symbol_height / 2);
	cairo_set_source_surface(cr, data->symbol_surface, x, y);
	cairo_paint(cr);
	cairo_restore(cr);
	frame = (frame + 1) % data->symbol_frames;
}

void on_draw_rotation_animation(cairo_t *cr, spinner_t *data)
{
	static float angle = 0.0;

	draw_background(cr, data);
	cairo_translate(cr, data->symbol_width / 2, data->symbol_height / 2);
	cairo_rotate(cr, angle);
	cairo_set_source_surface(cr, data->symbol_surface, -data->symbol_width / 2,
				 -data->symbol_height / 2);
	cairo_paint(cr);
	cairo_restore(cr);
	angle += data->rotate_step;
	if (angle > 2 * M_PI)
		angle = 0.0;
}

static uint32_t convert_to_cairo_format(uint32_t format)
{
	switch (format) {
	case DRM_FORMAT_RGB565:
		return CAIRO_FORMAT_RGB16_565;
	case DRM_FORMAT_XRGB8888:
		return CAIRO_FORMAT_ARGB32;
	}
	return CAIRO_FORMAT_INVALID;
}

static void cairo_draw_text(cairo_t *cr, Config config)
{
	if (strlen(config.text) > 0) {
		cairo_select_font_face(cr, config.text_font, CAIRO_FONT_SLANT_NORMAL,
				       CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, (double)config.text_size);
		cairo_move_to(cr, config.text_x, config.text_y);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_show_text(cr, config.text);
	}
}

static int cairo_create_display_surface(struct modeset_dev *dev, spinner_t *data)
{
	cairo_surface_t *disp_surface = cairo_image_surface_create_for_data(
	    dev->map, convert_to_cairo_format(dev->format->format), dev->width, dev->height,
	    dev->stride);
	if (cairo_surface_status(disp_surface)) {
		platsch_error("Failed to create cairo surface\n");
		return EXIT_FAILURE;
	}
	data->display_width = cairo_image_surface_get_width(disp_surface);
	data->display_height = cairo_image_surface_get_height(disp_surface);
	data->format = cairo_image_surface_get_format(disp_surface);
	data->disp_cr = cairo_create(disp_surface);
	return EXIT_SUCCESS;
}

// create a surface and paint background image
static int cairo_create_background_surface(spinner_t *data, Config config, const char *dir,
					   const char *base)
{
	char file_name[PLATSCH_MAX_FILENAME_LENGTH];
	int ret;

	ret = snprintf(file_name, sizeof(file_name), "%s/%s.png", dir, base);
	if (ret >= sizeof(file_name)) {
		platsch_error("Failed to fit file_name into buffer\n");
		return EXIT_FAILURE;
	}

	data->background_surface =
	    cairo_image_surface_create(data->format, data->display_width, data->display_height);
	if (cairo_surface_status(data->background_surface)) {
		platsch_error("Failed to create background surface\n");
		return EXIT_FAILURE;
	}

	cairo_surface_t *bk_img_surface = cairo_image_surface_create_from_png(file_name);

	if (cairo_surface_status(bk_img_surface)) {
		platsch_error("Failed to create cairo surface from %s\n", file_name);
		return EXIT_FAILURE;
	}

	int image_width = cairo_image_surface_get_width(bk_img_surface);
	int image_height = cairo_image_surface_get_height(bk_img_surface);
	double scale_x = (double)data->display_width / image_width;
	double scale_y = (double)data->display_height / image_height;

	cairo_t *cr_background = cairo_create(data->background_surface);
	cairo_scale(cr_background, scale_x, scale_y);
	cairo_set_source_surface(cr_background, bk_img_surface, 0, 0);

	cairo_paint(cr_background);
	cairo_draw_text(cr_background, config);
	return EXIT_SUCCESS;
}

static int cairo_create_symbol_surface(spinner_t *data, Config config)
{
	if (strlen(config.symbol) == 0)
		return EXIT_FAILURE;

	data->symbol_surface = cairo_image_surface_create_from_png(config.symbol);
	if (cairo_surface_status(data->symbol_surface)) {
		platsch_error("Failed loading %s\n", config.symbol);
		data->symbol_surface = NULL;
		return EXIT_FAILURE;
	}
	data->symbol_width = cairo_image_surface_get_width(data->symbol_surface);
	data->symbol_height = cairo_image_surface_get_height(data->symbol_surface);

	if (data->symbol_width > data->symbol_height) {
		data->symbol_frames = data->symbol_width / data->symbol_height;
		data->symbol_width = data->symbol_height;
	} else {
		data->symbol_frames = data->symbol_height / data->symbol_width;
		data->symbol_height = data->symbol_width;
	}

	data->rotate_step = config.rotate_step;

	if (config.symbol_x <0)
		data->symbol_x = data->display_width / 2 - data->symbol_width / 2;
	else
		data->symbol_x = config.symbol_x;

	if(config.symbol_y <0)
		data->symbol_y = data->display_height / 2 - data->symbol_height / 2;
	else
		data->symbol_y = config.symbol_y;

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	bool pid1 = getpid() == 1;
	char **initsargv;
	char file_name[PLATSCH_MAX_FILENAME_LENGTH];
	Config config = DEFAULT_CONFIG;
	const char *base = "splash";
	const char *dir = "/usr/share/platsch";
	const char *env;
	int ret;

	spinner_t *spinner_list = NULL, *spinner_node = NULL, *spinner_iter = NULL;
	struct modeset_dev *iter;
	struct timespec start, end;

	env = getenv("platsch_directory");
	if (env)
		dir = env;

	env = getenv("platsch_basename");
	if (env)
		base = env;

	ret = snprintf(file_name, sizeof(file_name), "%s/spinner.conf", dir);
	if (ret >= sizeof(file_name)) {
		platsch_error("Failed to fit file_name\n");
		return EXIT_FAILURE;
	}

	parseConfig(file_name, &config);

	struct modeset_dev *modeset_list = platsch_init();

	if (!modeset_list) {
		platsch_error("Failed to initialize modeset\n");
		return EXIT_FAILURE;
	}

	for (iter = modeset_list; iter; iter = iter->next) {
		spinner_node = (spinner_t *)malloc(sizeof(spinner_t));
		if (!spinner_node) {
			platsch_error("Failed to allocate memory for spinner_node\n");
			return EXIT_FAILURE;
		}
		memset(spinner_node, 0, sizeof(*spinner_node));

		if (cairo_create_display_surface(iter, spinner_node))
			return EXIT_FAILURE;

		if (cairo_create_background_surface(spinner_node, config, dir, base))
			return EXIT_FAILURE;

		cairo_create_symbol_surface(spinner_node, config);

		cairo_set_source_surface(spinner_node->disp_cr, spinner_node->background_surface, 0,
					 0);

		cairo_paint(spinner_node->disp_cr);
		platsch_setup_display(iter);

		spinner_node->next = spinner_list;
		spinner_list = spinner_node;
	}

	platsch_drmDropMaster();

	ret = fork();
	if (ret < 0)
		platsch_error("failed to fork for init: %m\n");
	else if (ret == 0)
		/*
		 * Always fork to make sure we got the required
		 * resources for drawing the animation in the child.
		 */
		goto drawing;

	if (pid1) {
		initsargv = calloc(sizeof(argv[0]), argc + 1);

		if (!initsargv) {
			platsch_error("failed to allocate argv for init\n");
			return EXIT_FAILURE;
		}
		memcpy(initsargv, argv, argc * sizeof(argv[0]));
		initsargv[0] = "/sbin/init";
		initsargv[argc] = NULL;

		execv("/sbin/init", initsargv);

		platsch_error("failed to exec init: %m\n");

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

drawing:
	// create cairo surface for drawing symbol to avoid frlickering.
	cairo_surface_t *drawing_surface = cairo_image_surface_create(
	    spinner_node->format, spinner_node->symbol_width, spinner_node->symbol_height);
	if (cairo_surface_status(drawing_surface)) {
		platsch_error("Failed to create drawing surface\n");
		return EXIT_FAILURE;
	}

	cairo_t *cr_drawing = cairo_create(drawing_surface);

	while (true) {
		clock_gettime(CLOCK_MONOTONIC, &start);

		for (spinner_iter = spinner_list; spinner_iter; spinner_iter = spinner_iter->next) {
			if (spinner_node->symbol_surface == NULL) {
				sleep(10);
				continue;
			}

			if (spinner_node->symbol_frames > 1)
				on_draw_sequence_animation(cr_drawing, spinner_iter);
			else
				on_draw_rotation_animation(cr_drawing, spinner_iter);

			cairo_set_source_surface(spinner_iter->disp_cr, drawing_surface,
						 spinner_iter->symbol_x, spinner_iter->symbol_y);
			cairo_paint(spinner_iter->disp_cr);
		}

		clock_gettime(CLOCK_MONOTONIC, &end);
		long elapsed_time =
		    (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_nsec - start.tv_nsec) / 1000;
		long sleep_time = (1000000 / config.fps) - elapsed_time;

		if (sleep_time > 0)
			usleep(sleep_time);
	}
}
