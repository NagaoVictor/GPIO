#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEV_PATH "/dev/i2c-1"
#define OLED_ADDR    0x3C

// Configura o terminal para leitura instantânea (sem precisar de Enter)
void set_conio_terminal_mode(struct termios *orig_opts) {
    struct termios new_opts;
    tcgetattr(STDIN_FILENO, orig_opts);
    new_opts = *orig_opts;
    new_opts.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}

void reset_terminal_mode(struct termios *orig_opts) {
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}

// Envia um comando isolado para o SSD1306 com pequeno delay
void oled_send_command(int file, unsigned char cmd) {
    unsigned char buf[2] = {0x00, cmd}; // 0x00 = indica comando
    write(file, buf, 2);
    usleep(500);
}

// Sequência completa e robusta de inicialização
void oled_init(int file) {
    oled_send_command(file, 0xAE); // Display OFF
    oled_send_command(file, 0xD5); // Set Display Clock Divide Ratio
    oled_send_command(file, 0x80);
    oled_send_command(file, 0xA8); // Set Multiplex Ratio (1/64)
    oled_send_command(file, 0x3F);
    oled_send_command(file, 0xD3); // Set Display Offset
    oled_send_command(file, 0x00);
    oled_send_command(file, 0x40); // Set Display Start Line
    oled_send_command(file, 0x8D); // Set Charge Pump Enable
    oled_send_command(file, 0x14); // Ativa bomba de alta tensão interna
    oled_send_command(file, 0x20); // Set Memory Addressing Mode
    oled_send_command(file, 0x00); // Horizontal addressing mode
    oled_send_command(file, 0xA1); // Set Segment Re-map
    oled_send_command(file, 0xC8); // Set COM Output Scan Direction
    oled_send_command(file, 0xDA); // Set COM Pins Hardware Configuration
    oled_send_command(file, 0x12);
    oled_send_command(file, 0x81); // Set Contrast Control
    oled_send_command(file, 0xCF);
    oled_send_command(file, 0xD9); // Set Pre-charge Period
    oled_send_command(file, 0xF1);
    oled_send_command(file, 0xDB); // Set VCOMH Deselect Level
    oled_send_command(file, 0x40);
    oled_send_command(file, 0xA4); // Entire Display On
    oled_send_command(file, 0xA6); // Set Normal Display (não invertido)
    oled_send_command(file, 0xAF); // Display ON (Acende!)
    usleep(10000);
}

// Limpa a tela preenchendo a RAM com zeros
void oled_clear(int file) {
    oled_send_command(file, 0x21); // Column Address
    oled_send_command(file, 0x00);
    oled_send_command(file, 0x7F);
    
    oled_send_command(file, 0x22); // Page Address
    oled_send_command(file, 0x00);
    oled_send_command(file, 0x07);

    unsigned char data[129];
    data[0] = 0x40; // 0x40 = indica fluxo de dados para a RAM
    for (int i = 1; i < 129; i++) {
        data[i] = 0x00; // Apaga os pixels
    }

    for (int page = 0; page < 8; page++) {
        write(file, data, 129);
        usleep(500);
    }
}

int main() {
    struct termios orig_opts;
    int i2c_fd;

    // Antes de rodar, lembre-se de dar o ciclo rápido de tirar e colocar o VCC do OLED para destravar o chip!

    if ((i2c_fd = open(I2C_DEV_PATH, O_RDWR)) < 0) {
        perror("Erro ao abrir /dev/i2c-1");
        return 1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Erro ao configurar o endereço do OLED");
        close(i2c_fd);
        return 1;
    }

    printf("Inicializando o OLED em C de baixo nivel...\n");
    oled_init(i2c_fd);
    oled_clear(i2c_fd);

    set_conio_terminal_mode(&orig_opts);
    printf("Sistema pronto! Digite no teclado (Pressione ESC para sair):\n");

    char c;
    while (1) {
        c = getchar(); 
        if (c == 27) break; // ESC

        printf("Tecla pressionada: %c\n", c);
        
        // Aqui podemos futuramente mapear a matriz da fonte do caractere c 
        // e enviá-la para o display via write(i2c_fd, ...)
    }

    oled_send_command(i2c_fd, 0xAE); // Display OFF ao sair
    reset_terminal_mode(&orig_opts);
    close(i2c_fd);
    return 0;
}