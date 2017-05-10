#include <kernel.h>

// prints a char to the current standard output
// for now, it will just print raw to the text driver
void char_to_stdout(int c) {
    text_putchar(c);
    return;
}

// gets a char from the current standard input
// for now, it will get the char from the keyboard buffer
// returns 0 if no input
// returns char on input
// DOES NOT HALT EXECUTION UNTIL INPUT
int char_from_stdin(void) {
    int c;
    c = keyboard_fetch_char();
    return c;
}
