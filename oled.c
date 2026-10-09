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
    // Desativa o modo canônico e o eco de caracteres na tela
    new_opts.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}

void reset_terminal_mode(struct termios *orig_opts) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig_opts);
}

int main() {
    struct termios orig_opts;
    int i2c_fd;

    // 1. Abre o barramento I2C
    if ((i2c_fd = open(I2C_DEV_PATH, O_RDWR)) < 0) {
        perror("Erro ao abrir /dev/i2c-1");
        return 1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Erro ao configurar o endereço do OLED");
        close(i2c_fd);
        return 1;
    }

    // 2. Configura o terminal para leitura em tempo real
    set_conio_terminal_mode(&orig_opts);
    printf("Digite qualquer tecla (Pressione ESC para sair):\n");

    char c;
    while (1) {
        c = getchar(); // Lê a tecla imediatamente
        
        if (c == 27) { // Tecla ESC para sair
            break;
        }

        printf("Tecla pressionada: %c\n", c);

        // Exemplo: Envia um comando/dado simulado para o OLED via I2C
        // Aqui você enviaria a matriz do caractere para o SSD1306
        unsigned char data_packet[2] = {0x40, c}; // 0x40 indica envio de dados RAM do SSD1306
        write(i2c_fd, data_packet, sizeof(data_packet));
    }

    // Restaura o terminal ao fechar
    reset_terminal_mode(&orig_opts);
    close(i2c_fd);
    return 0;
}