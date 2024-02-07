#ifndef BLUETOOTH_H
#define BLUETOOTH_H
#include "common.h"

/* Imports */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gap.h>

/* API */
extern struct k_sem bt_ready_sem4;
int bluetooth_init(void);
bool getConnectionStatus();

/* Config */

//UUID VALUES
#define BT_UUID_SENSOR_SERVICE_UUID_VAL     BT_UUID_128_ENCODE(0x7147ac18, 0xc824, 0x438e, 0x8506, 0x60829fbd96a3)
#define BT_UUID_GYRO_UUID_VAL               BT_UUID_128_ENCODE(0xbd148149, 0x4469, 0x479a, 0x856f, 0x497ea5e785e5)
#define BT_UUID_ACCEL_UUID_VAL              BT_UUID_128_ENCODE(0x95f61667, 0xffd9, 0x7d9e, 0xfe41, 0x9aed7794ef2f)

//UUIDS
#define BT_UUID_SENSOR_SERVICE  BT_UUID_DECLARE_128(BT_UUID_SENSOR_SERVICE_UUID_VAL)
#define BT_UUID_GYRO            BT_UUID_DECLARE_128(BT_UUID_GYRO_UUID_VAL)
#define BT_UUID_ACCEL           BT_UUID_DECLARE_128(BT_UUID_ACCEL_UUID_VAL)

//MAX TRANSMIT SIZE in bytes
#define MAX_TRANSMIT_SIZE 8

/** @brief Send the sensor value as notification.
 * @param[in] sensor_value The gyro value.
 * @retval 0 If successful
 */
int transmitGyroData(uint32_t sensor_value);

#endif
