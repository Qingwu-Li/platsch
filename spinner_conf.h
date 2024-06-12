#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef __SPINNER_CONF_H__
#define __SPINNER_CONF_H__


#define MAX_LINE_LENGTH 128

typedef struct {
	char symbol[MAX_LINE_LENGTH];
	int fps;
	int frames;
	int text_x;
	int text_y;
	char text_font[MAX_LINE_LENGTH];
	int text_size;
	char text[MAX_LINE_LENGTH];
} Config;

int parseConfig(const char *filename, Config *config);

#define DEFAULT_CONFIG { \
	.symbol = "", \
	.fps = 20, \
	.frames = 0, \
	.text_x = 0, \
	.text_y = 0, \
	.text_font = "Sans", \
	.text_size = 30, \
	.text = "" \
}
#endif
