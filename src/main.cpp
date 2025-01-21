#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include <driver/spi_master.h>
#include <esp_log.h>

#define TAG "SPI_DMA"

// SPI Configuration
#define SPI_HOST HSPI_HOST // Use VSPI_HOST or HSPI_HOST
#define PIN_NUM_MISO 19    // Master In Slave Out pin
#define PIN_NUM_MOSI 23    // Master Out Slave In pin
#define PIN_NUM_CLK  18    // Clock pin
#define PIN_NUM_CS   5     // Chip Select pin
#define BUFFER_SIZE  50

#define DMA_CHAN 1         // DMA Channel

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "[app_main] Startup..");
    esp_err_t ret;
    
    // SPI Bus Configuration
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1, // -1 means not used
        .quadhd_io_num = -1,
        .max_transfer_sz = BUFFER_SIZE, // Maximum transfer size
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(SPI_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);

    // SPI Device Configuration
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,                 // SPI mode 0
        .duty_cycle_pos = 128,     // Default duty cycle
        .cs_ena_posttrans = 3,     // CS active after transaction
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz clock
        .spics_io_num = PIN_NUM_CS,
        .flags = SPI_DEVICE_NO_DUMMY, // Use no dummy phase
        .queue_size = 7,           // Transaction queue size
    };

    spi_device_handle_t spi;
    ret = spi_bus_add_device(SPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    
    // Example #1: sending numbers
    uint8_t *send_data = (uint8_t *) heap_caps_malloc(BUFFER_SIZE, MALLOC_CAP_DMA); // DMA-capable memory
    for (int i = 0; i < sizeof(send_data); i++) {
        send_data[i] = i % 256; // Fill buffer with data
    }

    // Example #2: sending string
    // DMA_ATTR static uint8_t send_data[] = "I want something"; // define within a static variable

    uint8_t *recv_data = (uint8_t *) heap_caps_malloc(BUFFER_SIZE, MALLOC_CAP_DMA);

    if (!send_data || !recv_data) {
        ESP_LOGE(TAG, "Failed to allocate DMA memory!");
        return;
    }
    memset(recv_data, 2, BUFFER_SIZE); // Clear the receive buffer

    spi_transaction_t transaction = {
        .length = 8 * sizeof(send_data), // Transaction length in bits
        .rxlength = 8 * sizeof(recv_data), // Transaction length in bits
        .tx_buffer = send_data,         // Data to send
        .rx_buffer = recv_data,         // Data to recieve
    };

    // Transmit data using DMA
    ret = spi_device_transmit(spi, &transaction); // Blocking transfer
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI transmit failed: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPI transmit completed");
    }

    ESP_LOGI(TAG, "Received data:");
    for (int i = 0; i < BUFFER_SIZE; i++) { // Print the first 16 bytes
        // printf("recv_data[%d] = 0x%02X\n", i, recv_data[i]);
        printf("recv_data[%d] = %d\n", i, (uint8_t) recv_data[i]);
    }

    // Cleanup
    free(send_data);
    free(recv_data);
    spi_bus_remove_device(spi);
    spi_bus_free(SPI_HOST);
}
