/*
 * Copyright (C) 2019 Pengutronix, Uwe Kleine-König <u.kleine-koenig@pengutronix.de>
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Some code parts base on example code written in 2012 by David Herrmann
 * <dh.herrmann@googlemail.com> and dedicated to the Public Domain. It was found
 * in 2019 on
 * https://raw.githubusercontent.com/dvdhrm/docs/master/drm-howto/modeset.c
 */

#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libplatsch.h"


static struct option longopts[] =
{
	{ "help",      no_argument,       0, 'h' },
	{ "directory", required_argument, 0, 'd' },
	{ "basename",  required_argument, 0, 'b' },
	{ NULL,        0,                 0, 0   }
};

static void usage(const char *prog)
{
	error("Usage:\n"
	      "%s [-d|--directory <dir>] [-b|--basename <name>]\n"
	      "   [-h|--help]\n",
	      prog);
}

int main(int argc, char *argv[])
{
	char **initsargv;
	struct modeset_dev *iter;
	bool pid1 = getpid() == 1;
	const char *dir = "/usr/share/platsch";
	const char *base = "splash";
	const char *env;
	int ret = 0, c;

	env = getenv("platsch_directory");
	if (env)
		dir = env;

	env = getenv("platsch_basename");
	if (env)
		base = env;

	if (!pid1) {
		while ((c = getopt_long(argc, argv, "hd:b:", longopts, NULL)) != EOF) {
			switch (c) {
			case 'd':
				dir = optarg;
				break;
			case 'b':
				base = optarg;
				break;
			case '?':
				/* ‘getopt_long’ already printed an error message. */
				ret = 1;
				/* FALLTHRU */
			case 'h':
				usage(basename(argv[0]));
				exit(ret);
			}
		}

		if (optind < argc) {
			error("Too many arguments!\n");
			usage(basename(argv[0]));
			exit(1);
		}
	}

	struct modeset_dev *modeset_list = init();
	if (!modeset_list) {
		error("Failed to initialize modeset\n");
		return EXIT_FAILURE;
	}

	for (iter = modeset_list; iter; iter = iter->next) {
		draw(iter, dir, base);
	}

	finish();

	if (pid1) {
		ret = fork();
		if (ret < 0) {
			error("failed to fork for init: %m\n");
		} else if (ret == 0) {
			/*
			 * in the child go to sleep to keep the drm device open
			 * and give pid 1 to init.
			 */
			goto sleep;
		}

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

sleep:
	redirect_stdfd();

	do {
		sleep(10);
	} while (1);
}
