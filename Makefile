# Tools
AS      := as
ASFLAGS := --32
CC      := g++
LD      := ld
CFLAGS  := -m32 -std=gnu++20 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS := -m elf_i386 -T linker.ld

BUILD_DIR := build
SRC_DIR   := src

.PHONY: all iso run clean

all: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/loader.o: $(SRC_DIR)/loader.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/loader.o $(BUILD_DIR)/kernel.o linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(BUILD_DIR)/loader.o $(BUILD_DIR)/kernel.o -o $@

$(BUILD_DIR)/os.iso: $(BUILD_DIR)/kernel.bin grub.cfg | $(BUILD_DIR)
	mkdir -p isodir/boot/grub
	cp $(BUILD_DIR)/kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(BUILD_DIR)/os.iso isodir

iso: $(BUILD_DIR)/os.iso

run: iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/os.iso \
		-curses

clean:
	rm -rf $(BUILD_DIR) isodir

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
