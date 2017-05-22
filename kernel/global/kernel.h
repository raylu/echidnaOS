// general kernel header for direct (no syscall) function calling

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>
#include <systemasm.h>

void vga_disable_cursor(void);
void vga_80_x_50(void);
void disk_load_sector(uint8_t drive, uint8_t* target_address, uint32_t source_sector, uint32_t count);
uint32_t detect_mem(void);

// from "variables.h"

extern uint32_t memory_size;
extern uint32_t kernel_size;
extern uint32_t available_page;

// from "io.h"

void char_to_stdout(int c);
int char_from_stdin(void);

// from "memory.h"

void memory_map(uint32_t directory_number, uint32_t phys_address, uint32_t virt_address, uint8_t access);

// from "panic.h"

void panic(const char *msg);

// from "textdrv.h"

void text_putchar(char c);
int text_get_cursor_pos_x(void);
int text_get_cursor_pos_y(void);
void text_set_cursor_pos(int x, int y);
void text_set_cursor_palette(char c);
char text_get_cursor_palette(void);
void text_set_text_palette(char c);
char text_get_text_palette(void);
void text_clear(void);
void text_disable_cursor(void);
void text_enable_cursor(void);
void text_putstring(const char *string);
void text_putascii(const char *string, uint32_t length);

// from "pic.h"

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);

// from "gdt.h"

void load_GDT(void);

// from "idt.h"

void create_IDT(void);

// from "keyboard.h"

void keyboard_init(void);
void keyboard_handler(uint8_t input_byte);
char keyboard_fetch_char(void);

// end

#endif
