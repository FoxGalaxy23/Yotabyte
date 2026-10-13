#include "kernel.h"

char *video_memory = (char *) 0xB8000;
int cursor_row = 0;
int cursor_col = 0;

const char key_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

void clear_screen() {
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
    cursor_row = 0;
    cursor_col = 0;
}

void print_string(const char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            cursor_row++;
            cursor_col = 0;
        } else {
            int offset = (cursor_row * 80 + cursor_col) * 2;
            video_memory[offset] = str[i];
            video_memory[offset + 1] = 0x0A;
            cursor_col++;
            if (cursor_col >= 80) {
                cursor_col = 0;
                cursor_row++;
            }
        }
        i++;
    }
}

void print_int(int n) {
    if (n == 0) {
        print_string("0");
        return;
    }
    char buf[16];
    int i = 0;
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    while (i > 0) {
        char s[2] = {buf[--i], '\0'};
        print_string(s);
    }
}

void print_two_digits(int n) {
    if (n < 10) {
        print_string("0");
    }
    print_int(n);
}

char get_char() {
    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char scancode = inb(0x60);
            if (scancode & 0x80) continue;
            char c = key_map[scancode];
            if (c != 0) return c;
        }
    }
}
