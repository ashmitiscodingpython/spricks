#include <stdio.h>
#include <stdint.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"

#define WIDTH 128
#define HEIGHT 160

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19

uint16_t framebuffer[WIDTH * HEIGHT];

void init() {
    // SPI Initialization
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // DMA Initialization
    int chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);

    dma_channel_configure(
        chan,
        &c,
        &spi_get_hw(spi0)->dr,
        &framebuffer,
        128 * 160 * 2,
        false
    );
}

