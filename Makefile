# Donkey OS Makefile with WSL + VirtualBox Support
# =============================================================================
# Tool Configuration
# =============================================================================

AS      := as       # GNU assembler
ASFLAGS := --32     # Generate 32-bit code

CC      := g++      # C++ compiler  
LD      := ld       # GNU linker

# VirtualBox configuration (Windows paths from WSL)
VBOX_VM_NAME := "Donkey OS"
VBOX_PATH := "/mnt/c/Program Files/Oracle/VirtualBox"

# =============================================================================
# Directory Structure
# =============================================================================

BUILD_DIR   := build
SRC_DIR     := src
INCLUDE_DIR := include

# =============================================================================
# Compiler Flags
# =============================================================================

CFLAGS := -m32 \
          -ffreestanding \
          -fno-use-cxa-atexit \
          -nostdlib \
          -fno-builtin \
          -fno-exceptions \
          -fno-rtti \
          -fno-leading-underscore \
          -Wno-write-strings \
          -I$(INCLUDE_DIR)

# =============================================================================
# Linker Flags  
# =============================================================================

LDFLAGS := -m elf_i386 -T linker.ld

# =============================================================================
# Object Files
# =============================================================================

# Assembly object files
ASM_OBJECTS := $(BUILD_DIR)/loader.o \
               $(BUILD_DIR)/interruptstubs.o

# C++ object files  
CPP_OBJECTS := $(BUILD_DIR)/gdt.o \
               $(BUILD_DIR)/port.o \
			   $(BUILD_DIR)/memory_manager.o \
               $(BUILD_DIR)/driver.o \
			   $(BUILD_DIR)/driver_manager.o \
			   $(BUILD_DIR)/terminal.o \
               $(BUILD_DIR)/interrupts.o \
			   $(BUILD_DIR)/task_scheduler.o \
			   $(BUILD_DIR)/am79c973.o \
			   $(BUILD_DIR)/pci.o \
               $(BUILD_DIR)/keyboard.o \
			   $(BUILD_DIR)/mouse.o \
               $(BUILD_DIR)/kernel.o

# All object files
ALL_OBJECTS := $(ASM_OBJECTS) $(CPP_OBJECTS)

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all iso run clean setup test vbox-start vbox-stop vbox-create help

# =============================================================================
# Main Targets
# =============================================================================

all: $(BUILD_DIR)/kernel.bin

iso: $(BUILD_DIR)/os.iso

# =============================================================================
# Build Targets
# =============================================================================

# Assembly source files
$(BUILD_DIR)/loader.o: $(SRC_DIR)/loader.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/interruptstubs.o: $(SRC_DIR)/interruptstubs.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# C++ source files
$(BUILD_DIR)/gdt.o: $(SRC_DIR)/gdt.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/port.o: $(SRC_DIR)/port.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory_manager.o: $(SRC_DIR)/memory_manager.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/driver.o: $(SRC_DIR)/driver.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/driver_manager.o: $(SRC_DIR)/driver_manager.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/terminal.o: $(SRC_DIR)/terminal.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/interrupts.o: $(SRC_DIR)/interrupts.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/task_scheduler.o: $(SRC_DIR)/task_scheduler.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/am79c973.o: $(SRC_DIR)/am79c973.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pci.o: $(SRC_DIR)/pci.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/keyboard.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mouse.o: $(SRC_DIR)/mouse.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel binary
$(BUILD_DIR)/kernel.bin: $(ALL_OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $@

# Create bootable ISO
$(BUILD_DIR)/os.iso: $(BUILD_DIR)/kernel.bin grub.cfg | $(BUILD_DIR)
	@echo "Creating bootable ISO..."
	mkdir -p iso_dir/boot/grub
	cp $(BUILD_DIR)/kernel.bin iso_dir/boot/kernel.bin
	cp grub.cfg iso_dir/boot/grub/
	grub-mkrescue -o $(BUILD_DIR)/os.iso iso_dir
	rm -rf iso_dir
	@echo "✓ ISO created: $(BUILD_DIR)/os.iso"

# =============================================================================
# VirtualBox Targets
# =============================================================================

vbox-check:
	@if [ ! -f "$(VBOX_PATH)/VBoxManage.exe" ]; then \
		echo "VirtualBox not found at: $(VBOX_PATH)"; \
		echo "Please install VirtualBox on Windows first"; \
		exit 1; \
	fi
	@echo "✓ VirtualBox found"

vbox-create: vbox-check
	@echo "Creating VirtualBox VM: $(VBOX_VM_NAME)"
	@"$(VBOX_PATH)/VBoxManage.exe" createvm --name $(VBOX_VM_NAME) --ostype "Other" --register || echo "VM might already exist"
	@"$(VBOX_PATH)/VBoxManage.exe" modifyvm $(VBOX_VM_NAME) --memory 512 --vram 16
	@"$(VBOX_PATH)/VBoxManage.exe" modifyvm $(VBOX_VM_NAME) --nic1 nat
	@"$(VBOX_PATH)/VBoxManage.exe" modifyvm $(VBOX_VM_NAME) --mouse ps2
	@"$(VBOX_PATH)/VBoxManage.exe" modifyvm $(VBOX_VM_NAME) --keyboard ps2
	@"$(VBOX_PATH)/VBoxManage.exe" modifyvm $(VBOX_VM_NAME) --boot1 dvd --boot2 none --boot3 none --boot4 none
	@"$(VBOX_PATH)/VBoxManage.exe" storagectl $(VBOX_VM_NAME) --name "IDE Controller" --add ide || echo "Storage controller might already exist"
	@echo "✓ VM created successfully!"

vbox-start: vbox-check $(BUILD_DIR)/os.iso
	@echo "Starting $(VBOX_VM_NAME)..."
	@"$(VBOX_PATH)/VBoxManage.exe" controlvm $(VBOX_VM_NAME) poweroff 2>/dev/null || true
	@sleep 2
	@"$(VBOX_PATH)/VBoxManage.exe" storageattach $(VBOX_VM_NAME) --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium "$$(wslpath -w "$(PWD)/$(BUILD_DIR)/os.iso")"
	@echo "✓ ISO attached, starting VM..."
	@"$(VBOX_PATH)/VirtualBoxVM.exe" --comment $(VBOX_VM_NAME) --startvm $(VBOX_VM_NAME) &
	@echo "VM started! Click in the window and move your mouse to test!"

vbox-stop: vbox-check
	@echo "Stopping $(VBOX_VM_NAME)..."
	@"$(VBOX_PATH)/VBoxManage.exe" controlvm $(VBOX_VM_NAME) poweroff 2>/dev/null || echo "VM was not running"
	@echo "✓ VM stopped"

vbox-remove: vbox-check
	@echo "Removing $(VBOX_VM_NAME)..."
	@"$(VBOX_PATH)/VBoxManage.exe" controlvm $(VBOX_VM_NAME) poweroff 2>/dev/null || true
	@sleep 2
	@"$(VBOX_PATH)/VBoxManage.exe" unregistervm $(VBOX_VM_NAME) --delete 2>/dev/null || echo "VM was not registered"
	@echo "✓ VM removed"

# =============================================================================
# Convenience Targets
# =============================================================================

# Complete setup
setup: vbox-create iso
	@echo ""
	@echo "Setup complete!"
	@echo "Run 'make run' to start your OS"

# Build and run
run: iso vbox-start

dev: clean run

test: iso
	@echo "✓ Build test passed"

# =============================================================================
# Legacy QEMU Targets
# =============================================================================

# Run with QEMU 
run-qemu: iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/os.iso \
		-display curses \
		-netdev user,id=net0 \
		-device pcnet,netdev=net0

# =============================================================================
# Utility Targets
# =============================================================================

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR) iso_dir
	@echo "✓ Clean complete"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

help:
	@echo "🦓 Donkey OS Build System"
	@echo "========================"
	@echo ""
	@echo "First-time setup:"
	@echo "  make setup         - Create VM and build OS (run once)"
	@echo ""
	@echo "Development workflow:"
	@echo "  make run           - Build and run OS (most common)"
	@echo "  make dev           - Clean, build, and run"
	@echo "  make test          - Just build (no run)"
	@echo ""
	@echo "Individual steps:"
	@echo "  make               - Build kernel binary"
	@echo "  make iso           - Build bootable ISO"
	@echo "  make vbox-start    - Start VirtualBox with current ISO"
	@echo "  make vbox-stop     - Stop VirtualBox VM"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean         - Clean build files"
	@echo "  make vbox-remove   - Remove VirtualBox VM"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "Mouse testing:"
	@echo "  1. Run 'make run'"
	@echo "  2. Click in VirtualBox window to capture mouse"
	@echo "  3. Move mouse to test your driver!"
