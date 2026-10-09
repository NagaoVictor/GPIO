#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEV_PATH "/dev/i2c-1"
#define OLED_ADDR    0x3C

// Tabela de fonte 5x7 simplificada para dígitos (0-9)
static const unsigned char font5x7[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};

void set_conio_terminal_mode(struct termios *orig_opts) {
    struct termios new_opts;
    tcgetattr(STDIN_FILENO, orig_opts);
    memcpy(&new_opts, orig_opts, sizeof(struct termios));
    new_opts.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}

void reset_terminal_mode(struct termios *orig_opts) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig_opts);
}

void send_cmd(int fd, unsigned char cmd) {
    unsigned char buf[2] = {0x00, cmd};
    write(fd, buf, 2);
    usleep(200);
}

void oled_init(int fd) {
    send_cmd(fd, 0xAE); // Display OFF
    send_cmd(fd, 0xD5); send_cmd(fd, 0x80);
    send_cmd(fd, 0xA8); send_cmd(fd, 0x3F);
    send_cmd(fd, 0xD3); send_cmd(fd, 0x00);
    send_cmd(fd, 0x40);
    send_cmd(fd, 0x8D); send_cmd(fd, 0x14); // Charge pump ON
    send_cmd(fd, 0x20); send_cmd(fd, 0x00); // Horizontal mode
    send_cmd(fd, 0xA1);
    send_cmd(fd, 0xC8);
    send_cmd(fd, 0xDA); send_cmd(fd, 0x12);
    send_cmd(fd, 0x81); send_cmd(fd, 0xCF);
    send_cmd(fd, 0xD9); send_cmd(fd, 0xF1);
    send_cmd(fd, 0xDB); send_cmd(fd, 0x40);
    send_cmd(fd, 0xA4);
    send_cmd(fd, 0xA6);
    send_cmd(fd, 0xAF); // Display ON
}

void oled_clear(int fd) {
    send_cmd(fd, 0x21); send_cmd(fd, 0x00); send_cmd(fd, 0x7F);
    send_cmd(fd, 0x22); send_cmd(fd, 0x00); send_cmd(fd, 0x07);

    unsigned char block[129];
    block[0] = 0x40;
    memset(&block[1], 0x00, 128);

    for (int p = 0; p < 8; p++) {
        write(fd, block, 129);
        usleep(200);
    }
}

void oled_draw_digit(int fd, int digit, int col, int page) {
    if (digit < 0 || digit > 9) return;

    send_cmd(fd, 0x21); send_cmd(fd, col); send_cmd(fd, col + 5);
    send_cmd(fd, 0x22); send_cmd(fd, page); send_cmd(fd, page);

    unsigned char data[7];
    data[0] = 0x40; 
    memcpy(&data[1], font5x7[digit], 5);
    data[6] = 0x00; 

    write(fd, data, 7);
}

int main() {
    struct termios orig_opts;
    int fd = open(I2C_DEV_PATH, O_RDWR);
    if (fd < 0) {
        perror("Erro ao abrir I2C");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Erro ao configurar endereço I2C");
        close(fd);
        return 1;
    }

    oled_init(fd);
    oled_clear(fd); // Garante que a tela inicia limpa e ativa

    set_conio_terminal_mode(&orig_opts);
    printf("Sistema pronto! Digite numeros de 0 a 9 (ESC para sair):\n");

    int current_col = 0;
    int current_page = 0;

    char c;
    while (1) {
        c = getchar();
        if (c == 27) break; // ESC

        if (c >= '0' && c <= '9') {
            int digit = c - '0';
            oled_draw_digit(fd, digit, current_col, current_page);
            printf("Dígito %d impresso na coluna %d\n", digit, current_col);

            current_col += 6;
            if (current_col > 120) {
                current_col = 0;
                current_page = (current_page + 1) % 8;
            }
        }
    }

    send_cmd(fd, 0xAE);
    reset_terminal_mode(&orig_opts);
    close(fd);
    return 0;
}