#ifndef IMU_H
#define IMU_H
#include "common.h"
#include <zephyr/drivers/spi.h>


#define SPI_OP  SPI_OP_MODE_MASTER |SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE
#define CONFIG(rw, reg) ((rw << 7)|reg)
#define WRITE                  0
#define READ                   1
#define FUNC_CFG_ACCESS     0x01
#define PIN_CTRL            0x02
#define FIFO_CTRL1          0x07
#define FIFO_CTRL2          0x08
#define FIFO_CTRL3          0x09
#define FIFO_CTRL4          0x0A
#define COUNTER_BDR_REG1    0x0B
#define COUNTER_BDR_REG2    0x0C
#define INT1_CTRL           0x0D
#define INT2_CTRL           0x0E
#define WHO_AM_I            0x0F
#define CTRL1_XL            0x10
#define CTRL2_G             0x11
#define CTRL3_C             0x12
#define CTRL4_C             0x13
#define CTRL5_C             0x14
#define CTRL6_C             0x15
#define CTRL7_G             0x16
#define CTRL8_XL            0x17
#define CTRL9_XL            0x18
#define CTRL10_C            0x19
#define SAMPLE_FREQ         0x0A 

// Sesonr configuration macros

// Gyroscope configs - CTRL7_G 
#define GYROSCOPE_HIGH_PERFORMANCE_DISABLED (1 << 7)
#define GYROSCOPE_HIGH_PERFORMANCE_ENABLED  (0 << 7)
//highpass filter. Note: only works if the gyroscope is in high performance mode
#define GYROSCOPE_HIGH_PASS_FILTER_ENABLE           (1 << 6) 
#define GYROSCOPE_HIGH_PASS_FILTER_DISABLE          (0 << 6)
#define GYROSCOPE_HIGH_PASS_FILTER_CUTOFF_16MHZ     (0b00 << 4)
#define GYROSCOPE_HIGH_PASS_FILTER_CUTOFF_65MHZ     (0b01 << 4)
#define GYROSCOPE_HIGH_PASS_FILTER_CUTOFF_260MHZ    (0b10 << 4)
#define GYROSCOPE_HIGH_PASS_FILTER_CUTOFF_1_04HZ    (0b11 << 4)

// Gyroscope configs - CTRL2_G
#define GYRO_OFF        (0x0 << 4)
#define GYRO_12_5_HZ    (0x1 << 4)
#define GYRO_26_HZ      (0x2 << 4)
#define GYRO_52_HZ      (0x3 << 4)
#define GYRO_104_HZ     (0x4 << 4)
#define GYRO_208_HZ     (0x5 << 4)
#define GYRO_416_HZ     (0x6 << 4)
#define GYRO_833_HZ     (0x7 << 4)
#define GYRO_1660_HZ    (0x8 << 4)
#define GYRO_3330_HZ    (0x9 << 4)
#define GYRO_6660_HZ    (0xA << 4)

// Accelerometer ODR configuration - CTRL1_XL
#define ACCEL_OFF       (0x0 << 4)
#define ACCEL_1_6_HZ    (0xb << 4)
#define ACCEL_12_5_HZ   (0x1 << 4)
#define ACCEL_26_HZ     (0x2 << 4)
#define ACCEL_52_HZ     (0x3 << 4)
#define ACCEL_104_HZ    (0x4 << 4)
#define ACCEL_208_HZ    (0x5 << 4)
#define ACCEL_416_HZ    (0x6 << 4)
#define ACCEL_833_HZ    (0x7 << 4)
#define ACCEL_1660_HZ   (0x8 << 4)
#define ACCEL_3330_HZ   (0x9 << 4)
#define ACCEL_6660_HZ   (0xA << 4)

// Accelerometer Scale configuration - CTRL1_XL
#define ACCEL_2G_SCALE_FS_MODE_0  (0b00 << 2) //default
#define ACCEL_2G_SCALE_FS_MODE_1  (0b00 << 2)
#define ACCEL_4G_SCALE_FS_MODE_0  (0b10 << 2)
#define ACCEL_4G_SCALE_FS_MODE_1  (0b10 << 2)
#define ACCEL_8G_SCALE_FS_MODE_0  (0b11 << 2)
#define ACCEL_8G_SCALE_FS_MODE_1  (0b11 << 2)
#define ACCEL_16G_SCALE_FS_MODE_0 (0b01 << 2)

// Accelerometr high-scale resolution selection - CTRL2_XL
#define LPF2_XL_ENABLE  1
#define LPF2_XL_DISABLE 0 //default

int imu_main(void);

#endif