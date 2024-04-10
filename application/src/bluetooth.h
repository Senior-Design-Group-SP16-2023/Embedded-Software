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




/* Config */

//UUID VALUES
#define BT_UUID_SENSOR_SERVICE_UUID_VAL     BT_UUID_128_ENCODE(0x7147ac18, 0xc824, 0x438e, 0x8506, 0x60829fbd96a3)
#define BT_UUID_SENSOR_DATA_UUID_VAL               BT_UUID_128_ENCODE(0xbd148149, 0x4469, 0x479a, 0x856f, 0x497ea5e785e5)
#define BT_UUID_SENSOR_CONFIG_UUID_VAL              BT_UUID_128_ENCODE(0x95f61667, 0xffd9, 0x7d9e, 0xfe41, 0x9aed7794ef2f)

//UUIDS
#define BT_UUID_SENSOR_SERVICE  BT_UUID_DECLARE_128(BT_UUID_SENSOR_SERVICE_UUID_VAL)
#define BT_UUID_DATA            BT_UUID_DECLARE_128(BT_UUID_SENSOR_DATA_UUID_VAL)
#define BT_UUID_CONFIG           BT_UUID_DECLARE_128(BT_UUID_SENSOR_CONFIG_UUID_VAL)

//convenience data formatting macros
#define PACKET_SIZE 13
#define GYRO_BUFFER_SIZE PACKET_SIZE
#define ACCEL_BUFFER_SIZE PACKET_SIZE/2
typedef char ble_packet_buffer_t[PACKET_SIZE];
typedef char ble_gyro_buffer_t[GYRO_BUFFER_SIZE];
typedef char ble_accel_buffer_t[ACCEL_BUFFER_SIZE];


/** @brief Send the sensor value as notification. 
 * @param[in] sensor_value The sensor value to transmit. Must be less than MAX_TRANSMIT_SIZE
 * @retval 0 If successful
 */
int8_t transmitData(ble_packet_buffer_t data);



typedef void (*config_write_callback_t)(
    const void *buf, 
    uint16_t len, 
    uint16_t offset, 
    uint8_t flags
);

/**
 * @brief Set the callback function for the config write operation. This function
 * will execute whenever the config field of the BLE service is written to.
 * @param[in] callback The callback function to set
*/
void set_config_write_callback(config_write_callback_t *callback);

int8_t bluetooth_init(void);

#endif
