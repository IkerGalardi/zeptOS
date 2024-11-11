RESULTS=build/opensbi-dynamic.bin

build/opensbi-dynamic.bin:
	make -C firmware/ LLVM=1 PLATFORM=generic
	mv firmware/build/platform/generic/firmware/fw_dynamic.bin build/opensbi-dynamic.bin

all: $(RESULTS)

clean:
	rm -f $(RESULTS)

qemu: $(RESULTS)
	qemu-system-riscv64 -bios build/opensbi-dynamic.bin -serial stdio