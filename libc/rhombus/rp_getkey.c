/*
 * Copyright (C) 2012 Nick Johnson <nickbjohnson4224 at gmail.com>
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

#include <rhombus.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <rho/natio.h>

/*****************************************************************************
 * rp_getkey
 *
 * Attempts to obtain an access key for a certain action class on a resource.
 * Returns the access key on success, zero on failure (valid keys are not 
 * allowed to be zero.)
 */

uint64_t rp_getkey(rp_t rp, int action) {
	uint64_t key;
	uint32_t key_low, key_hi;
	char *reply;

	if (!rp) {
		return 0;
	}

	reply = rcall(rp, 0, "get-key %d", action);

	if (iserror(reply)) {
		errno = geterror(reply);
		free(reply);
		return 0;
	}

	sscanf(reply, "%u %u", &key_low, &key_hi);
	free(reply);

	key = (uint64_t) key_low | (uint64_t) key_hi << 32ULL;

	return key;
}
