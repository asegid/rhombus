/* Copyright 2010 Nick Johnson */

#include <stdint.h>
#include <flux.h>

#include <driver/ata.h>

static void ata_pio_write  (uint8_t drive, uint64_t sector, uint16_t *buffer);
static void ata_pio_read   (uint8_t drive, uint64_t sector, uint16_t *buffer);
static void atapi_pio_write(uint8_t drive, uint64_t sector, uint16_t *buffer);
static void atapi_pio_read (uint8_t drive, uint64_t sector, uint16_t *buffer);

void pio_write_sector(uint8_t drive, uint64_t sector, uint16_t *buffer) {
	ata_pio_write(drive, sector, buffer);
}

void pio_read_sector(uint8_t drive, uint64_t sector, uint16_t *buffer) {
	ata_pio_read(drive, sector, buffer);
}

static void ata_pio_read(uint8_t drive, uint64_t sector, uint16_t *buffer) {
	size_t i;
	uint8_t err;
	bool lba48 = false;

	/* must not be interrupted */
	sigblock(true);

	/* send LBA to controller */
	lba48 = ata_send_lba(drive, sector);

	if (lba48) {
		outb(ata_base[drive] + REG_CMD, CMD_READ_PIO48);
	}
	else {
		outb(ata_base[drive] + REG_CMD, CMD_READ_PIO);
	}

	/* read in one sector of words */
	for (i = 0; i < SECTSIZE / sizeof(uint16_t); i++) {
		ata_sleep400(drive);
		while (inb(ata_base[drive] + REG_STAT) & STAT_BUSY);
		buffer[i] = inw(ata_base[drive] + REG_DATA);
	}

	sigblock(false);
}

static void ata_pio_write(uint8_t drive, uint64_t sector, uint16_t *buffer) {
	size_t i;
	uint8_t err;
	bool lba48 = false;

	/* must not be interrupted */
	sigblock(true);

	lba48 = ata_send_lba(drive, sector);

	if (lba48) {
		outb(ata_base[drive] + REG_CMD, CMD_WRITE_PIO48);
	}
	else {
		outb(ata_base[drive] + REG_CMD, CMD_WRITE_PIO);
	}

	/* write one sector of words */
	for (i = 0; i < SECTSIZE / sizeof(uint16_t); i++) {
		ata_sleep400(drive);
		while (inb(ata_base[drive] + REG_STAT) & STAT_BUSY);
		outw(ata_base[drive] + REG_DATA, buffer[i]);
	}

	/* flush write cache */
	outb(ata_base[drive] + REG_CMD, CMD_CACHE_FLUSH);
	while (inb(ata_base[drive] + REG_STAT) & STAT_BUSY);

	sigblock(false);
}
