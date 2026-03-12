#=============================================================================
# RetroBoot Makefile
# Builds a UEFI application (.efi) for x86-64
#=============================================================================

CC      = x86_64-w64-mingw32-gcc
OBJCOPY = x86_64-w64-mingw32-objcopy

SOURCES = retroboot.c
TARGET  = RETROBOOT.EFI

CFLAGS  = \
    -Wall -Wextra -std=c11 -O2          \
    -ffreestanding                       \
    -fno-stack-protector                 \
    -fno-stack-check                     \
    -fno-strict-aliasing                 \
    -mno-red-zone                        \
    -maccumulate-outgoing-args           \
    -nostdlib                            \
    -mabi=ms                             \
    -DRETROBOOT_BUILD

LDFLAGS = \
    -Wl,--subsystem,10                   \
    -Wl,-dll                             \
    -Wl,--entry,efi_main                 \
    -Wl,--file-alignment,512             \
    -Wl,--section-alignment,4096         \
    -Wl,-Map,retroboot.map               \
    -nostdlib                            \
    -shared

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SOURCES) retroboot.h
	$(CC) $(CFLAGS) $(LDFLAGS) -e efi_main $(SOURCES) -o $(TARGET)
	@echo ""
	@echo "  Build successful: $(TARGET)"
	@echo "  Copy to:  /EFI/BOOT/BOOTX64.EFI  on a FAT32 ESP"
	@echo ""

# Create a bootable FAT32 disk image (requires mtools + dosfstools)
image: $(TARGET)
	dd if=/dev/zero of=retroboot.img bs=1M count=64
	mkfs.fat -F 32 retroboot.img
	mmd     -i retroboot.img ::/EFI
	mmd     -i retroboot.img ::/EFI/BOOT
	mcopy   -i retroboot.img $(TARGET) ::/EFI/BOOT/BOOTX64.EFI
	@echo "  Disk image: retroboot.img"

# Test in QEMU with OVMF UEFI firmware
# Requires: qemu-system-x86_64 + OVMF (apt install ovmf)
OVMF_CODE = /usr/share/ovmf/OVMF.fd
OVMF_VARS = ovmf_vars.fd

qemu: image
	test -f $(OVMF_VARS) || cp $(OVMF_CODE) $(OVMF_VARS)
	qemu-system-x86_64                           \
	    -enable-kvm                              \
	    -m 512M                                  \
	    -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	    -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	    -drive format=raw,file=retroboot.img     \
	    -serial stdio                            \
	    -vga std                                 \
	    -monitor telnet:127.0.0.1:1234,server,nowait

# Same but without KVM (works in VMs/CI)
qemu-nokvm: image
	test -f $(OVMF_VARS) || cp $(OVMF_CODE) $(OVMF_VARS)
	qemu-system-x86_64                           \
	    -m 512M                                  \
	    -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	    -drive if=pflash,format=raw,file=$(OVMF_VARS) \
	    -drive format=raw,file=retroboot.img     \
	    -serial stdio                            \
	    -vga std

# Install to a mounted ESP (set ESPDIR=/path/to/esp)
ESPDIR ?= /mnt/esp
install: $(TARGET)
	@test -d $(ESPDIR) || (echo "Set ESPDIR=/path/to/mounted/esp"; exit 1)
	mkdir -p $(ESPDIR)/EFI/BOOT
	cp $(TARGET) $(ESPDIR)/EFI/BOOT/BOOTX64.EFI
	sync
	@echo "  Installed to $(ESPDIR)/EFI/BOOT/BOOTX64.EFI"

clean:
	rm -f $(TARGET) retroboot.img retroboot.map *.o $(OVMF_VARS)