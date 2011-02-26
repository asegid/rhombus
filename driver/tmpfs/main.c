/*
 * Copyright (C) 2009, 2010 Nick Johnson <nickbjohnson4224 at gmail.com>
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

#include <stdlib.h>
#include <string.h>
#include <driver.h>
#include <proc.h>
#include <ipc.h>
#include <abi.h>

#include "tmpfs.h"

uint8_t tmpfs_index_top = 1;

struct vfs_obj *tmpfs_cons(uint64_t source, int type) {
	struct vfs_obj *fobj = NULL;

	switch (type) {
	case RP_TYPE_FILE:
		fobj        = calloc(sizeof(struct vfs_obj), 1);
		fobj->type  = RP_TYPE_FILE;
		fobj->size  = 0;
		fobj->data  = NULL;
		fobj->index = tmpfs_index_top++;
		fobj->acl   = acl_set_default(fobj->acl, PERM_READ | PERM_WRITE);
		break;
	case RP_TYPE_DIR:
		fobj        = calloc(sizeof(struct vfs_obj), 1);
		fobj->type  = RP_TYPE_DIR;
		fobj->index = tmpfs_index_top++;
		fobj->acl   = acl_set_default(fobj->acl, PERM_READ | PERM_WRITE);
		fobj->link  = 0;
		break;
	}

	return fobj;
}

int tmpfs_free(uint64_t source, struct vfs_obj *obj) {

	acl_free(obj->acl);

	if (obj->data) {
		free(obj->data);
	}

	free(obj);

	return 0;
}

size_t tmpfs_read(uint64_t source, struct vfs_obj *file, uint8_t *buffer, size_t size, uint64_t offset) {
	
	if (!file->data) {
		return 0;
	}

	if (offset >= file->size) {
		return 0;
	}

	if (offset + size >= file->size) {
		size = file->size - offset;
	}

	memcpy(buffer, &file->data[offset], size);
	
	return size;
}

size_t tmpfs_write(uint64_t source, struct vfs_obj *file, uint8_t *buffer, size_t size, uint64_t offset) {

	if (offset + size >= file->size) {
		file->data = realloc(file->data, offset + size);
		file->size = offset + size;
	}

	memcpy(&file->data[offset], buffer, size);

	return size;
}

int tmpfs_reset(uint64_t source, struct vfs_obj *file) {
	
	if (file->data) {
		free(file->data);
		file->size = 0;
		file->data = NULL;
	}

	return 0;
}

/****************************************************************************
 * tmpfs - temporary ramdisk filesystem driver
 *
 * Usage: tmpfs
 *
 * Constructs an empty ramdisk filesystem. Maximum size is limited only by
 * heap space.
 */

int main(int argc, char **argv) {	
	struct vfs_obj *root;

	root = calloc(sizeof(struct vfs_obj), 1);
	root->type = RP_TYPE_DIR;
	root->acl = acl_set_default(root->acl, PERM_READ | PERM_WRITE);
	vfs_set_index(0, root);

	/* set interface */
	di_wrap_read (tmpfs_read);
	di_wrap_write(tmpfs_write);
	di_wrap_reset(tmpfs_reset);
	vfs_wrap_cons(tmpfs_cons);
	vfs_wrap_free(tmpfs_free);
	vfs_wrap_init();

	/* daemonize */
	msendb(RP_CONS(getppid(), 0), PORT_CHILD);
	_done();

	return 0;
}
