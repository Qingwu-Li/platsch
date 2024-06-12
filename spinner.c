
#include <cairo.h>
#include <math.h>
#include <sys/time.h>

#include "libplatsch.h"
#include "spinner_conf.h"

typedef struct spinner {
	cairo_format_t fmt;
	cairo_surface_t *background_surface;
	cairo_surface_t *symbol_surface;
	cairo_surface_t *drawing_surface;
	cairo_t *cr_background;
	cairo_t *cr_drawing;
	cairo_t *disp_cr;
	int background_height;
	int background_width;
	int display_height;
	int display_width;
	int symbol_height;
	int symbol_width;
	struct modeset_dev *dev;
	struct spinner *next;
} spinner_t;

void on_draw_Sequence_animation(cairo_t *cr, spinner_t *data)
{
	static int current_frame;
	int num_frames = data->symbol_width / data->symbol_height;
	int frame_width = data->symbol_height;

	cairo_set_source_surface(cr, data->background_surface, 0, 0);
	cairo_paint(cr);

	cairo_save(cr);

	cairo_translate(cr, data->display_width / 2, data->display_height / 2);

	cairo_set_source_surface(cr, data->symbol_surface,
				 -frame_width / 2 - current_frame * frame_width, -frame_width / 2);

	cairo_rectangle(cr, -frame_width / 2, -frame_width / 2, frame_width, frame_width);
	cairo_clip(cr);
	cairo_paint(cr);

	cairo_restore(cr);

	current_frame = (current_frame + 1) % num_frames;
}

void on_draw_rotation_animation(cairo_t *cr, spinner_t *data)
{
	static float angle = 0.0;

	cairo_set_source_surface(cr, data->background_surface, 0, 0);
	cairo_paint(cr);
	cairo_save(cr);
	cairo_translate(cr, data->background_width / 2, data->background_height / 2);
	cairo_rotate(cr, angle);
	cairo_translate(cr, -data->symbol_width / 2, -data->symbol_height / 2);
	cairo_set_source_surface(cr, data->symbol_surface, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
	angle += 0.1;
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

static int cairo_create_display_surface(struct modeset_dev *dev, spinner_t *spinner_node)
{
	cairo_surface_t *disp_surface = cairo_image_surface_create_for_data(
	    dev->map, convert_to_cairo_format(dev->format->format), dev->width, dev->height,
	    dev->stride);
	if (cairo_surface_status(disp_surface)) {
		error("Failed to create cairo surface\n");
		return EXIT_FAILURE;
	}
	spinner_node->display_width = cairo_image_surface_get_width(disp_surface);
	spinner_node->display_height = cairo_image_surface_get_height(disp_surface);
	spinner_node->fmt = cairo_image_surface_get_format(disp_surface);
	spinner_node->disp_cr = cairo_create(disp_surface);
	return EXIT_SUCCESS;
}

// create a surface and paint background image
static int cairo_create_background_surface(spinner_t *spinner_node, Config config, const char *dir,
					   const char *base)
{
	char filename[128];
	int ret;

	ret = snprintf(filename, sizeof(filename), "%s/%s.png", dir, base);
	if (ret >= sizeof(filename)) {
		error("Failed to fit filename into buffer\n");
		return EXIT_FAILURE;
	}

	spinner_node->background_surface = cairo_image_surface_create(
	    spinner_node->fmt, spinner_node->display_width, spinner_node->display_height);
	if (cairo_surface_status(spinner_node->background_surface)) {
		error("Failed to create background surface\n");
		return EXIT_FAILURE;
	}

	cairo_surface_t *bk_img_surface = cairo_image_surface_create_from_png(filename);

	if (cairo_surface_status(bk_img_surface)) {
		error("Failed to create cairo surface from %s\n", filename);
		return EXIT_FAILURE;
	}

	int image_width = cairo_image_surface_get_width(bk_img_surface);
	int image_height = cairo_image_surface_get_height(bk_img_surface);
	double scale_x = (double)spinner_node->display_width / image_width;
	double scale_y = (double)spinner_node->display_height / image_height;

	spinner_node->cr_background = cairo_create(spinner_node->background_surface);
	cairo_scale(spinner_node->cr_background, scale_x, scale_y);
	cairo_set_source_surface(spinner_node->cr_background, bk_img_surface, 0, 0);

	spinner_node->background_width =
	    cairo_image_surface_get_width(spinner_node->background_surface);
	spinner_node->background_height =
	    cairo_image_surface_get_height(spinner_node->background_surface);

	cairo_paint(spinner_node->cr_background);
	cairo_draw_text(spinner_node->cr_background, config);
	return EXIT_SUCCESS;
}

static int cairo_create_symbol_surface(spinner_t *spinner_node, Config config)
{
	if (strlen(config.symbol) == 0)
		return EXIT_FAILURE;

	spinner_node->symbol_surface = cairo_image_surface_create_from_png(config.symbol);
	if (cairo_surface_status(spinner_node->symbol_surface)) {
		error("Failed loading %s\n", config.symbol);
		spinner_node->symbol_surface = NULL;
		return EXIT_FAILURE;
	}
	spinner_node->symbol_width = cairo_image_surface_get_width(spinner_node->symbol_surface);
	spinner_node->symbol_height = cairo_image_surface_get_height(spinner_node->symbol_surface);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	bool pid1 = getpid() == 1;
	char filename[128];
	Config config = DEFAULT_CONFIG;
	const char *base = "splash";
	const char *dir = "/usr/share/platsch";
	const char *env;
	int ret;
	long elapsed_time;

	spinner_t *spinner_list = NULL, *spinner_node = NULL, *spinner_iter = NULL;
	struct modeset_dev *iter;
	struct timeval start, end;

	env = getenv("platsch_directory");
	if (env)
		dir = env;

	env = getenv("platsch_basename");
	if (env)
		base = env;

	ret = snprintf(filename, sizeof(filename), "%s/spinner.conf", dir);
	if (ret >= sizeof(filename)) {
		error("Failed to fit filename\n");
		return EXIT_FAILURE;
	}

	parseConfig(filename, &config);

	struct modeset_dev *modeset_list = init();

	if (!modeset_list) {
		error("Failed to initialize modeset\n");
		return EXIT_FAILURE;
	}

	for (iter = modeset_list; iter; iter = iter->next) {
		spinner_node = (spinner_t *)malloc(sizeof(spinner_t));
		if (!spinner_node) {
			error("Failed to allocate memory for spinner_node\n");
			return EXIT_FAILURE;
		}
		memset(spinner_node, 0, sizeof(*spinner_node));

		if (cairo_create_display_surface(iter, spinner_node))
			return EXIT_FAILURE;

		if (cairo_create_background_surface(spinner_node, config, dir, base))
			return EXIT_FAILURE;

		cairo_create_symbol_surface(spinner_node, config);

		spinner_node->drawing_surface = cairo_image_surface_create(
		    spinner_node->fmt, spinner_node->display_width, spinner_node->display_height);
		if (cairo_surface_status(spinner_node->drawing_surface)) {
			error("Failed to create drawing surface\n");
			return EXIT_FAILURE;
		}
		// create cairo context for drawing surface to avoid frlickering
		spinner_node->cr_drawing = cairo_create(spinner_node->drawing_surface);
		// draw static image with text as first frame
		cairo_set_source_surface(spinner_node->disp_cr, spinner_node->background_surface, 0,
					 0);
		cairo_paint(spinner_node->disp_cr);
		update_display(iter);

		spinner_node->next = spinner_list;
		spinner_list = spinner_node;
	}

	char **initsargv;

	ret = fork();
	if (ret < 0)
		error("failed to fork for init: %m\n");
	else if (ret == 0)
		/*
		 * Always fork to make sure that got the required 
		 * resources for drawing the animation in the child.
		 */
		goto drawing;

	if (pid1) {
		initsargv = calloc(sizeof(argv[0]), argc + 1);

		if (!initsargv) {
			error("failed to allocate argv for init\n");
			return EXIT_FAILURE;
		}
		memcpy(initsargv, argv, argc * sizeof(argv[0]));
		initsargv[0] = "/sbin/init";
		initsargv[argc] = NULL;

		execv("/sbin/init", initsargv);

		error("failed to exec init: %m\n");

		return EXIT_FAILURE;
	}
	return 0;

drawing:
	while (true) {
		gettimeofday(&start, NULL);
		for (spinner_iter = spinner_list; spinner_iter; spinner_iter = spinner_iter->next) {
			if (spinner_node->symbol_surface == NULL)
				usleep(1000000 / config.fps);
			if (spinner_node->symbol_width / spinner_node->symbol_height > 2)
				on_draw_Sequence_animation(spinner_iter->cr_drawing, spinner_iter);
			else
				on_draw_rotation_animation(spinner_iter->cr_drawing, spinner_iter);

			cairo_set_source_surface(spinner_iter->disp_cr,
						 spinner_iter->drawing_surface, 0, 0);
			cairo_paint(spinner_iter->disp_cr);
		}
		gettimeofday(&end, NULL);
		elapsed_time =
		    (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

		long sleep_time = (1000000 / config.fps) - elapsed_time;

		if (sleep_time > 0)
			usleep(sleep_time);
	}
	redirect_stdfd();
}
