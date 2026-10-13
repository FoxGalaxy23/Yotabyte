static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}
const char key_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

char *video_memory = (char *) 0xB8000;
int cursor_row = 0;
int cursor_col = 0;

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
    // Выводим в обратном порядке
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

unsigned char read_cmos(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}

unsigned char bcd_to_bin(unsigned char bcd) {
    return (bcd & 0x0F) + ((bcd / 16) * 10);
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
            video_memory[((cursor_row * 80 + cursor_col) * 2)] = '\n';
            cursor_row++;
            cursor_col = 0;
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
                cursor_col--;
                int offset = (cursor_row * 80 + cursor_col) * 2;
                video_memory[offset] = ' ';
                video_memory[offset + 1] = 0x07;
            }
        } 
        else {
            if (cmd_index < 255) {
                cmd_buffer[cmd_index++] = c;
                int offset = (cursor_row * 80 + cursor_col) * 2;
                video_memory[offset] = c;
                video_memory[offset + 1] = 0x0A;
                cursor_col++;
                if (cursor_col >= 80) {
                    cursor_col = 0;
                    cursor_row++;
                }
            }
        }
    }
}
