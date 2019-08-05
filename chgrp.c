/*
 * UNG's Not GNU
 *
 * Copyright (c) 2011-2019, Jakob Kaivo <jkk@ung.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>

#define FOLLOWCOMMANDLINE (1)
#define FOLLOWRECURSIVE (2)
#define NOFOLLOW (3)

static int chgrp(gid_t gid, const char *path, int flag)
{
	(void)flag;
	if (chown(path, (uid_t)-1, gid) == -1) {
		fprintf(stderr, "chgrp: Couldn't change group of %s: %s\n", path, strerror(errno));
		return 1;
	}
	return 0;
}

static gid_t strtogid(const char *s)
{
	struct group *grp = getgrnam(s);
	if (grp == NULL) {
		gid_t gid = (gid_t)atoi(s);
		if (gid == 0 && strcmp(s, "0")) {
			fprintf(stderr, "chgrp: Couldn't find group %s\n", s);
			exit(1);
		}
		return gid;
	}
	return grp->gr_gid;
}

int main(int argc, char **argv)
{
	int c = 0, r = 0;
	int changelink = 0;
	int flag = 0;
	int recursive = 0;

	while ((c = getopt(argc, argv, "hHLPR")) != -1) {
		switch (c) {
		case 'h':
			changelink = 1;
			break;

		case 'H':
			flag = FOLLOWCOMMANDLINE;
			break;

		case 'L':
			flag = FOLLOWRECURSIVE;
			break;

		case 'P':
			flag = NOFOLLOW;
			break;

		case 'R':
			recursive = 1;
			break;

		default:
			return 1;
		}
	}

	if (recursive && changelink) {
		fprintf(stderr, "chgrp: Options -h and -R are mutually exclusive\n");
		return 1;
	}

	if (optind >= argc - 1) {
		fprintf(stderr, "chgrp: Group and at least one file are required\n");
		return 1;
	}

	gid_t gid = strtogid(argv[optind++]);

	while (optind < argc) { 
		r |= chgrp(gid, argv[optind++], flag);
	}

	return r;
}
