/*
 * Copyright (C) 2009, 2010 Nick Johnson <nickbjohnson4224 at gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
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

#include <stdlib.h>
#include <string.h>
#include <natio.h>

/****************************************************************************
 * vfs_mov_file
 *
 * Sends a request for a file in driver <root> to be moved from path <path0>
 * to path <path1>. Returns zero on success, nonzero on failure. Note that
 * a driver can only move a file within its own filesystem, and any moves
 * across drivers must be done manually.
 */

int vfs_mov_file(FILE *root, const char *path0, const char *path1) {
	struct vfs_query query;

	query.opcode = VFS_ACT | VFS_MOV | VFS_FILE;
	strlcpy(query.path0, path0, MAX_PATH);
	strlcpy(query.path1, path1, MAX_PATH);

	if (!vfssend(root, &query)) {
		return -1;
	}
	else {
		if (query.opcode & VFS_ERR) {
			return query.opcode;
		}
		else {
			return 0;
		}
	}
}
