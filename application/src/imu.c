#include "imu.h"
#include "bluetooth.h"

uint8_t rx_data[2] = {0};
struct spi_buf spi_rx_buffer = {rx_data, 2};
const struct spi_buf_set rx_buf = {&spi_rx_buffer, 1};

uint8_t tx_data[2] = {0};
struct spi_buf spi_tx_buffer = {tx_data, 2};
const struct spi_buf_set tx_buf = {&spi_tx_buffer, 1};

uint8_t sensor_data[7] = {0};
struct spi_buf spi_sensor_buffer = {sensor_data, 7};
const struct spi_buf_set sensor_buf = {&spi_sensor_buffer, 1};

// uint8_t config_reg[1];
// struct spi_buf spi_config_buffer = {config_reg, 1};
// const struct spi_buf_set config_buf = {&spi_config_buffer, 1};

const struct spi_dt_spec st_lsm6dso = SPI_DT_SPEC_GET(DT_NODELABEL(st_lsm6dso), SPI_OP, 0);

#define SENSORDATALEN 6
static char _txpacket[SENSORDATALEN+2]; //padding byte of 0 on both sides
// static char sensor_value[6] = _txpacket+1;
static char sensor_value[SENSORDATALEN] = {0};

//Helper macro to assemble sensor data out of the SPI buffer per sensor spec
#define ASSEMBLE_SENSOR_DATA(spibufset, index) \
	(int16_t)(((uint16_t)((uint8_t *)spibufset.buffers->buf)[index+1]) << 8 | ((uint8_t *)spibufset.buffers->buf)[index]);

int imu_main(void){
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
    // config_reg[0] = CONFIG(READ, WHO_AM_I);
	tx_data[0] = CONFIG(READ, WHO_AM_I);
	tx_data[1] = 0x00; //wipe out the previous data
    if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &rx_buf)){
		printf("Send failed at line %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }
    printf("Chip ID: 0x%x\n", ((uint8_t *)rx_buf.buffers->buf)[1]);
    if(((uint8_t *)rx_buf.buffers->buf)[1] != 0x6C){
		printf("Invalid Chip ID, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

    
	// Configure Gyroscope to run at 52Hz
    tx_data[0] = CONFIG(WRITE, CTRL2_G);
    tx_data[1] = GYRO_52_HZ;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		printf("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
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
		printf("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
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
		printf("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
	}

    tx_data[0] = CONFIG(READ, SENSOR_DATA_BASE);
	tx_data[1] = 0x00; //wipe out the previous data
    while(1){
		if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &sensor_buf)){
			printf("Read failed\n");
		}else{
			//Assemble data into a buffer
			((int16_t*)sensor_value)[0] = ASSEMBLE_SENSOR_DATA(sensor_buf,1);
			((int16_t*)sensor_value)[2] = ASSEMBLE_SENSOR_DATA(sensor_buf,3);
			((int16_t*)sensor_value)[4] = ASSEMBLE_SENSOR_DATA(sensor_buf,5);

			// printf("Gyro Data:\n");
			// printf("X: %d\n",((int16_t*)sensor_value)[0]);
			// printf("Y: %d\n",((int16_t*)sensor_value)[2]);
			// printf("Z: %d\n",((int16_t*)sensor_value)[4]);
			transmitData(sensor_value, SENSORDATALEN);
		}
		k_msleep(1000/SAMPLE_FREQ);
	}
	return 0;
}