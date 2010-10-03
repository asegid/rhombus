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

#include <stdio.h>
#include <natio.h>
#include <mutex.h>
#include <ipc.h>

static lfs_handler_t _action[16] = {
	NULL,
	NULL,
	NULL,
	NULL,
	lfs_get_default
};

static bool _m_action;
static bool _active;

/****************************************************************************
 * lfs_event_start
 *
 * Starts event handling for the LFS.
 */

void lfs_event_start(void) {
	when(PORT_VFS, lfs_event);
	pdump(PORT_VFS);
	_active = true;
}

/****************************************************************************
 * lfs_event_stop
 *
 * Stops event handline for the LFS/
 */

void lfs_event_stop(void) {
	_active = false;
}

/****************************************************************************
 * lfs_event
 *
 * Event handler for local file system - calls the appropriate handler
 * registered with lfs_when_* when an event is caught. If there is no
 * handler registered, it returns a query with errcode VFS_ERR.
 */

void lfs_event(struct packet *packet, uint8_t port, uint32_t caller) {
	struct vfs_query *query;
	lfs_handler_t handler;

	query = pgetbuf(packet);

	if (_active) {
		mutex_spin(&_m_action);
		handler = _action[(query->opcode & VFS_VERB) >> 4];
		mutex_free(&_m_action);
	
		if (!handler) {
			query->opcode = VFS_ERR;
		}
		else {
			handler(query, packet->target_inode, caller);
		}
	}
	else {
		query->opcode = VFS_ERR;
	}

	psend(PORT_REPLY, caller, packet);
	pfree(packet);
}

/****************************************************************************
 * lfs_when_new
 *
 * Sets handler for VFS_NEW verb.
 */

void lfs_when_new(lfs_handler_t handler) {

	mutex_spin(&_m_action);
	_action[VFS_NEW >> 4] = handler;
	mutex_free(&_m_action);
}

/****************************************************************************
 * lfs_when_del
 *
 * Sets handler for VFS_DEL verb.
 */

void lfs_when_del(lfs_handler_t handler) {

	mutex_spin(&_m_action);
	_action[VFS_DEL >> 4] = handler;
	mutex_free(&_m_action);
}

/****************************************************************************
 * lfs_when_mov
 *
 * Sets handler for VFS_MOV verb.
 */

void lfs_when_mov(lfs_handler_t handler) {

	mutex_spin(&_m_action);
	_action[VFS_MOV >> 4] = handler;
	mutex_free(&_m_action);
}

/****************************************************************************
 * lfs_when_get
 *
 * Sets handler for VFS_GET verb.
 */

void lfs_when_get(lfs_handler_t handler) {

	mutex_spin(&_m_action);
	_action[VFS_GET >> 4] = handler;
	mutex_free(&_m_action);
}

/****************************************************************************
 * lfs_when_set
 *
 * Sets handler for VFS_SET verb.
 */

void lfs_when_set(lfs_handler_t handler) {

	mutex_spin(&_m_action);
	_action[VFS_SET >> 4] = handler;
	mutex_free(&_m_action);
}
