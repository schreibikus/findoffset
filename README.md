findoffset
==========

The application to get an offset in a file that contains content from another file.
I use this application to find in the NAND image the places where certain code are located.
Example of use:
./findoffset -p bootloader.bin -p kernel.bin -p filesystem.bin flashdump.bin
file bootloader.bin placed at offset 0x40000 len 0x2704 (9988) bytes!
file bootloader.bin placed at offset 0x60000 len 0x2704 (9988) bytes!
file kernel.bin placed at offset 0x140000 len 0x2E010 (188432) bytes!
file kernel.bin placed at offset 0x180000 len 0x2E010 (188432) bytes!
file filesystem.bin placed at offset 0x1C0000 len 0x230000 (2293760) bytes!
file filesystem.bin placed at offset 0x400000 len 0x230000 (2293760) bytes!



For license see https://www.gnu.org/licenses/gpl-3.0.en.html
