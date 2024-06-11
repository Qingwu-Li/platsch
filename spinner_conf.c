#include "spinner_conf.h"
#include <errno.h>
#include <string.h>

int parseConfig(const char *filename, Config *config)
{
	FILE *file;
	char line[MAX_LINE_LENGTH*2];
	char key[MAX_LINE_LENGTH];
	char value[MAX_LINE_LENGTH+1];
	char *value_start;
	char *value_end;

	file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Unable to open file: %s\n", filename);
		return -EFAULT;
	}

	while (fgets(line, sizeof(line), file)) {
		if (strlen(line) > MAX_LINE_LENGTH) {
			fprintf(stderr, "conf string too long\n");
			continue;
		}
		if (line[0] != '#' && sscanf(line, "%[^=]=%[^\n]", key, value) == 2) {
			value_start = strchr(line, '=') + 1;
			value_end = line + strlen(line) - 1;

			while (isspace(*value_start)) value_start++;
			while (isspace(*value_end) || *value_end == '"') value_end--;

			if (*value_start == '"') {
				value_start++;
				if (*value_end == '"') value_end--;
			}

			strncpy(value, value_start, value_end - value_start + 1);
			value[value_end - value_start + 1] = '\0';
			value[sizeof(value) - 1] = '\0';

			if (strcmp(key, "backdrop") == 0) {
				strncpy(config->backdrop, value, MAX_LINE_LENGTH);
				config->backdrop[sizeof(config->backdrop) - 1] = '\0';
			} else if (strcmp(key, "symbol") == 0) {
				strncpy(config->symbol, value, MAX_LINE_LENGTH);
				config->symbol[sizeof(config->symbol) - 1] = '\0';
			} else if (strcmp(key, "fps") == 0) {
				config->fps = atoi(value);
			} else if (strcmp(key, "frames") == 0) {
				config->frames = atoi(value);
			} else if (strcmp(key, "text_x") == 0) {
				config->text_x = atoi(value);
			} else if (strcmp(key, "text_y") == 0) {
				config->text_y = atoi(value);
			} else if (strcmp(key, "text_font") == 0) {
				strncpy(config->text_font, value, MAX_LINE_LENGTH);
				config->text_font[sizeof(config->text_font) - 1] = '\0';
			} else if (strcmp(key, "text_size") == 0) {
				config->text_size = atoi(value);
			}
		}
	}

	fclose(file);
	return 0;
}
