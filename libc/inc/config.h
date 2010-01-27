#ifndef CONFIG_H
#define CONFIG_H

/********** GLOBAL OPTIONS **********/

	/* Enable checks on privileged code */
/*	#define PARANOID */

/********** KERNEL OPTIONS **********/

	/***** TASKING SUBSYSTEM *****/

	/* Maximum number of tasks, must be less than 65536 */
	#define MAX_TASKS 4096

	/* Scheduling algorithm */
	#define SCHED_ROUND_ROBIN 0
	#define SCHED_VAR_QUANTUM 1 /* Incomplete */
	#define SCHED_ALG SCHED_ROUND_ROBIN

	/***** MEMORY SUBSYSTEM *****/

	#define PAGESZ 0x1000

	/* These must remain in order */
	#define ESPACE	0xFC000000	/* Temporary space for exec data */
	#define SSPACE	0xFCF00000	/* Stack space (grows down from next address) */
	#define LSPACE	0xFD000000	/* Space for exec bootstrap */
	#define KSPACE 	0xFE000000	/* Kernel space */
	#define KMALLOC 0xFE200000	/* Kernel init allocator */

	/* Two areas of 16 pages for temporary mappings */
	#define TMP_DST 0xFF000000
	#define TMP_SRC 0xFF010000

	#define TMP_MAP	0xFF800000
	#define PGE_MAP	0xFFC00000

	/* Address of task table */
	#define TASK_TBL (KSPACE + 0x400000)

	/* Task image stack */
	#define SSTACK_BSE	(LSPACE - 0x7000)
	#define SSTACK_INI	(LSPACE - 0x2000)
	#define SSTACK_TOP	(LSPACE - 0x2000)

	/* User stack */
	#define USTACK_BSE	(LSPACE - 0x10000)
	#define USTACK_INI	(LSPACE - 0x8010)
	#define USTACK_TOP	(LSPACE - 0x8000)

	/* Exec bootstrap location */
	#define EXEC_STRAP	(SSPACE - 0x100000)

	/* Remove init code after boot (experimental) */
/*	#define KERNEL_GC */

/********** DRIVER OPTIONS **********/

	/***** DEVICE DETECTION *****/

	/* Bus scanning */
	#define SCAN_PCI
/*	#define SCAN_ISA */
	#define SCAN_ATA

	/***** VIDEO SETTINGS *****/
	
	/* Video mode */
	#define VIDEO_TEXT
/*	#define VIDEO_VGA */
/*	#define VIDEO_VESA */

/********** C LIBRARY OPTIONS **********/

	/***** HEAP *****/

	#define EXEC_START 0x00400000
	#define HEAP_START 0x10000000
	#define HEAP_MXBRK 0x20000000

#endif
