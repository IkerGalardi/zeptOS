CC=riscv64-linux-gnu-gcc
AS=riscv64-linux-gnu-as
LD=riscv64-linux-gnu-ld
OBJCOPY=riscv64-linux-gnu-objcopy
OBJDUMP=riscv64-linux-gnu-objdump

RESULTS=build/opensbi-dynamic.bin \
        build/kernel \
        build/fs.img

all: $(RESULTS)

build/opensbi-dynamic.bin:
	make -C firmware/ PLATFORM=generic CROSS_COMPILE=riscv64-linux-gnu-
	mv firmware/build/platform/generic/firmware/fw_dynamic.bin build/opensbi-dynamic.bin

build/kernel:
	make -C os/ kernel/kernel CC=$(CC) AS=$(AS) LD=$(LD)
	mv os/kernel/kernel build/kernel

build/fs.img:
	make -C os/ fs.img
	mv os/fs.img build/fs.img

clean:
	rm -f $(RESULTS)
	make -C firmware/ clean
	make -C os clean

QEMUOPTS  = -machine virt -bios build/opensbi-dynamic.bin -kernel build/kernel
QEMUOPTS += -m 128M -smp 4 -nographic -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=build/fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

qemu-mmode: $(RESULT)
	make -C os/ qemu

qemu: $(RESULTS)
	qemu-system-riscv64 $(QEMUOPTS)