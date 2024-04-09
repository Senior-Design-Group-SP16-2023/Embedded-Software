#include "imu.h"
#include "bluetooth.h"
LOG_MODULE_REGISTER(IMU_C, LOG_LEVEL_INF);

//device tree spi spec
const struct spi_dt_spec st_lsm6dso = SPI_DT_SPEC_GET(DT_NODELABEL(st_lsm6dso), SPI_OP, 0);

//data buffers for tx/rx
uint8_t tx_data[2] = {0};
uint8_t rx_data[8] = {0};
struct spi_buf spi_tx_buffer = {tx_data, 2};
struct spi_buf spi_rx_buffer = {rx_data, 8};
const struct spi_buf_set tx_buf = {&spi_tx_buffer, 1};
const struct spi_buf_set rx_buf = {&spi_rx_buffer, 1};

// Bluetooth packet buffer
#define SENSORDATALEN 12
#define GYRO_DATA_OFFSET 0
#define ACCEL_DATA_OFFSET 6
static char sensor_value[SENSORDATALEN] = {0};
int16_t* gyro_data = (int16_t*)(sensor_value + GYRO_DATA_OFFSET);
int16_t* accel_data = (int16_t*)(sensor_value + ACCEL_DATA_OFFSET);

// Helper macro to assemble sensor data out of the SPI buffer 
// per sensor spec for values that are sharded across two registers
#define ASSEMBLE_SENSOR_DATA(spibufset, index) \
	(int16_t)(((uint16_t)((uint8_t *)spibufset.buffers->buf)[index+1]) << 8 | ((uint8_t *)spibufset.buffers->buf)[index]);


int calibrate_sensor(void){
	//TODO: Implement sensor calibration routine for 10 seconds
	return -1;
}

int imu_main(void){
    if(!spi_is_ready_dt(&st_lsm6dso)){
		LOG_ERR("SPI not ready\n");
		return -1;
    }

    // Disable embedded functions and hub sensor
    tx_data[0] = CONFIG(WRITE, FUNC_CFG_ACCESS);
    tx_data[1] = 0x00;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed at line %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

    // Verify Who Am I
    // config_reg[0] = CONFIG(READ, WHO_AM_I);
	tx_data[0] = CONFIG(READ, WHO_AM_I);
	tx_data[1] = 0x00; //wipe out the previous data
    if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &rx_buf)){
		LOG_ERR("Send failed at line %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }
    printf("Chip ID: 0x%x\n", ((uint8_t *)rx_buf.buffers->buf)[1]);
    if(((uint8_t *)rx_buf.buffers->buf)[1] != 0x6C){
		LOG_ERR("Invalid Chip ID, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

    
	// Configure Gyroscope to run at 52Hz
    tx_data[0] = CONFIG(WRITE, CTRL2_G);
    tx_data[1] = GYRO_52_HZ;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

	// Configure Gyro performance mode and high pass filter
    tx_data[0] = CONFIG(WRITE, CTRL7_G);
    tx_data[1] = (
		GYROSCOPE_HIGH_PERFORMANCE_ENABLED | 
		GYROSCOPE_HIGH_PASS_FILTER_DISABLE | 
		GYROSCOPE_HIGH_PASS_FILTER_CUTOFF_1_04HZ
	);
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }
	// Configure Gyro Low pass filter
	// tx_data[0] = CONFIG(WRITE, CTRL6_C);
	// tx_data[1] = 0x00; //todo: replace with value to enable the gyro lowpass filter
	// if(spi_write_dt(&st_lsm6dso, &tx_buf)){
	// 	printf("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
	// 	return -1;
	// }

	// Configure Accelerometer to run at 52Hz
	tx_data[0] = CONFIG(WRITE, CTRL1_XL);
	tx_data[1] = ACCEL_52_HZ;
	if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
	}

    while(1){
		tx_data[0] = CONFIG(READ, SENSOR_GYRO_BASE);
		tx_data[1] = 0x00; //wipe out the previous data
		if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &rx_buf)){
			LOG_ERR("GYRO READ FAILED\n");
		} else {
			gyro_data[0] = ASSEMBLE_SENSOR_DATA(rx_buf, 1);
			gyro_data[1] = ASSEMBLE_SENSOR_DATA(rx_buf, 3);
			gyro_data[2] = ASSEMBLE_SENSOR_DATA(rx_buf, 5);
			LOG_INF("Gyro Data:\nX: %d\nY: %d\nZ: %d\n", gyro_data[0], gyro_data[1], gyro_data[2]);
		}

		tx_data[0] = CONFIG(READ, SENSOR_ACCEL_BASE);
		tx_data[1] = 0x00; //wipe out the previous data
		if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &rx_buf)){
			LOG_ERR("ACCEL READ FAILED\n");
		} else {
			accel_data[0] = ASSEMBLE_SENSOR_DATA(rx_buf, 1);
			accel_data[1] = ASSEMBLE_SENSOR_DATA(rx_buf, 3);
			accel_data[2] = ASSEMBLE_SENSOR_DATA(rx_buf, 5);
			LOG_INF("Accel Data:\nX: %d\nY: %d\nZ: %d\n", accel_data[0], accel_data[1], accel_data[2]);
		}

		transmitData(sensor_value, SENSORDATALEN);
		k_msleep(1000/POLL_FREQ);
	}
	return 0;
}
