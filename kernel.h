#ifndef KERNEL_H
#define KERNEL_H

// Низкоуровневые функции портов
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}

// Прототипы функций экрана и ввода
void clear_screen();
void print_string(const char *str);
void print_int(int n);
void print_two_digits(int n);
char get_char();

// Функции железа (CMOS, питание)
unsigned char read_cmos(unsigned char reg);
unsigned char bcd_to_bin(unsigned char bcd);
void shutdown();
void reboot();

// Стандартные функции
int strcmp(const char *s1, const char *s2);

#endif
