#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEV_PATH "/dev/i2c-1"
#define OLED_ADDR    0x3C

void send_cmd(int fd, unsigned char cmd) {
    unsigned char buf[2] = {0x00, cmd};
    write(fd, buf, 2);
    usleep(200);
}

int main() {
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

    printf("Enviando comandos de inicializacao direta...\n");

    // Sequência essencial de ativação
    send_cmd(fd, 0xAE); // Display OFF
    send_cmd(fd, 0xD5); send_cmd(fd, 0x80); // Clock div
    send_cmd(fd, 0xA8); send_cmd(fd, 0x3F); // Multiplex ratio (64)
    send_cmd(fd, 0xD3); send_cmd(fd, 0x00); // Display offset
    send_cmd(fd, 0x40); // Start line
    send_cmd(fd, 0x8D); send_cmd(fd, 0x14); // Charge pump ON
    send_cmd(fd, 0x20); send_cmd(fd, 0x00); // Horizontal addressing mode
    send_cmd(fd, 0xA1); // Segment remap
    send_cmd(fd, 0xC8); // COM output scan direction
    send_cmd(fd, 0xDA); send_cmd(fd, 0x12); // COM pins
    send_cmd(fd, 0x81); send_cmd(fd, 0xCF); // Contrast
    send_cmd(fd, 0xD9); send_cmd(fd, 0xF1); // Pre-charge
    send_cmd(fd, 0xDB); send_cmd(fd, 0x40); // VCOMH
    send_cmd(fd, 0xA4); // Resume to RAM content
    send_cmd(fd, 0xA6); // Normal display (não invertido)
    send_cmd(fd, 0xAF); // Display ON

    // Configura o ponteiro para cobrir a tela inteira (Colunas 0-127, Páginas 0-7)
    send_cmd(fd, 0x21); send_cmd(fd, 0x00); send_cmd(fd, 0x7F);
    send_cmd(fd, 0x22); send_cmd(fd, 0x00); send_cmd(fd, 0x07);

    // Envia dados preenchendo a tela com pixels acesos (0xFF) para testar se acende
    unsigned char block[129];
    block[0] = 0x40; // Indicador de dados
    for (int i = 1; i < 129; i++) {
        block[i] = 0xFF; // Todos os pixels da coluna acesos
    }

    for (int p = 0; p < 8; p++) {
        write(fd, block, 129);
        usleep(500);
    }

    printf("Dados enviados. O display deve estar aceso.\n");
    close(fd);
    return 0;
}