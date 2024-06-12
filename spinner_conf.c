#include "spinner_conf.h"
#include <errno.h>

int parseConfig(const char *filename, Config *config)
{
	char *value_end;
	char *value_start;
	char key[MAX_LINE_LENGTH];
	char line[MAX_LINE_LENGTH];
	char value[MAX_LINE_LENGTH];
	FILE *file;

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

			while (isspace(*value_start))
				value_start++;
			while (isspace(*value_end) || *value_end == '"')
				value_end--;

			if (*value_start == '"')
				value_start++;
			if (*value_end == '"')
					value_end--;

			if (value_end < value_start) {
				memset(value, 0, sizeof(value));
				continue;
			} else {
				strncpy(value, value_start, value_end - value_start + 1);
				value[value_end - value_start + 1] = '\0';
				value[sizeof(value) - 1] = '\0';
			}
			value[MAX_LINE_LENGTH - 1] = '\0';

			if (strcmp(key, "symbol") == 0) {
				strncpy(config->symbol, value, MAX_LINE_LENGTH);
			} else if (strcmp(key, "text") == 0) {
				strncpy(config->text, value, MAX_LINE_LENGTH);
			} else if (strcmp(key, "fps") == 0) {
				config->fps = atoi(value);
			} else if (strcmp(key, "text_x") == 0) {
				config->text_x = atoi(value);
			} else if (strcmp(key, "text_y") == 0) {
				config->text_y = atoi(value);
			} else if (strcmp(key, "text_font") == 0) {
				strncpy(config->text_font, value, MAX_LINE_LENGTH);
			} else if (strcmp(key, "text_size") == 0) {
				config->text_size = atoi(value);
			}
		}
	}
	fclose(file);
	return 0;
}
