/*
 * Copyright (C) 2011 Jaagup Repan
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

#include "wmanager.h"

struct window_t *main_window;

void update_tiling() {
	struct window_t *window, *last;
	bool others = false;
	int width = screen_width / 2;
	int height = screen_height;
	int count = 0;
	int y = 0;

	for (window = windows; window; window = window->next) {
		if (!(window->flags & FLOATING)) {
			if (!main_window) {
				main_window = window;
			}
			else {
				others = true;
			}
			if (window != main_window) {
				if (window->flags & CONSTANT_SIZE) {
					height -= window->height + 2;
					if (screen_width - width < window->width) {
						width = screen_width - window->width;
					}
				}
				else {
					last = window;
					count++;
				}
			}
		}
	}

	if (!main_window) {
		return;
	}

	if (width < 10) {
		width = 10;
	}

	main_window->x = main_window->y = 0;
	if (main_window->flags & CONSTANT_SIZE) {
		width = main_window->width;
	}
	else {
		resize_window(main_window, others ? (size_t) width : screen_width, screen_height, true);
	}

	for (window = windows; window; window = window->next) {
		if (!(window->flags & FLOATING) && window != main_window) {
			window->x = width + 2;
			window->y = y;
			if (!(window->flags & CONSTANT_SIZE)) {
				resize_window(window, screen_width - width - 2,
						height / count + (window == last ? height % count : -2), true);
			}
			y += window->height + 2;
		}
	}

	update_screen(0, 0, screen_width, screen_height);
}