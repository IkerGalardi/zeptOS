FROM alpine:edge

RUN apk add clang make lld llvm
RUN apk add bash perl
RUN apk add qemu-system-riscv64
