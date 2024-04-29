#ifndef IMU_H
#define IMU_H
#include "common.h"
#include <zephyr/drivers/spi.h>


#define SPI_OP  SPI_OP_MODE_MASTER |SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE
#define CONFIG(rw, reg) ((rw << 7)|reg)
#define WRITE                  0
#define READ                   1

// Register Addresses
// Control Registers
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

//Data Registers
#define OUTX_L_G            0x22
#define OUTX_H_G            0x23
#define OUTY_L_G            0x24
#define OUTY_H_G            0x25
#define OUTZ_L_G            0x26
#define OUTZ_H_G            0x27
#define SENSOR_GYRO_BASE    OUTX_L_G

#define OUTX_L_A            0x28
#define OUTX_H_A            0x29
#define OUTY_L_A            0x2A
#define OUTY_H_A            0x2B
#define OUTZ_L_A            0x2C
#define OUTZ_H_A            0x2D
#define SENSOR_ACCEL_BASE   OUTX_L_A


// Sensor configuration macros

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

// Accelerometer high-scale resolution selection - CTRL1_XL
#define LPF2_XL_ENABLE  1
#define LPF2_XL_DISABLE 0 //default

// Gyroscope configs - CTRL2_G
#define GYRO_OFF        (0b0000 << 4)
#define GYRO_12_5_HZ    (0b0001 << 4)
#define GYRO_26_HZ      (0b0010 << 4)
#define GYRO_52_HZ      (0b0011 << 4)
#define GYRO_104_HZ     (0b0100 << 4)
#define GYRO_208_HZ     (0b0101 << 4)
#define GYRO_416_HZ     (0b0110 << 4)
#define GYRO_833_HZ     (0b0111 << 4)
#define GYRO_1660_HZ    (0b1000 << 4)
#define GYRO_3330_HZ    (0x1001 << 4)
#define GYRO_6660_HZ    (0x1010 << 4)

#define GYRO_250_DPS    (0b00 << 2)
#define GYRO_500_DPS    (0b01 << 2)
#define GYRO_1000_DPS   (0b10 << 2)
#define GYRO_2000_DPS   (0b11 << 2)

#define GYRO_FS_DPS     0
#define GYRO_150_DPS    1

#define GYRO_LPF_1_DISABLE  (0 << 1)
#define GYRO_LPF_1_ENABLE   (1 << 1)

// CTRL6_C
#define DEN_NONE        (0b000 << 5)
#define DEN_EDGE        (0b100 << 5)
#define DEN_LEVEL_TRIG  (0b010 << 5)
#define DEN_LEVEL_LATCH (0b011 << 5)
#define DEN_LEVEL_FIFO  (0b110 << 5)
#define XL_HM_ENABLE    (0 << 4)
#define XL_HM_DISABLE   (1 << 4)
#define USR_OFF_W_10    (0 << 3)
#define USR_OFF_W_6     (1 << 3)
#define G_FTYPE_000       0
#define G_FTYPE_001       1
#define G_FTYPE_010       2
#define G_FTYPE_011       3
#define G_FTYPE_100       4
#define G_FTYPE_101       5
#define G_FTYPE_110       6
#define G_FTYPE_111       7

// CTRL7_G
#define G_HM_ENABLE     (0 << 7)
#define G_HM_DISABLE    (1 << 7)
#define G_HP_DISABLE    (0 << 6)
#define G_HP_ENABLE     (1 << 6)
#define G_HP_16_mHZ     (0b00 << 4)
#define G_HP_65_mHZ     (0b01 << 4)
#define G_HP_260_mHZ    (0b10 << 4)
#define G_HP_1_04_HZ    (0b11 << 4)
#define OIS_ON_SPI      (0 << 2)
#define OIS_ON_PRI      (1 << 2)
#define XL_OFS_BYPASS   (0 << 1)
#define XL_OFS_ENABLE   (1 << 1)
#define OIS_DISABLE     0
#define OIS_ENABLE      1

// CTRL8_XL
#define HPCF_XL_000         (0b000 << 5)
#define HPCF_XL_001         (0b001 << 5)
#define HPCF_XL_010         (0b010 << 5)
#define HPCF_XL_011         (0b011 << 5)
#define HPCF_XL_100         (0b100 << 5)
#define HPCF_XL_101         (0b101 << 5)
#define HPCF_XL_110         (0b110 << 5)
#define HPCF_XL_111         (0b111 << 5)
#define XL_HP_REF_DISABLE   (0 << 4)
#define XL_HP_REF_ENABLE    (1 << 4)
#define FAST_SETTLE_DISABLE (0 << 3)
#define FAST_SETTLE_ENABLE  (1 << 3)
#define XL_LPF2             (0 << 2)
#define XL_HP_SLOPE         (1 << 2)
#define XL_FS_MODE_OLD      (0 << 1)
#define XL_FS_MODE_NEW      (1 << 1)
#define LPF2_6D_DISABLE     0
#define LPF2_6D_ENABLE      1

// Main function for Sensor Query thread
int8_t imu_main(void);

// Function to put the IMU in calibration mode
void set_calibration_mode(const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

#endif