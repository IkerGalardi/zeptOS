FROM alpine:edge

RUN apk add clang make lld llvm
RUN apk add bash perl
RUN apk add qemu-system-riscv64

RUN apk add gdb-multiarch
RUN mkdir -p root/.config/gdb &&\
    echo "set auto-load safe-path /" > /root/.config/gdb/gdbinit
