FROM alpine:3.21.3

RUN apk add clang19 make lld llvm
RUN apk add qemu-system-riscv64
