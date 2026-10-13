#include "kernel.h"

unsigned char read_cmos(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}

unsigned char bcd_to_bin(unsigned char bcd) {
    return (bcd & 0x0F) + ((bcd / 16) * 10);
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void shutdown() {
    print_string("\nShutting down your system now...\n");
    __asm__ volatile("outw %0, %1" : : "a" ((unsigned short)0x2000), "Nd" ((unsigned short)0x604));
    while(1) __asm__("hlt");
}

void reboot() {
    print_string("\nRebooting...\n");
    outb(0x64, 0xFE);
    while(1) __asm__("hlt");
}

void kernel_main() {
    clear_screen();
    print_string("Welcome to Yotabyte!\n");
    print_string("For get info about commands please write 'help'\n\n");

    char cmd_buffer[256];
    int cmd_index = 0;

    print_string("User --> ");

    while (1) {
        char c = get_char();

        if (c == '\n') {
            // Переход на новую строку (используем прямой доступ к видеопамяти как у тебя)
            // или можно дописать функцию перевода строки, но пока оставим логику
            print_string("\n");
            cmd_buffer[cmd_index] = '\0';

            if (strcmp(cmd_buffer, "time") == 0) {
                unsigned char second = bcd_to_bin(read_cmos(0x00));
                unsigned char minute = bcd_to_bin(read_cmos(0x02));
                unsigned char hour   = bcd_to_bin(read_cmos(0x04));

                print_string("Time : ");
                print_two_digits(hour);
                print_string(":");
                print_two_digits(minute);
                print_string(":");
                print_two_digits(second);
                print_string("\n");
            }
            else if (strcmp(cmd_buffer, "date") == 0) {
                unsigned char day   = bcd_to_bin(read_cmos(0x07));
                unsigned char month = bcd_to_bin(read_cmos(0x08));
                unsigned char year  = bcd_to_bin(read_cmos(0x09));

                print_string("date: ");
                print_two_digits(day);
                print_string(".");
                print_two_digits(month);
                print_string(".20");
                print_two_digits(year);
                print_string("\n");
            }
            else if (strcmp(cmd_buffer, "help") == 0) {
                print_string("shutdown -> turn off your computer now\n");
                print_string("reboot -> reboot your pc\n");
                print_string("time -> check the time\n");
                print_string("date -> check the date\n");
                print_string("clear -> just clear the display\n");
                print_string("echo 'text' -> print the text\n");
            }
            else if (strcmp(cmd_buffer, "shutdown") == 0) {
                shutdown();
            }
            else if (strcmp(cmd_buffer, "reboot") == 0) {
                reboot();
            }
            else if (strcmp(cmd_buffer, "clear") == 0) {
                clear_screen();
            }
            else if (cmd_buffer[0] == 'e' && cmd_buffer[1] == 'c' && cmd_buffer[2] == 'h' && cmd_buffer[3] == 'o' && cmd_buffer[4] == ' ') {
                print_string(&cmd_buffer[5]);
                print_string("\n");
            }
            else if (cmd_index > 0) {
                print_string("idk what is this 0_0\n");
            }

            cmd_index = 0;
            print_string("User --> ");
        }
        else if (c == '\b') {
            if (cmd_index > 0) {
                cmd_index--;
                // Логику стирания символа на экране можно оставить или тоже вынести
                print_string("\b \b");
            }
        }
        else {
            if (cmd_index < 255) {
                cmd_buffer[cmd_index++] = c;
                char s[2] = {c, '\0'};
                print_string(s);
            }
        }
    }
}