/* Copyright 2009 Nick Johnson */

#include <lib.h>
#include <int.h>
#include <task.h>
#include <mem.h>

/***** IRQ HANDLERS *****/

/* Handles IRQ 0, and advances a simple counter used as a clock */
uint32_t tick = 0;
image_t *pit_handler(image_t *image) {
	if (image->cs | 1) tick++;

	/* Switch to next scheduled task */
	return task_switch(task_next(0));
}

image_t *irq_redirect(image_t *image) {
	uint32_t args[4];

	/* Send S_IRQ signal to the task registered with the IRQ */
	/* with the IRQ number as arg 0 */ 
	args[0] = DEIRQ(image->num);
	return signal(irq_holder[DEIRQ(image->num)], S_IRQ, args, TF_NOERR);
}

/***** FAULT HANDLERS *****/

/* Generic fault */
image_t *fault_generic(image_t *image) {
	uint32_t args[4];

	/* If in kernelspace, panic */
	if ((image->cs & 0x3) == 0) {
		printk("EIP:%x NUM:%d ERR:%x\n", image->eip, image->num, image->err);
		panic("unknown exception");
	}

	/* If in userspace, redirect to signal S_GEN */
	args[0] = image->num;
	args[1] = image->err;
	return signal(curr_pid, S_GEN, args, TF_NOERR | TF_EKILL);
}

/* Page fault */
image_t *fault_page(image_t *image) {
	extern void list_sched(void);
	uint32_t args[4];
	uint32_t cr2;

	/* Get faulting address from register CR2 */
	asm volatile ("movl %%cr2, %0" : "=r" (cr2));

	/* If in kernelspace, panic */
	if ((image->cs & 0x3) == 0) { /* i.e. if it was kernelmode */
		printk("page fault at %x, ip = %x frame %x task %d\n", 
			cr2, image->eip, page_get(cr2), curr_pid);
		panic("page fault exception");
	}

	/* If in userspace, redirect to signal S_PAG, with faulting address */
	args[0] = image->eip;
	args[1] = image->err;
	args[3] = cr2;
	return signal(curr_pid, S_PAG, args, TF_NOERR | TF_EKILL);
}

/* Floating point exception */
image_t *fault_float(image_t *image) {
	uint32_t args[4];

	/* If in kernelspace, panic */
	if ((image->cs & 0x3) == 0) {
		panic("floating point exception");
	}

	/* If in userspace, redirect to signal S_FPE */
	args[0] = image->eip;
	return signal(curr_pid, S_FPE, args, TF_NOERR | TF_EKILL);
}

/* Double fault */
image_t *fault_double(image_t *image) {

	/* Can only come from kernel problems */
	printk("DS:%x CS:%x\n", image->ds, image->cs);
	panic("double fault exception");
	return NULL;

}

/* SSND - send a signal to a task
 *
 * Arguments:
 *  edi: target task
 *  esi: 
 *   byte 0: signal number
 *   byte 1: blocking flags
 *  eax-edx: arguments (16 bytes)
 * Returns nothing
 *
 * Pushes the target's current state onto its TIS
 * Invokes the signal handler of the target task
 * Sets esi to signal number
 * Sets edi to the caller's PID
 * Sets eax-edx to arguments
 */
image_t *ssnd_call(image_t *image) {
	uint32_t args[4];
	uint8_t sig, flags;

	args[0] = image->eax;
	args[1] = image->ebx;
	args[2] = image->ecx;
	args[3] = image->edx;
	sig = image->esi & 0xFF;
	flags = (image->esi >> 8) & 0xFF;

	return signal((pid_t) image->edi, sig, args, flags);
}

/* SRET - return from a signal handler
 *
 * Arguments:
 *  esi: caller
 * Returns nothing
 *
 * Unblocks the caller
 * Pops the target's previous state off of the TIS
 */
image_t *sret_call(image_t *image) {
	return sret(image);
}

/* Push - move data to another address space
 * 
 * Arguments:
 *  eax: target task (0 for physical memory)
 *  ecx: size of data in bytes (cannot exceed 16KB)
 *  esi: source address of data
 *  edi: destination address of data
 * Returns 0 on success
 */
image_t *push_call(image_t *image) {
	uint32_t src = image->esi;
	uint32_t dst = image->edi;
	uint32_t size = image->ecx;
	uint32_t targ = image->eax;
	task_t *t;
	uint32_t i;

	/* Bounds check addresses */
	if (src + size > LSPACE || dst + size > LSPACE) ret(image, ERROR);
	if (size > 0x4000) ret(image, ERROR);	/* Limit writes to 16K */

	/* Find and check target */
	if (targ) {
		t = task_get(targ);
		if (!t) ret(image, ERROR);
		map_temp(t->map);
	}

	/* Map pages - XXX really ugly, fix or at least comment */
	for (i = 0; i < size + 0x1000; i += 0x1000) {
		if (targ) {
			if ((temp_get(dst + i) & PF_PRES) == 0) ret(image, ERROR);
			page_set((uint32_t) tdst + i,
				page_fmt(temp_get(dst + i), (PF_RW | PF_PRES)));
		}
		else page_set((uint32_t) tdst + i,
			page_fmt((dst + i) &~ 0xFFF, (PF_RW | PF_PRES)));


		if ((page_get(src + i) & PF_PRES) == 0) ret(image, EPERMIT);
		page_set((uint32_t) tsrc + i,
			page_fmt(ctbl[(src + i) >> 12], (PF_RW | PF_PRES)));
	}

	/* Copy memory */
	memcpy((void*) ((uint32_t) tdst + (dst & 0xFFF)),
		(void*) ((uint32_t) tsrc + (src & 0xFFF)), size);
	
	ret(image, 0);
}

/* Pull - move data from another address space
 *
 * Arguments:
 *  eax: target task (0 for physical memory)
 *  ecx: size of data in bytes (cannot exceed 16KB)
 *  esi: source address of data
 *  edi: destination address of data
 * Returns 0 on success
 */
image_t *pull_call(image_t *image) {
	uint32_t src = image->esi;
	uint32_t dst = image->edi;
	uint32_t size = image->ecx;
	uint32_t targ = image->eax;
	task_t *t;
	uint32_t i;

	/* Bounds check addresses */
	if (src + size > LSPACE || dst + size > LSPACE) ret(image, ERROR);
	if (size > 0x4000) ret(image, ERROR);	/* Limit reads to 16K */

	/* Find and check target */
	if (targ) {
		t = task_get(targ);
		if (!t) ret(image, ERROR);
		map_temp(t->map);
	}

	/* Map pages - XXX really ugly, fix or at least comment */
	for (i = 0; i < size + 0x1000; i += 0x1000) {
		if (targ) {
			if ((temp_get(src + i) & PF_PRES) == 0) ret(image, ERROR);
			page_set((uint32_t) tsrc + i,
				page_fmt(temp_get(src + i), (PF_RW | PF_PRES | PF_DISC)));
		}
		else page_set((uint32_t) tsrc + i,
			page_fmt((src + i) &~ 0xFFF, (PF_RW | PF_PRES|PF_DISC)));

		if ((page_get(dst + i) & PF_PRES) == 0) ret(image, ERROR);
		page_set((uint32_t) tdst + i,
			page_fmt(page_get(dst + i), (PF_RW | PF_PRES | PF_DISC)));
	}

	/* Copy memory */
	memcpy((void*) ((uint32_t) tdst + (dst & 0xFFF)), 
		(void*) ((uint32_t) tsrc + (src & 0xFFF)), size);
	
	ret(image, 0);
}

/***** ABI 2 System Calls *****/
image_t *fire(image_t *image) {
	ret(image, -1);
}

image_t *drop(image_t *image) {
	ret(image, -1);
}

image_t *hand(image_t *image) {
	uint32_t old_handler;

	old_handler = curr_task->shandler;
	curr_task->shandler = image->eax;
	ret(image, old_handler);
}

image_t *ctrl(image_t *image) {
	uint32_t flags = image->eax;
	uint32_t mask = image->edx;
	uint8_t irq;

	if ((curr_task->flags & TF_SUPER) == 0) {
		mask &= TF_SMASK;
	}

	curr_task->flags = (curr_task->flags & ~mask) | (flags & mask);

	if (mask & TF_IRQRD) {
		if (flags & TF_IRQRD) {
			irq = (flags >> 24) & 0xFF;
			if (irq < 15) {
				irq_holder[irq] = curr_pid;
				register_int(IRQ(irq), irq_redirect);
			}
		}
		else {
			irq = (curr_task->flags >> 24) & 0xFF;
			irq_holder[irq] = 0;
			register_int(IRQ(irq), NULL);
		}
	}

	ret(image, curr_task->flags);
}

image_t *info(image_t *image) {
	uint8_t sel = image->eax;

	switch (sel) {
	case 0: ret(image, curr_pid);
	case 1: ret(image, curr_task->parent);
	case 2: ret(image, tick);
	case 3: ret(image, 2);
	default: ret(image, -1);
	}
}

image_t *mmap(image_t *image) {
	uintptr_t addr = image->ebx;
	size_t count =   image->ecx;
	uint16_t flags = image->edx & 0xFFF;
	uint32_t frame = image->edx & ~0xFFF;
	uint16_t pflags = 0;

	if (addr & 0xFFF) ret(image, -1);
	if (addr >= KSPACE) ret(image, -1);
	if (count > 1024) ret(image, -1);

	if (flags & 0x08) {
		mem_free(addr, count * PAGESZ);
		ret(image, 0);
	}
	
	pflags |= PF_USER;
	if (flags & 0x01) pflags |= PF_PRES;
	if (flags & 0x02) pflags |= PF_RW;
	if (flags & 0x04) pflags |= PF_PRES;

	if (flags & 0x10) {
		if (curr_task->flags & TF_SUPER) {
			page_set(addr, page_fmt(frame, pflags));
			ret(image, 0);
		}
		else {
			ret(image, -1);
		}
	}

	if (flags & 0x20) {
		mem_alloc(addr, PAGESZ, pflags);
		ret(image, page_ufmt(page_get(addr)));
	}

	mem_alloc(addr, count * PAGESZ, pflags);
	ret(image, 0);
}

image_t *fork(image_t *image) {
	pid_t parent;
	task_t *child;

	/* Save current PID - it will become the parent */
	parent = curr_pid;

	/* Create a new task from the parent */
	child = task_new(task_get(parent));
	if (!child) ret(image, 0); /* Fail if child was not created */

	/* (still in parent) Set return value to child's PID */
	image->eax = child->pid;

	/* Switch to child */
	image = task_switch(child);

	/* (now in child) Set return value to negative parent's PID */
	image->eax = (uint32_t) -parent;

	return image;
}

image_t *exit(image_t *image) {
	uint32_t args[4];
	pid_t catcher;
	uint32_t ret_val;
	task_t *t;

	/* Send death signal to parent */
	catcher = curr_task->parent;

	/* Copy return value because images are cleared with the address space */
	ret_val = image->eax;

	/* If init exits, halt */
	if (curr_pid == 1) {
		panic("init died");
	}

	/* Deallocate current address space and clear metadata */
	map_clean(curr_task->map);	/* Deallocate pages and page tables */
	map_free(curr_task->map);	/* Deallocate page directory itself */
	task_rem(curr_task);		/* Clear metadata and relinquish PID */

	t = task_get(catcher);
	if (!t || !t->shandler || t->flags & TF_SBLOK) {
		/* Parent will not accept - reschedule */
		return task_switch(task_next(0));
	}
	else {
		/* Send S_DTH signal to parent with return value */
		args[0] = ret_val;
		return signal(catcher, S_DTH, args, TF_NOERR);
	}
}
