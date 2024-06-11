#include "libplatsch.h"
#include "spinner_conf.h"
#include <cairo.h>
#include <math.h>
#include <sys/time.h>

typedef struct spinner {
	cairo_format_t fmt;
	cairo_surface_t *background_surface;
	cairo_surface_t *icon_surface;
	cairo_surface_t *image_surface;
	cairo_surface_t *drawing_surface;
	cairo_t *cr_background;
	cairo_t *cr_drawing;
	cairo_t *device_cr;
	int background_height;
	int background_width;
	int display_height;
	int display_width;
	int icon_height;
	int icon_width;
	struct modeset_dev *dev;
	struct spinner *next;
} spinner_t;

void on_draw_Sequence_animation(cairo_t *cr, spinner_t *data)
{
	static int current_frame;
	int num_frames = data->icon_width / data->icon_height;
	int frame_width = data->icon_height;

	cairo_set_source_surface(cr, data->background_surface, 0, 0);
	cairo_paint(cr);

	cairo_save(cr);

	cairo_translate(cr, data->display_width / 2, data->display_height / 2);

	cairo_set_source_surface(cr, data->icon_surface,
				 -frame_width / 2 - current_frame * frame_width,
				 -frame_width / 2);

	cairo_rectangle(cr, -frame_width / 2, -frame_width / 2,
			frame_width, frame_width);
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
	cairo_translate(cr, -data->icon_width / 2, -data->icon_height / 2);
	cairo_set_source_surface(cr, data->icon_surface, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
	angle += 0.1;
	if (angle > 2 * M_PI)
		angle = 0.0;
}

int main(int argc, char *argv[])
{
	bool pid1 = getpid() == 1;
	char filename[128];
	Config config = DEFAULT_CONFIG;
	const char *base = "splash";
	const char *dir = "/usr/share/platsch";
	const char *env;
	int frames;
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
		fprintf(stderr, "Failed to initialize modeset\n");
		return EXIT_FAILURE;
	}

	for (iter = modeset_list; iter; iter = iter->next) {
		spinner_node = (spinner_t *)malloc(sizeof(spinner_t));
		if (!spinner_node) {
			fprintf(stderr, "Failed to allocate memory for spinner_node\n");
			return EXIT_FAILURE;
		}
		memset(spinner_node, 0, sizeof(*spinner_node));
		printf("spinner_node=%p\n", spinner_node);

		spinner_node->device_cr = cairo_init(iter, dir, base);
		if (!spinner_node->device_cr)
			return EXIT_FAILURE;

		cairo_surface_t *surface = cairo_get_target(spinner_node->device_cr);

		if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
			fprintf(stderr, "Failed to get cairo surface\n");
			return EXIT_FAILURE;
		}
		spinner_node->display_width = cairo_image_surface_get_width(surface);
		spinner_node->display_height = cairo_image_surface_get_height(surface);
		spinner_node->fmt = cairo_image_surface_get_format(surface);

		spinner_node->background_surface = cairo_image_surface_create(
			spinner_node->fmt,
			spinner_node->display_width,
			spinner_node->display_height);
		if (cairo_surface_status(spinner_node->background_surface)
			!= CAIRO_STATUS_SUCCESS) {
			fprintf(stderr, "Failed to load splash.png\n");
			return EXIT_FAILURE;
		}

		spinner_node->image_surface = cairo_image_surface_create_from_png(config.backdrop);
		if (cairo_surface_status(spinner_node->image_surface) != CAIRO_STATUS_SUCCESS) {
			error("Failed to create cairo surface from %s\n", config.backdrop);
			return EXIT_FAILURE;
		}

		int image_width = cairo_image_surface_get_width(spinner_node->image_surface);
		int image_height = cairo_image_surface_get_height(spinner_node->image_surface);
		double scale_x = (double)spinner_node->display_width / image_width;
		double scale_y = (double)spinner_node->display_height / image_height;

		spinner_node->cr_background = cairo_create(spinner_node->background_surface);
		cairo_scale(spinner_node->cr_background, scale_x, scale_y);
		cairo_set_source_surface(spinner_node->cr_background,
			spinner_node->image_surface, 0, 0);

		cairo_paint(spinner_node->cr_background);

		cairo_select_font_face(spinner_node->cr_background, config.text_font,
				       CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(spinner_node->cr_background, (double)config.text_size);
		cairo_set_source_rgb(spinner_node->cr_background, 0, 0, 0);
		cairo_move_to(spinner_node->cr_background, config.text_x, config.text_y);
		cairo_show_text(spinner_node->cr_background, config.text);

		spinner_node->background_width = cairo_image_surface_get_width(
			spinner_node->background_surface);
		spinner_node->background_height = cairo_image_surface_get_height(
			spinner_node->background_surface);
		printf("spinner_node->background_width=%d, spinner_node->background_height=%d\n",
		       spinner_node->background_width, spinner_node->background_height);

		spinner_node->icon_surface = cairo_image_surface_create_from_png(config.symbol);
		if (cairo_surface_status(spinner_node->icon_surface) != CAIRO_STATUS_SUCCESS) {
			fprintf(stderr, "Failed to load %s\n", config.symbol);
			return EXIT_FAILURE;
		}
		spinner_node->icon_width = cairo_image_surface_get_width(
			spinner_node->icon_surface);
		spinner_node->icon_height = cairo_image_surface_get_height(
			spinner_node->icon_surface);
		printf("spinner_node->icon_width=%d, spinner_node->icon_height=%d\n",
		       spinner_node->icon_width, spinner_node->icon_height);

		spinner_node->drawing_surface = cairo_image_surface_create(
			spinner_node->fmt,
			spinner_node->display_width,
			spinner_node->display_height);
		if (cairo_surface_status(spinner_node->drawing_surface) != CAIRO_STATUS_SUCCESS) {
			error("Failed to create drawing surface\n");
			return EXIT_FAILURE;
		}
		spinner_node->cr_drawing = cairo_create(spinner_node->drawing_surface);

		cairo_set_source_surface(
			spinner_node->device_cr,
			spinner_node->drawing_surface, 0, 0);
		update_display(iter);

		spinner_node->next = spinner_list;
		spinner_list = spinner_node;
	}

	if (pid1) {
		char **initsargv;

		ret = fork();
		printf("fork ret=%d\n", ret);
		if (ret < 0)
			error("failed to fork for init: %m\n");
		else if (ret == 0)
			goto drawing;

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

drawing:
	printf("drawing\n");
	frames = config.frames;
	if (config.frames == 0)
		frames = 1;

	while (frames) {
		gettimeofday(&start, NULL);
		for (spinner_iter = spinner_list; spinner_iter; spinner_iter = spinner_iter->next) {
			if (spinner_node->icon_width / spinner_node->icon_height > 2)
				on_draw_Sequence_animation(spinner_iter->cr_drawing, spinner_iter);
			else
				on_draw_rotation_animation(spinner_iter->cr_drawing, spinner_iter);

			cairo_set_source_surface(
				spinner_iter->device_cr,
				spinner_iter->drawing_surface, 0, 0);
			cairo_paint(spinner_iter->device_cr);
		}
		gettimeofday(&end, NULL);
		elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 +
			(end.tv_usec - start.tv_usec);

		long sleep_time = (1000000 / config.fps) - elapsed_time;

		if (sleep_time > 0)
			usleep(sleep_time);

		if (config.frames > 0)
			frames--;
	}

	return 0;
}
