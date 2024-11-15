CC=riscv64-unknown-elf-gcc
AS=riscv64-unknown-elf-as
LD=riscv64-unknown-elf-ld

RESULTS=build/opensbi-dynamic.bin

build/opensbi-dynamic.bin:
	make -C firmware/ PLATFORM=generic CROSS_COMPILE=riscv64-linux-gnu-
	mv firmware/build/platform/generic/firmware/fw_dynamic.bin build/opensbi-dynamic.bin

all: $(RESULTS)

clean:
	rm -f $(RESULTS)
	make -C firmware/ clean

qemu: $(RESULTS)
	qemu-system-riscv64 -bios build/opensbi-dynamic.bin -serial stdio