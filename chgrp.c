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
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef OPEN_MAX
#define OPEN_MAX _POSIX_OPEN_MAX
#endif

static int changelink = 0;
static int newgid = 0;
static int retval = 0;
static enum { UNSET, COMMANDLINE, RECURSIVE, NONE } follow = UNSET;

int chgrp(const char *p, const struct stat *st, int typeflag, struct FTW *f)
{
	(void)st; (void)typeflag; (void)f;

	if (chown(p, (uid_t)-1, newgid) == -1) {
		fprintf(stderr, "chgrp: %s: %s\n", p, strerror(errno));
		retval = 1;
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
	int recursive = 0;

	int c;
	while ((c = getopt(argc, argv, "hHLPR")) != -1) {
		switch (c) {
		case 'h':
			changelink = 1;
			break;

		case 'H':
			follow = COMMANDLINE;
			break;

		case 'L':
			follow = RECURSIVE;
			break;

		case 'P':
			follow = NONE;
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
		fprintf(stderr, "chgrp: missing operand\n");
		return 1;
	}

	newgid = strtogid(argv[optind++]);

	/* TODO: handle -hHLP */
	while (optind < argc) { 
		if (recursive) {
			nftw(argv[optind++], chgrp, OPEN_MAX, 0);
		} else {
			chgrp(argv[optind++], NULL, 0, NULL);
		}
	}

	return retval;
}
