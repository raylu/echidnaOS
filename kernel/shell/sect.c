#include <stdio.h>
#include <kernel.h>
#include "shell.h"

void sect_cmd(void) {
    uint8_t* test_sector=kmalloc(512);

    disk_load_sector(0x80, test_sector, 0, 1);

    text_putascii(test_sector, 512);
    
    return;
}
