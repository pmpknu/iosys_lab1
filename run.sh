#!/bin/bash
set -xue

# Путь QEMU 
QEMU=qemu-system-riscv32

# Запуск QEMU
$QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot

