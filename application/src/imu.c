#include "imu.h"
#include "bluetooth.h"
LOG_MODULE_REGISTER(IMU_C, LOG_LEVEL_INF);

//device tree spi spec
const struct spi_dt_spec st_lsm6dso = SPI_DT_SPEC_GET(DT_NODELABEL(st_lsm6dso), SPI_OP, 0);

//data buffers for tx/rx
uint8_t tx_data[2] = {0};
uint8_t rx_data[16] = {0};
struct spi_buf spi_tx_buffer = {tx_data, 2};
struct spi_buf spi_rx_buffer = {rx_data, 16};
const struct spi_buf_set tx_buf = {&spi_tx_buffer, 1};
const struct spi_buf_set rx_buf = {&spi_rx_buffer, 1};

// Data buffer for Bluetooth
static ble_packet_buffer_t sensor_value = {0};
#define GYRO_DATA_OFFSET 0 //base of buffer
#define ACCEL_DATA_OFFSET 6 //3 16bit integer offset for gyro
#define TIMESTAMP_OFFSET 12 // 6 16bit integer offset to compensate for gyro and accel
#define SENSOR_VALUE_BASE (sensor_value)
int16_t* gyro_data 		= 		(int16_t*)(SENSOR_VALUE_BASE + GYRO_DATA_OFFSET);
int16_t* accel_data 	= 		(int16_t*)(SENSOR_VALUE_BASE + ACCEL_DATA_OFFSET);
int32_t* timestamp_ptr 	= 		(int32_t*)(SENSOR_VALUE_BASE + TIMESTAMP_OFFSET);

// Config Parameters (Calibration, Timestamp)
static uint8_t calibration_enabled = 0;
static uint8_t calibration_counter = 0;
static int16_t calibration_offsets[6] = {0}; //TODO: Get rid of magic num, 3 for gyro 3 for accel
static int32_t timestamp = 0;

#define GYRO_POLL_FREQ GYRO_104_HZ
#define ACCEL_POLL_FREQ ACCEL_104_HZ
#define SENSOR_TRANSMIT_FREQ 0x64

// Calibration routine for sensor offsets
static void calibrate_sensor(ble_packet_buffer_t data){
	LOG_INF("\nCalibrating: %i\n Old Offsets: [%i,%i,%i,%i,%i,%i]\n New Data: [%i,%i,%i,%i,%i,%i]\n", 
		calibration_counter, calibration_offsets[0], calibration_offsets[1], calibration_offsets[2], 
		calibration_offsets[3], calibration_offsets[4], calibration_offsets[5], 
		((int16_t*)data)[0], ((int16_t*)data)[1], ((int16_t*)data)[2], 
		((int16_t*)data)[3], ((int16_t*)data)[4], ((int16_t*)data)[5]);

	for(uint8_t i = 0; i < 6; i++){
		calibration_offsets[i] = ((calibration_offsets[i]*calibration_counter) + ((int16_t*)data)[i])/(calibration_counter+1);
	}
	//log calibration offsets
	LOG_INF("\n New Calibration Offsets: [%i, %i, %i, %i, %i, %i]\n", 
		calibration_offsets[0], calibration_offsets[1], calibration_offsets[2], 
		calibration_offsets[3], calibration_offsets[4], calibration_offsets[5]);
}

// Callback for Bluetooth
void set_calibration_mode(const void* buf, uint16_t len, uint16_t offset, uint8_t flags){	
	LOG_INF("IMU Config Write Called\n");
	for(uint16_t i = offset; i < len; i++){
		LOG_INF("idx: %i, value: 0x%x ", i, ((uint8_t*)buf)[i]);
	}
	calibration_enabled = ((uint8_t*)buf)[offset];
}

// Helper macro to assemble sensor data out of the SPI buffer 
// per sensor spec for values that are sharded across two registers
#define ASSEMBLE_SENSOR_DATA(spibufset, index) \
	(int16_t)(((uint16_t)((uint8_t *)spibufset.buffers->buf)[index+1]) << 8 | ((uint8_t *)spibufset.buffers->buf)[index]);


static int8_t imu_init(void){
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
    tx_data[1] = GYRO_POLL_FREQ;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

	// Configure Gyro
	// ODR, FS
	tx_data[0] = CONFIG(WRITE, CTRL2_G);
    tx_data[1] = (
		GYRO_104_HZ | 
		GYRO_250_DPS | 
		GYRO_FS_DPS
	);
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

	// Gyro LPF 1
	tx_data[0] = CONFIG(WRITE, CTRL4_C);
    tx_data[1] = GYRO_LPF_1_ENABLE;
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }
	tx_data[0] = CONFIG(WRITE, CTRL6_C);
    tx_data[1] = (
		DEN_NONE |
		XL_HM_ENABLE |
		USR_OFF_W_10 |
		G_FTYPE_100
	);
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

	// High Performance, High Pass, XL_Offset
	tx_data[0] = CONFIG(WRITE, CTRL7_G);
    tx_data[1] = (
		G_HM_ENABLE |
		G_HP_ENABLE |
		G_HP_16_mHZ |
		OIS_ON_SPI |
		XL_OFS_BYPASS |
		OIS_DISABLE
	);
    if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
    }

	// Configure Accelerometer
	// ODR, FS, Resolution
	tx_data[0] = CONFIG(WRITE, CTRL1_XL);
	tx_data[1] = (
		ACCEL_POLL_FREQ |
		ACCEL_2G_SCALE_FS_MODE_1 |
		LPF2_XL_ENABLE
	);
	if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
	}

	// HP/LPF2 settings
	tx_data[0] = CONFIG(WRITE, CTRL8_XL);
	tx_data[1] = (
		HPCF_XL_000 |
		XL_HP_REF_DISABLE |
		FAST_SETTLE_DISABLE |
		XL_LPF2	|
		XL_FS_MODE_OLD |
		LPF2_6D_DISABLE
	);
	if(spi_write_dt(&st_lsm6dso, &tx_buf)){
		LOG_ERR("Send failed, line: %d, file: %s\n", __LINE__, __FILE__);
		return -1;
	}

	return 0;
}

static void rectify_packet(ble_packet_buffer_t data){
	int16_t* dataptr = (int16_t*)data;
	for(uint8_t i = 0; i < 6; i++){
		dataptr[i] -= calibration_offsets[i];
	}
}

int8_t imu_main(void){
	int8_t err = imu_init();
	if(err){
		LOG_ERR("IMU init failed\n");
		return -1;
	}
    while(1){
		tx_data[0] = CONFIG(READ, SENSOR_GYRO_BASE);
		tx_data[1] = 0x00; //wipe out the previous data
		if(spi_transceive_dt(&st_lsm6dso, &tx_buf, &rx_buf)){
			LOG_ERR("SPI READ FAILED\n");
			continue;
		}
		gyro_data[0] 	= ASSEMBLE_SENSOR_DATA(rx_buf, 1); 	 //Gyro X
		gyro_data[1] 	= ASSEMBLE_SENSOR_DATA(rx_buf, 3); 	 //Gyro Y
		gyro_data[2] 	= ASSEMBLE_SENSOR_DATA(rx_buf, 5); 	 //Gyro Z
		accel_data[0] 	= ASSEMBLE_SENSOR_DATA(rx_buf, 7); 	 //Accel X
		accel_data[1] 	= ASSEMBLE_SENSOR_DATA(rx_buf, 9); 	 //Accel Y
		accel_data[2] 	= ASSEMBLE_SENSOR_DATA(rx_buf, 11);  //Accel Z
		*timestamp_ptr = ++timestamp;
		// LOG_INF("Gyro Data:\nX: %d\nY: %d\nZ: %d\n", gyro_data[0], gyro_data[1], gyro_data[2]);
		// LOG_INF("Accel Data:\nX: %d\nY: %d\nZ: %d\n", accel_data[0], accel_data[1], accel_data[2]);
		if(calibration_counter < UINT8_MAX && calibration_enabled){
			if(calibration_counter == 0) memset(calibration_offsets, 0, sizeof(calibration_offsets));
			calibrate_sensor(sensor_value);
			++calibration_counter;
			timestamp = 0;
		} else {
			calibration_enabled = 0;
			calibration_counter = 0;
			rectify_packet(sensor_value);
			int8_t err = transmitData(sensor_value);
			if(!err){
				LOG_INF("Transmitted Gyro Data:\nX: %d\nY: %d\nZ: %d\n", gyro_data[0], gyro_data[1], gyro_data[2]);
				LOG_INF("Transmitted Accel Data:\nX: %d\nY: %d\nZ: %d\n", accel_data[0], accel_data[1], accel_data[2]);
				LOG_INF("Transmitted Timestamp: %d\n", *timestamp_ptr);
			}
		}

		k_msleep(1000/SENSOR_TRANSMIT_FREQ);
	}
	return 0;
}
