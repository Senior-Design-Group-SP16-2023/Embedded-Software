#ifndef IMU_H
#define IMU_H
#include "common.h"
#include <zephyr/drivers/spi.h>


#define SPI_OP  SPI_OP_MODE_MASTER |SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE
#define CONFIG(rw, reg) ((rw << 7)|reg)
#define WRITE 0
#define READ 1
#define FUNC_CFG_ACCESS 0x01
#define PIN_CTRL 0x02
#define FIFO_CTRL1 0x07
#define FIFO_CTRL2 0x08
#define FIFO_CTRL3 0x09
#define FIFO_CTRL4 0x0A
#define COUNTER_BDR_REG1 0x0B
#define COUNTER_BDR_REG2 0x0C
#define INT1_CTRL 0x0D
#define INT2_CTRL 0x0E
#define WHO_AM_I 0x0F
#define CTRL1_XL 0x10
#define CTRL2_G 0x11
#define CTRL3_C 0x12
#define CTRL4_C 0x13
#define CTRL5_C 0x14
#define CTRL6_C 0x15
#define CTRL7_G 0x16
#define CTRL8_XL 0x17
#define CTRL9_XL 0x18
#define CTRL10_C 0x19

int imu_main(void);

#endif