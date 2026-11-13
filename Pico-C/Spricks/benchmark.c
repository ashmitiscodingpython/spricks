#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include <string.h>
#include <inttypes.h>

#define WIDTH 128
#define HEIGHT 160

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_DC   22
#define PIN_LITE 17

#define SPEED 24

const uint8_t rotations[4] = {0x00, 0x60, 0xC0, 0xA0};
#define ROTATION 0

uint16_t framebuffer[WIDTH * HEIGHT];

void write_data(const uint8_t *data, size_t length) {
    gpio_put(PIN_DC, 1);
    spi_write_blocking(SPI_PORT, data, length);
} 

void writecommand(uint8_t command) {
    gpio_put(PIN_DC, 0);
    spi_write_blocking(SPI_PORT, &command, 1);
}

void writeparamcommand(uint8_t command, uint8_t *param, size_t length) {
    writecommand(command);
    gpio_put(PIN_DC, 1);
    write_data(param, length);
    gpio_put(PIN_DC, 0);
}

void update_display() {
    uint8_t column[] = {0x00, 0x00, 0x00, 0x7F};
    uint8_t row[] = {0x00, 0x00, 0x00, 0x9F};
    writeparamcommand(0x2A, column, 4);
    writeparamcommand(0x2B, row, 4);
    writecommand(0x2C); // sending framebuffer
    gpio_put(PIN_DC, 1);
    write_data((const uint8_t *) framebuffer, sizeof(framebuffer));
}

void clear() {
    memset(framebuffer, 0x00, sizeof(framebuffer));
}

void pixel(int x, int y, uint16_t color) {
    framebuffer[x + (y * WIDTH)] = color;
}

void draw_line_h(int x0, int y0, int x1, int y1) {
    if (x0 > x1) {
        int temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int dir = (dx < 0) ? -1 : 1;
    dy *= dir;
    if (dx != 0) {
        int y = y0;
        int p = 2*dy - dx;
        for (int i = 0; i < (dx + 1); i++) {
            pixel(x0 + i, y, 0xFFFF);
            if (p >= 0) {
                y += dir;
                p = p - 2*dx;
            }
            p = p + 2*dy;
        }
    }
}

void draw_line_v(int x0, int y0, int x1, int y1) {
    if (y0 > y1) {
        int temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int dir = (dy < 0) ? -1 : 1;
    dy *= dir;
    if (dy != 0) {
        int x = x0;
        int p = 2*dx - dy;
        for (int i = 0; i < (dy + 1); i++) {
            pixel(x, y0 + i, 0xFFFF);
            if (p >= 0) {
                x += dir;
                p = p - 2*dy;
            }
            p = p + 2*dx;
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1) {
    if (abs(x1 - x0) > abs(y1 - y0)) {
        draw_line_h(x0, y0, x1, y1);
    } else {
        draw_line_v(x0, y0, x1, y1);
    }
}

void fill_random_lines() {
    clear();
    for (int i = 0; i < 101; i++) {
        int x0 = rand() % 128;
        int y0 = rand() % 160;
        int x1 = rand() % 128;
        int y1 = rand() % 160;
        draw_line(x0, y0, x1, y1);
    }
}

void init_display () {
    spi_init(SPI_PORT, 1000*1000*SPEED);
    spi_set_format(
        SPI_PORT,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 0);

    gpio_init(26);
    gpio_set_dir(26, GPIO_OUT);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);

    gpio_put(26, 0);  // turn on reset
    sleep_ms(10);

    gpio_put(26, 1);  // let reset go
    sleep_ms(120);

    gpio_put(PIN_DC, 0);
    writecommand(0x01); // SWRST
    sleep_us(150);
    writecommand(0x11); // Sleep Out
    sleep_ms(120);

    uint8_t data3[] = {0x01, 0x2C, 0x2D};
    writecommand(0xB1);
    write_data(data3, sizeof(data3));

    writecommand(0xB2); // Frame rate control.
    write_data(data3, sizeof(data3));

    uint8_t data6[] = {0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d};
    writecommand(0xB3); // Frame rate control.
    write_data(data6, sizeof(data6));
    sleep_us(10);

    writecommand(0xB4); // Display inversion control
    uint8_t data1[] = {0x07};
    write_data(data1, sizeof(data1));
    writecommand(0xC0); // Power control
    uint8_t data4[] = {0xA2, 0x02, 0x84};
    write_data(data4, sizeof(data4));

    writecommand(0xC1); // Power control
    write_data((uint8_t[]){0xC5}, 1);

    uint8_t data2[2] = {0x0A, 0x00};
    writecommand(0xC2); // Power control
    write_data(data2, sizeof(data2));

    writecommand(0xC3); // Power control
    data2[0] = 0x8A;
    data2[1] = 0x2A;
    write_data(data2, sizeof(data2));

    writecommand(0xC4); // Power control
    data2[0] = 0x8A;
    data2[1] = 0xEE;
    write_data(data2, sizeof(data2));

    writecommand(0xC5); // Power control
    write_data((uint8_t[]){0x0E}, 1);

    writecommand(0x20); // INVOFF

    writecommand(0x36); // MADCTL
    write_data(&rotations[ROTATION], 1);

    writecommand(0x3A);  // COLMOD
    uint8_t format = 0x05;
    write_data(&format, 1);

    writecommand(0x2A); // CASET (Column Address)
    write_data((uint8_t[]){0x00, 0x01, 0x00, 0x7F}, 4);

    writecommand(0x2B); // RASET (Row Address)
    write_data((uint8_t[]){0x00, 0x01, 0x00, 0x9F}, 4);

    uint8_t pg[] = {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2b, 0x39, 0x00, 0x01, 0x03, 0x10};
    writecommand(0xE0); // Positive Gamma Correction (GMCTRP)
    write_data(pg, sizeof(pg));

    uint8_t ng[] = {0x03, 0x1d, 0x07, 0x06, 0x2e, 0x2c, 0x29, 0x2d, 0x2e, 0x2e, 0x37, 0x3f, 0x00, 0x00, 0x02, 0x10};
    writecommand(0xE1); // Positive Gamma Correction (GMCTRN)
    write_data(ng, sizeof(ng));

    gpio_init(PIN_LITE);
    gpio_set_dir(PIN_LITE, GPIO_OUT);
    gpio_put(PIN_LITE, 1);
    sleep_ms(1500);
    clear();

    writecommand(0x13); // NORON (normal display on)
    writecommand(0x29); // display on
}

int main(void) {
    stdio_init_all();

    init_display();

    gpio_init(28);
    gpio_set_dir(28, GPIO_OUT);
    gpio_put(28, 1);

    uint32_t sum = 0;
    for (int i = 0; i < 100; i++) {
        fill_random_lines();
        uint32_t start = time_us_32();
        update_display();
        while (spi_is_busy(SPI_PORT)) {}
        uint32_t end = time_us_32();
        uint32_t elapsed = end - start;
        printf("Time: %" PRIu32 " us\n", elapsed);
        sum += elapsed;
    }
    double average = (double) sum / 100;
    printf("Average time: %.2f us\n", average);
    printf("Average FPS: %.2f", (1000000.0 / average));

    gpio_put(28, 0);
}