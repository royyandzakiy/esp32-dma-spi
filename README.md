# DMA SPI Test on ESP32

## **What This Repo Does**
This repository demonstrates how to use Direct Memory Access (DMA) with SPI on the ESP32. It includes examples of:
- Allocating DMA-capable memory.
- Configuring SPI with and without DMA.
- Tweaking buffer sizes and transfer configurations.

## **How to Use This**
1. Clone this repository.
2. Open the project in your ESP-IDF development environment.
3. Modify the `app_main()` function in `main.c` for specific use cases.
4. Build and flash the project:
   ```bash
   idf.py build flash monitor
   ```

## **How It Works**
1. **DMA Initialization:**
   - Allocates DMA-capable memory using `heap_caps_malloc` with `MALLOC_CAP_DMA`.
2. **SPI Configuration:**
   - Configures SPI with `spi_bus_initialize` and assigns a DMA channel.
   - Uses `spi_device_transmit` for data transfers.
3. **Cleanup:**
   - Frees memory and releases the SPI bus after use.

### Example Code Snippet:
```c
uint8_t *dma_buffer = (uint8_t *)heap_caps_malloc(4096, MALLOC_CAP_DMA);
memset(dma_buffer, 0xAA, 4096);

spi_bus_config_t bus_config = {
    .sclk_io_num = 14,  // Clock pin
    .mosi_io_num = 13,  // MOSI pin
    .miso_io_num = -1,  // Not used
    .max_transfer_sz = 4096
};
spi_bus_initialize(SPI2_HOST, &bus_config, 1);

spi_transaction_t trans = {
    .length = 4096 * 8, // Bits
    .tx_buffer = dma_buffer
};
spi_device_transmit(spi_handle, &trans);
free(dma_buffer);
```

## **How DMA Works**
DMA allows peripherals like SPI to directly read/write to memory without CPU involvement, reducing overhead and improving performance. By allocating memory with `MALLOC_CAP_DMA`, you ensure the buffer is accessible by the DMA hardware.

### Key Points:
- No explicit `dma_store_data` or `dma_access_data` functions are needed.
- The DMA controller manages the data transfer transparently.

## **Additional Use Cases**
### SPI Without DMA:
```c
spi_bus_initialize(SPI2_HOST, &bus_config, 0); // DMA channel = 0
```
Transfers are CPU-managed and slower for large data sizes.

### Variations with I2C and UART:
- **I2C DMA:** Suitable for large data transfers with DMA-capable buffers.
- **UART DMA:** Ideal for high-speed data streaming.