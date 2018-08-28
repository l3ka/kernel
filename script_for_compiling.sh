#!/bin/bash

#  Title: Windows X
#  By: Darko Lekic (lekic.d7@gmail.com)
#  By: Nikola Pejic (peja96bl@gmail.com)
#  Description: Compiler Script

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

rm *.o
rm -r -f isodir

nasm -felf32 start.asm -o start.o
i686-elf-gcc -c main.c -o main.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c scrn.c -o scrn.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c gdt.c -o gdt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c idt.c -o idt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c isrs.c -o isrs.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c irq.c -o irq.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c timer.c -o timer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c kb.c -o kb.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c beep.c -o beep.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c mouse.c -o mouse.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include
i686-elf-gcc -c digital_watch.c -o digital_watch.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I./include

i686-elf-gcc -T link.ld -o Windows_X.bin -ffreestanding -O2 -nostdlib start.o main.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o beep.o digital_watch.o mouse.o -lgcc
grub-file --is-x86-multiboot Windows_X.bin
if grub-file --is-x86-multiboot Windows_X.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi
rm -r isodir
mkdir -p isodir/boot/grub
cp Windows_X.bin isodir/boot/Windows_X.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o Windows_X.iso isodir --xorriso=./xorriso/xorriso-1.4.6/xorriso/xorriso
# qemu-system-i386 -cdrom Windows_X.iso

rm *.o
# rm -r -f isodir

