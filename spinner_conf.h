#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef __SPINNER_CONF_H__
#define __SPINNER_CONF_H__


#define MAX_LINE_LENGTH 128

typedef struct {
	char symbol[MAX_LINE_LENGTH];
	char text_font[MAX_LINE_LENGTH];
	char text[MAX_LINE_LENGTH];
	float rotate_step;
	int fps;
	int symbol_x;
	int symbol_y;
	int text_size;
	int text_x;
	int text_y;
} Config;

int parseConfig(const char *filename, Config *config);

#define DEFAULT_CONFIG { \
	.symbol = "", \
	.fps = 20, \
	.text_x = 0, \
	.text_y = 0, \
	.text_font = "Sans", \
	.text_size = 30, \
	.text = "", \
	.rotate_step = 0.1, \
	.symbol_x = -1, \
	.symbol_y = -1 \
}
#endif
