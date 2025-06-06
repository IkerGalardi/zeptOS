# zeptOS

zeptOS is a minimal software stack comprised of firmware and operating system
for embedded applications where the processor is powerful enought to support MMU
and other security features but is too slow to run a fully fledged linux
distribution or *BSD.

## Build instructions

The dependencies are necessary:

- **llvm toolchain** (needs to support CFI extensions)
- **gdb** (in case debugging is necessary)
- **qemu-system-riscv64** (needs to support CFI extensions)

Even though a Dockerfile is available on the repository, its not completelly
necessary to use if you already have a recent enough clang version (version 20.0
onwards). In case the environment needs to be used, just execute the next commands:

```
  make buildenv
  make env
```

Once the proper environment is prepared, simply executing `make` will compile the firmware, kernel, userspace and file system image. To emulate the operating system simply execute `make qemu`.

Qemu monitor is not enabled by default, so Ctrl+C will be passed to the operating system. In case qemu needs to be stopped simply Ctrl+A then Ctrl+X.

In case debugging is neccessary execute `make qemu-gdb` and by simply executing gdb (gdb needs to support the riscv architecture, usually installing the gdb-multiarch package is necessary) on the project directory attaches the debugger to qemu. That point on, simply add a breakpoint and run the `continue` command.
