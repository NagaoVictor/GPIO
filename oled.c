#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEV_PATH "/dev/i2c-1"
#define OLED_ADDR    0x3C

void set_conio_terminal_mode(struct termios *orig_opts) {
    struct termios new_opts;
    tcgetattr(STDIN_FILENO, orig_opts);
    new_opts = *orig_opts;
    new_opts.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}

void reset_terminal_mode(struct termios *orig_opts) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig_opts);
}

void oled_send_command(int file, unsigned char cmd) {
    unsigned char buf[2] = {0x00, cmd};
    write(file, buf, 2);
    usleep(1000); // Pequeno atraso para o SSD1306 processar o comando
}

void oled_init(int file) {
    unsigned char init_sequence[] = {
        0x00,       
        0xAE,       // Display OFF
        0xD5, 0x80, // Set Display Clock Divide Ratio
        0xA8, 0x3F, // Set Multiplex Ratio (1/64)
        0xD3, 0x00, // Set Display Offset
        0x40,       // Set Display Start Line
        0x8D, 0x14, // Set Charge Pump Enable
        0x20, 0x00, // Set Memory Addressing Mode (Horizontal)
        0xA1,       // Set Segment Re-map
        0xC8,       // Set COM Output Scan Direction
        0xDA, 0x12, // Set COM Pins Hardware Configuration
        0x81, 0xCF, // Set Contrast Control
        0xD9, 0xF1, // Set Pre-charge Period
        0xDB, 0x40, // Set VCOMH Deselect Level
        0xA4,       // Entire Display On
        0xA6,       // Set Normal Display
        0xAF        // Display ON
    };
    write(file, init_sequence, sizeof(init_sequence));
    usleep(10000); // 10ms para estabilizar após ligar
}

void oled_clear(int file) {
    oled_send_command(file, 0x21); // Set Column Address
    oled_send_command(file, 0x00); // Start Column: 0
    oled_send_command(file, 0x7F); // End Column: 127
    
    oled_send_command(file, 0x22); // Set Page Address
    oled_send_command(file, 0x00); // Start Page: 0
    oled_send_command(file, 0x07); // End Page: 7

    unsigned char data[129];
    data[0] = 0x40; // Indica dados para a RAM
    for (int i = 1; i < 129; i++) {
        data[i] = 0x00;
    }

    for (int page = 0; page < 8; page++) {
        write(file, data, 129);
        usleep(1000);
    }
}

int main() {
    struct termios orig_opts;
    int i2c_fd;

    if ((i2c_fd = open(I2C_DEV_PATH, O_RDWR)) < 0) {
        perror("Erro ao abrir /dev/i2c-1");
        return 1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Erro ao configurar o endereço do OLED");
        close(i2c_fd);
        return 1;
    }

    printf("Inicializando e limpando o OLED SSD1306...\n");
    oled_init(i2c_fd);
    oled_clear(i2c_fd);

    set_conio_terminal_mode(&orig_opts);
    printf("Sistema pronto! Digite no teclado (Pressione ESC para sair):\n");

    char c;
    while (1) {
        c = getchar(); 
        if (c == 27) break; // ESC

        printf("Tecla enviada: %c\n", c);
    }

    oled_send_command(i2c_fd, 0xAE);
    reset_terminal_mode(&orig_opts);
    close(i2c_fd);
    return 0;
}