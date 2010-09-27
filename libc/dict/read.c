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

#include <dict.h>
#include <stdint.h>
#include <stdlib.h>
#include <mutex.h>
#include <string.h>
#include <natio.h>
#include <proc.h>

/****************************************************************************
 * dict_read_rec
 *
 * Recursive dictionary reading function, used by interface read functions.
 * Returns null on failure, pointer to dictionary value copy on success. The
 * returned pointer is allocated on the heap and must be freed later with 
 * free. Follows all links found. This function is not thread-safe.
 */

static char *dread_rec(struct __dict *root, const char *key) {

	if (root->link) {
		return dreadr(root->link, key);
	}

	if (!key[0]) {
		if (root->value) {
			return strdup(root->value);
		}
		else {
			return NULL;
		}
	}

	if (root->next[(size_t) key[0]]) {
		return dread_rec(root->next[(size_t) key[0]], &key[1]);
	}
	
	return NULL;
}

/****************************************************************************
 * dread
 *
 * Read from dictionary with the given key into the buffer <val>, which must
 * have the size stored in <vlen>. <vlen> is set to the number of bytes
 * written into <val>. Returns -1 on failure, 0 on success. This function is 
 * thread-safe.
 */

char *dread(const char *key) {
	char *value;

	mutex_spin(&dict_mutex);
	value = dread_rec(dict_root, key);
	mutex_free(&dict_mutex);

	return value;
}

/****************************************************************************
 * dreadr
 *
 * Reads from the dictionary of the file <target> using the given string as 
 * a key into the buffer <val>, which must have the size stored in <vlen>. 
 * <vlen> is then set to the number of bytes written into <val>. Returns -1 
 * on failure, 0 on success. This function is thread-safe.
 */

char *dreadr(FILE *targ, const char *key) {
	struct __link_req req;
	size_t size;

	strlcpy(req.key, key, 2048);

	size = ssend(targ, &req, &req, sizeof(struct __link_req), 0, PORT_DREAD);

	if (!size) {
		return NULL;
	}

	return strdup(req.val);
}
