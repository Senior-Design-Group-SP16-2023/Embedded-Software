#include "imu.h"
#include "bluetooth.h"

uint8_t rx_data[2];
struct spi_buf spi_rx_buffer = {rx_data, 2};
const struct spi_buf_set rx_buf = {&spi_rx_buffer, 1};

uint8_t sensor_data[7];
struct spi_buf spi_sensor_buffer = {sensor_data, 7};
const struct spi_buf_set sensor_buf = {&spi_sensor_buffer, 1};

uint8_t config_reg[1];
struct spi_buf spi_config_buffer = {config_reg, 1};
const struct spi_buf_set config_buf = {&spi_config_buffer, 1};

uint8_t tx_data[2];
struct spi_buf spi_tx_buffer = {tx_data, 2};
const struct spi_buf_set tx_buf = {&spi_tx_buffer, 1};

const struct spi_dt_spec st_lsm6dso = SPI_DT_SPEC_GET(DT_NODELABEL(st_lsm6dso), SPI_OP, 0);

static char sensor_value[8];

int imu_main(void* arg){
    if(!spi_is_ready_dt(&st_lsm6dso)){
		printf("SPI not ready\n");
		return -1;
    }

    // Disable embedded functions and hub sensor
    tx_data[0] = CONFIG(WRITE, FUNC_CFG_ACCESS);
    tx_data[1] = 0x00;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		printf("Send failed at line %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

    // Verify Who Am I
    config_reg[0] = CONFIG(READ, WHO_AM_I);
    if(spi_transceive_dt(&st_lsm6dso, &config_buf, &rx_buf)){
		printf("Send failed at line %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }
    printk("Chip ID: 0x%x\n", ((uint8_t *)rx_buf.buffers->buf)[1]);
    if(((uint8_t *)rx_buf.buffers->buf)[1] != 0x6C){
		printk("Invalid Chip ID, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

    // Configure Gyroscope
    tx_data[0] = CONFIG(WRITE, CTRL2_G);
    tx_data[1] = 0x30;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		printk("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }
    tx_data[0] = CONFIG(WRITE, CTRL7_G);
    tx_data[1] = 0x70;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		printk("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

    tx_data[0] = CONFIG(READ, 0x22);
    while(1){
		if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &sensor_buf)){
			printk("Read failed\n");
		}else{
			// printk("Gyro Data:\n");
			// printk("X: %d\n",(int16_t)(((uint16_t)((uint8_t *)sensor_buf.buffers->buf)[2]) << 8 | ((uint8_t *)sensor_buf.buffers->buf)[1]));
			// printk("Y: %d\n",(int16_t)(((uint16_t)((uint8_t *)sensor_buf.buffers->buf)[4]) << 8 | ((uint8_t *)sensor_buf.buffers->buf)[3]));
			// printk("Z: %d\n",(int16_t)(((uint16_t)((uint8_t *)sensor_buf.buffers->buf)[6]) << 8 | ((uint8_t *)sensor_buf.buffers->buf)[5]));

			//Assemble data into a buffer
			((int16_t*)sensor_value)[0] = (int16_t)(((uint16_t)((uint8_t *)sensor_buf.buffers->buf)[2]) << 8 | ((uint8_t *)sensor_buf.buffers->buf)[1]);
			((int16_t*)sensor_value)[1] = (int16_t)(((uint16_t)((uint8_t *)sensor_buf.buffers->buf)[4]) << 8 | ((uint8_t *)sensor_buf.buffers->buf)[3]);
			((int16_t*)sensor_value)[2] = (int16_t)(((uint16_t)((uint8_t *)sensor_buf.buffers->buf)[6]) << 8 | ((uint8_t *)sensor_buf.buffers->buf)[5]);

			printk("Gyro Data:\n");
			printk("X: %d\n",((int16_t*)sensor_value)[0]);
			printk("Y: %d\n",((int16_t*)sensor_value)[1]);
			printk("Z: %d\n",((int16_t*)sensor_value)[2]);
			transmitData(sensor_value);
		}
		k_msleep(100);
	}

    return 0;
}
