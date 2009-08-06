// Copyright 2009 Nick Johnson

#include <lib.h>
#include <mem.h>

pool_t fpool[MAX_PHMEM / 4];

void page_touch(u32int page) {
	page &= ~0x3FFFFF;
	u32int target = (u32int) &ctbl[page >> 12];
	if (cmap[page >> 22] & PF_PRES) return;
	cmap[page >> 22] = frame_new() | (PF_PRES | PF_RW | PF_USER);
	asm volatile ("invlpg %0" :: "m" (target));
	pgclr(&ctbl[page >> 12]);
}

void page_set(u32int page, page_t value) {
	page &= ~0xFFF;
	if ((cmap[page >> 22] & PF_PRES) == 0) page_touch(page);
	ctbl[page >> 12] = value;
	asm volatile ("invlpg %0" :: "m" (page));
}

page_t page_get(u32int page) {
	return (cmap[page >> 22] & PF_PRES) ? ctbl[page >> 12] : 0;
}

void page_flush() {
	u32int cr3;
	asm volatile("mov %%cr3, %0" : "=r" (cr3));
	asm volatile("mov %0, %%cr3" : : "r" (cr3));
}
