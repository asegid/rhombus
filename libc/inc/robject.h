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

#ifndef __RLIBC_ROBJECT_H
#define __RLIBC_ROBJECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************
 * Rhombus Objects (robjects)
 *
 * Rhombus objects are the way in which processes and drivers running on a
 * Rhombus system interact with each other. A process can be seen from an
 * external standpoint as being a collection of one or more robjects, to which
 * messages can be sent. Robjects can represent files, directories, pipes,
 * windows, interfaces, devices, drivers, processes... pretty much anything
 * that processes or users might want to interact with.
 *
 * The things contained within this sublibrary are used to send, route, and
 * recieve requests to and from robjects, usually via the low-level message 
 * passing IPC layer.
 */

struct robject; // defined below

void __robject_init(void);

/*****************************************************************************
 * Rhombus object pointers (robject pointers)
 *
 * This stuff right here, this is important. Robject pointers are the way
 * that Rhombus indentifies pretty much everything on the system. Every file
 * can be uniquely identified by a robject pointer, which, because it is an 
 * integer type, is easy to pass to and from functions. Robject pointers 
 * also have canonical string representations which are used in the rcall and 
 * event interfaces, and are human-readable.
 *
 * The string representations have the following format:
 *
 *   @p:i
 *
 * Where p is the PID of the pointer and i is the index, both in decimal.
 * For example, if the PID is 42 and the index is 123, the string 
 * representation would be "@42:123".
 *
 * There are also four macros used to portably interface with robject
 * pointer contents:
 *
 *   RP_CONS(pid, index) - construct a rp with the given PID and index.
 *   RP_PID(rp)          - evaluate to the PID of the rp.
 *   RP_INDEX(rp)        - evaluate to the index of the rp.
 *   RP_NULL             - universally represents no robject; false-like
 */

// guaranteed to be an integer type
typedef uint64_t rp_t;

#define RP_CONS(idx, pid) ((((uint64_t) (pid)) << 32) | (uint64_t) (idx))
#define RP_INDEX(rp)      ((uint32_t) ((rp) >> 32))
#define RP_PID(rp)        ((uint32_t) ((rp) & 0xFFFFFFFF))
#define RP_HEAD(rp)       ((uint64_t) ((rp) & 0xFFFFFFFF))
#define RP_NULL           ((uint64_t) 0)

char *rtoa(rp_t rp);         // convert robject pointer to string
rp_t  ator(const char *str); // convert string to robject pointer

/*****************************************************************************
 * Rhombus Object Message Passing (rcall)
 *
 * Robjects can accept a bunch of I/O specific messages (see natio.h), but
 * more commonly use a generic text-based message protocol called rcall.
 *
 * This protocol is very simple. The only argument is an ASCII string, and the
 * only return value is an ASCII string. Within the argument, there is some
 * structure, however. The argument string is interpreted as a sequence of
 * space-separated tokens (much like the arguments to a command line utility),
 * with the first token being the name of the robject method to be called.
 */

// rcall hook format
typedef char *(*rcall_t)(struct robject *self, rp_t src, int argc, char **argv);

char *rcall(rp_t rp, const char *fmt, ...);

/**************************************************************************** 
 * Rhombus Object Event System (event)
 *
 * The event protocol is an asyncronous, broadcasting parallel of the rcall 
 * protocol. Only a single ASCII string is sent as event data, and events are 
 * sent from robjects to processes. Each robject maintains a list of "event 
 * subscribers", to which messages are sent if an event is to be sent from 
 * that robject. Think of it as an RSS feed.
 *
 * Instead of a method name like in rcall, event uses the first token of the
 * argument string as an event type, which is used to route it. Event types
 * should be used to group similar events together (like keypress events, or
 * mouse movement events, or window events.)
 */

// event hook format
typedef void (*event_t)(rp_t src, int argc, char **argv);

int event_subscribe  (rp_t event_source);
int event_unsubscribe(rp_t event_source);

int event(rp_t rp, const char *value);

int event_hook(const char *type, event_t hook);

/*****************************************************************************
 * Rhombus Object Indexing and Lookup
 */

extern struct robject *robject_root;

void            robject_set(uint32_t index, struct robject *ro);
struct robject *robject_get(uint32_t index);

uint32_t robject_new_index(void);

/*****************************************************************************
 * Rhombus Object Operations
 */

/* 
 * robject event storage internals
 */

struct __robject_event_set {
	struct __robject_event_set *next;
	struct __robject_event_set *prev;
	rp_t target;
};

struct __robject_event_set *__event_set_add(struct __robject_event_set *set, rp_t target);
struct __robject_event_set *__event_set_del(struct __robject_event_set *set, rp_t target);
void   __event_set_send(struct __robject_event_set *set, const char *value);
void   __event_set_free(struct __robject_event_set *set);

/*
 * robject definition
 */

struct robject {
	bool     mutex; // OPT - this should be a readers/writer lock
	bool     driver_mutex;
	uint32_t index; // object index within process; do not modify
	
	// parent interface (for defaulting messages to)
	struct robject *parent;

	// robject fields
	struct s_table *call_table; // table of rcall hooks
	struct s_table *data_table; // table of general data
	struct __robject_event_set  *event_subs; // list of event subscribers
};

// constructor/destructor
struct robject *robject_cons(uint32_t index, struct robject *parent);
void            robject_free(struct robject *ro);

// various field manipulations
void    robject_set_call(struct robject *ro, const char *call, rcall_t hook);
rcall_t robject_get_call(struct robject *ro, const char *call);
void    robject_set_data(struct robject *ro, const char *field, void *data);
void   *robject_get_data(struct robject *ro, const char *field);

// event management
void    robject_add_subscriber(struct robject *ro, rp_t target);
void    robject_del_subscriber(struct robject *ro, rp_t target);

// reference counting
uint32_t robject_get_refc(struct robject *ro);

// basic interface
void  robject_event(struct robject *ro, const char *event);
char *robject_call (struct robject *ro, rp_t source, const char *args);
void *robject_data (struct robject *ro, const char *field);

/*****************************************************************************
 * Standard Classes
 */

extern struct robject *robject_class_basic;

/*****************************************************************************
 * Type System
 */

int robject_is_type(const char *typestr, const char *type);
int robject_check_type(struct robject *ro, const char *type);

#endif/*__RLIBC_ROBJECT_H*/
