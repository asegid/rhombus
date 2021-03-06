/*
 * Copyright (C) 2009-2012 Nick Johnson <nickbjohnson4224 at gmail.com>
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

#ifndef __RLIBC_IPC_H
#define __RLIBC_IPC_H

#include <stdbool.h>
#include <stdint.h>
#include <rho/natio.h>
#include <rho/arch.h>

/* action numbers ***********************************************************/

// kernel events and signals
#define ACTION_QUIT   0
#define ACTION_TERM   1
#define ACTION_ABORT  2
#define ACTION_KILL   3
#define ACTION_STOP   4
#define ACTION_CONT   5
#define ACTION_TRAP   6
#define ACTION_INT    7
#define ACTION_IRQ    8
#define ACTION_ALARM  9
#define ACTION_CHILD  10
#define ACTION_FLOAT  11
#define ACTION_PAGE   12
#define ACTION_ILL    13
#define ACTION_USER1  14
#define ACTION_USER2  15

// I/O and similar
#define ACTION_REPLY  16
#define ACTION_READ   17
#define ACTION_WRITE  18
#define ACTION_SYNC   19
#define ACTION_RESET  20
#define ACTION_SHARE  21
#define ACTION_RCALL  22
#define ACTION_EVENT  23
#define ACTION_CLOSE  24
#define ACTION_MMAP   25
#define ACTION_FINISH 26

/* message structure ********************************************************/

struct msg {
	uint64_t source; // resource pointer of source
	uint64_t target; // resource pointer of target
	uint64_t key;    // authentication key for action
	uint32_t length; // length of _data_ (not whole message)
	uint8_t  action; // action requested
	uint8_t  arch;   // architecture (i.e. byte order)
	uint16_t padding;
	uint8_t  data[]; // contained data
} __attribute__((packed));

#define ARCH_LEND	0
#define ARCH_BEND	1
#define ARCH_NAT	ARCH_LEND

/* queueing *****************************************************************/

int         mqueue_push(struct msg *msg);
struct msg *mqueue_pull(uint8_t port, uint64_t source);
struct msg *mqueue_wait(uint8_t port, uint64_t source);

void mqueue_set_policy(uint8_t port, bool do_queue);

/* sending and recieving ****************************************************/

int         msend(struct msg *msg);
struct msg *mwait(uint8_t port, uint64_t source);

/* high level send functions ************************************************/

int mreply(struct msg *msg);
int merror(struct msg *msg);
int msendb(uint64_t target, uint8_t port);
int msendv(uint64_t target, uint8_t port, void *data, size_t length);

/* event handlers ***********************************************************/

void when(uint8_t port, void (*handler)(struct msg *msg));

#endif/*__RLIBC_IPC_H*/
