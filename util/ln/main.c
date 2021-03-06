/*
 * Copyright (C) 2011 Nick Johnson <nickbjohnson4224 at gmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>

#include <rho/natio.h>

/*****************************************************************************
 * ln - create hard and symbolic links
 *
 * SYNOPSIS
 *
 *   ln <target> <link_name>
 *
 *   Creates a new link to <target> at the path <link_name>.
 *
 *   ln -s <target> <link_name>
 *
 *   Creates a new symbolic link to <target> at the path <link_name>.
 */

int main(int argc, char **argv) {
	
	if (argc < 3) {
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return 1;
	}

	if (argc == 4) {
		if (fs_slink(argv[3], argv[2])) {
			fprintf(stderr, "%s: error: ", argv[0]);
			perror(NULL);
		}
	}
	else {
		if (fs_link(argv[2], argv[1])) {
			fprintf(stderr, "%s: error: ", argv[0]);
			perror(NULL);
		}
	}

	return 0;
}
