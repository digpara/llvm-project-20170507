// REQUIRES: aarch64
// RUN: llvm-mc -filetype=obj -triple=aarch64-none-freebsd %s -o %t.o
// RUN: not ld.lld -shared %t.o -o %t.so 2>&1 | FileCheck %s
// CHECK: can't create dynamic relocation R_AARCH64_LDST64_ABS_LO12_NC against symbol: dat
// CHECK: >>> defined in {{.*}}.o
// CHECK: >>> referenced by {{.*}}.o:(.text+0x0)

  ldr x0, [x0, :lo12:dat]
.data
.globl dat
dat:
  .word 0
