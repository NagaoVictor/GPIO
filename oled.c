#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEV_PATH "/dev/i2c-1"
#define OLED_ADDR    0x3C

// Função para configurar o terminal em modo raw (sem precisar de Enter)
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

// Envia um byte de comando para o SSD1306
void oled_send_command(int file, unsigned char cmd) {
    unsigned char buf[2] = {0x00, cmd}; // 0x00 indica fluxo de comando
    write(file, buf, 2);
}

// Inicialização padrão para acender o OLED SSD1306 (128x64)
void oled_init(int file) {
    unsigned char init_sequence[] = {
        0x00,       // Byte de controle: Comando
        0xAE,       // Display OFF
        0xD5, 0x80, // Set Display Clock Divide Ratio / Oscillator Frequency
        0xA8, 0x3F, // Set Multiplex Ratio (1/64)
        0xD3, 0x00, // Set Display Offset
        0x40,       // Set Display Start Line
        0x8D, 0x14, // Set Charge Pump Enable (0x14 ativa a bomba interna de tensão)
        0x20, 0x00, // Set Memory Addressing Mode (Horizontal)
        0xA1,       // Set Segment Re-map (A0 ou A1 dependendo da orientação)
        0xC8,       // Set COM Output Scan Direction
        0xDA, 0x12, // Set COM Pins Hardware Configuration
        0x81, 0xCF, // Set Contrast Control
        0xD9, 0xF1, // Set Pre-charge Period
        0xDB, 0x40, // Set VCOMH Deselect Level
        0xA4,       // Entire Display On (Resume)
        0xA6,       // Set Normal Display (não invertido)
        0xAF        // Display ON! (Aqui ele acende)
    };
    write(file, init_sequence, sizeof(init_sequence));
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

    printf("Inicializando o OLED SSD1306...\n");
    oled_init(i2c_fd);

    set_conio_terminal_mode(&orig_opts);
    printf("Sistema pronto! Digite no teclado (Pressione ESC para sair):\n");

    char c;
    while (1) {
        c = getchar(); 
        if (c == 27) break; // ESC

        printf("Tecla enviada: %c\n", c);
        
        // Aqui você pode disparar comandos baseados na tecla digitada
        // Por exemplo, piscar o display ou mandar dados de controle
    }

    // Limpa e desliga o display ao sair
    oled_send_command(i2c_fd, 0xAE); // Display OFF

    reset_terminal_mode(&orig_opts);
    close(i2c_fd);
    return 0;
}