
OSNAME = CustomOS

GNUEFI = ../gnu-efi
OVMFDIR = ../OVMFbin
LDS = kernel.ld
CC = gcc
LD = ld
#CC = /root/opt/cross/bin/i686-elf-gcc
#LD = /root/opt/cross/bin/i686-elf-ld

CFLAGS = -masm=intel -mno-red-zone -ffreestanding -fshort-wchar -I./include -I./libc/include
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

ASMC = nasm
ASMFLAGS = -g -f elf64
SRCDIR := src
OBJDIR := lib
BUILDDIR = bin
FONTSDIR = ../fonts
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC =  $(call rwildcard,$(SRCDIR),*.c)
ASMSRC = $(call rwildcard,$(SRCDIR),*.asm)

LIBCDIR := libc
LIBCSRC = $(call rwildcard,$(LIBCDIR),*.c)
LIBCOBJS = $(patsubst $(LIBCDIR)/%.c, $(OBJDIR)/%.libk.o, $(LIBCSRC))

OBJS  = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))

DIRS = $(wildcard $(SRCDIR)/*)

kernel: $(LIBCOBJS) $(OBJS) link

$(OBJDIR)/%.libk.o: $(LIBCDIR)/%.c
	@ echo !==== COMPILING %^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)/interrupt_handlers.o: $(SRCDIR)/interrupt_handlers.c
	@ echo !==== COMPILING %^
	@ mkdir -p $(@D)
	$(CC) -masm=intel -mno-red-zone -mgeneral-regs-only -ffreestanding -I./include -I./libc/include -c $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo !==== COMPILING %^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ echo !==== COMPILING ASM $^
	@mkdir -p $(@D)
	$(ASMC) $(ASMFLAGS) $^ -o $@

link:
	@ echo !==== LINKING %^
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(LIBCOBJS) $(OBJS)

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(SRCDIR)
	@mkdir $(OBJDIR)

buildimg:
	dd if=/dev/zero of=$(BUILDDIR)/$(OSNAME).img bs=512 count=93750
	mformat -i $(BUILDDIR)/$(OSNAME).img -f 2880 ::
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BOOTEFI) ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/$(OSNAME).img startup.nsh ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/kernel.elf ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(FONTSDIR)/console.psf ::

clean:
	@find ./ -type f -name "*.o" -exec rm -rf {} \;

all: clean kernel buildimg

run:
	qemu-system-x86_64 -machine q35 -drive file=$(BUILDDIR)/$(OSNAME).img -m 512M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none
