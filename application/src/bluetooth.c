#include "common.h"
#include "bluetooth.h"
#include <stdint.h>
#include <stdio.h>



#pragma region CONFIG

//setup
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
struct k_sem bt_ready_sem4; // Semaphore to signal that the bluetooth subsystem is ready
//connection mgmt
struct k_mutex bt_connection_state_lock; // Mutex to protect the connection status variable
bool connectionStatus = false;

//custom service
bool gyroNotificationsEnabled = false;
bool accelNotificationsEnabled = false;
uint8_t gyro_transmit_buf[MAX_TRANSMIT_SIZE];
uint8_t accel_transmit_buf[MAX_TRANSMIT_SIZE];
struct bt_gatt_attr *gyro_attr;
struct bt_gatt_attr *accel_attr;

#pragma endregion

#pragma region CONNECTION_MGMT
/**
 * @brief Get the connection status
*/
bool getConnectionStatus(){
	k_mutex_lock(&bt_connection_state_lock, K_FOREVER);
	return connectionStatus;
	k_mutex_unlock(&bt_connection_state_lock);
}

/**
 * @brief Set the connection status. Private to this file
*/
static void setConnectionStatus(bool status){
	k_mutex_lock(&bt_connection_state_lock, K_FOREVER);
	connectionStatus = status;
	k_mutex_unlock(&bt_connection_state_lock);
}


/**
 * @brief Callback function that executes when a connection is established
 * @param connection Pointer to the connection object
 * @param err Error code
*/
static void connected_callback(struct bt_conn *connection, uint8_t err){
	//Sets the connection status to true, indicating that a device is connected
	printf("Connected to device, err: %i\n", err);
	setConnectionStatus(true);
	return;
}

/**
 * @brief Callback function that executes when a connection is lost
 * @param connection Pointer to the connection object
 * @param reason Reason for disconnection
*/
static void disconnected_callback(struct bt_conn *connection, uint8_t reason){
	printf("Disconnected from device, reason %i\n", reason);
	setConnectionStatus(false);
	return;
}


/**
 * @brief Callback function table for Bluetooth connection events
*/
struct bt_conn_cb conn_callbacks = {
	.connected = connected_callback,
	.disconnected = disconnected_callback,
	.le_param_req = NULL,
	.le_param_updated = NULL,		
};

#pragma endregion

#pragma region CUSTOM_SERVICE



/**
 * @brief Responsible for sending Gyro data to the connected device
*/
int send_gyro_data(uint64_t gyro_data){
	if(!gyroNotificationsEnabled){
		return -1;
	}
	//copy gyro data into the transmit buffer
	memcpy(gyro_transmit_buf, &gyro_data, sizeof(gyro_data));

	//queue notification into subsystem
	return bt_gatt_notify(
		NULL, 
		gyro_attr, 
		&gyro_data,
		sizeof(gyro_data)
	);

	return 0;
}

/**
 * @brief Responsible for sending Accelerometer data to the connected device
*/
int send_accel_data(uint64_t accel_data){
	if(!accelNotificationsEnabled){
		return -1;
	}

	//copy accel data into the transmit buffer
	memcpy(accel_transmit_buf, &accel_data, sizeof(accel_data));

	//queue notification into subsystem
	return bt_gatt_notify(
		NULL, 
		accel_attr, 
		&accel_data,
		sizeof(accel_data)
	);

	return 0;
}

/**
 * @brief Sets the CCCD value for the Gyro Characteristic
*/
void gyro_cccd_callback(const struct bt_gatt_attr *attr, uint16_t value){
	printf("Gyro CCCD changed to %i\n",value);
	gyroNotificationsEnabled = (value == BT_GATT_CCC_NOTIFY);
}

/**
 * @brief Sets the CCCD value for the Accel Characteristic
*/
void accel_cccd_callback(const struct bt_gatt_attr *attr, uint16_t value){
	printf("Accel CCCD changed to %i\n", value);
	accelNotificationsEnabled = (value == BT_GATT_CCC_NOTIFY);
}

/* Sensor Service Declaration and Registration */
BT_GATT_SERVICE_DEFINE(
	custom_sensor_service,
	BT_GATT_PRIMARY_SERVICE(
		BT_UUID_SENSOR_SERVICE
	),

	/*Gyro characteristic*/
	BT_GATT_CHARACTERISTIC(
		BT_UUID_GYRO,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL, 
		NULL, 
		NULL
	),
	BT_GATT_CCC(
		accel_cccd_callback, //callback for CCC changes
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
	),
	
	/*Accel Characteristic*/
	BT_GATT_CHARACTERISTIC(
		BT_UUID_ACCEL,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL, 
		NULL, 
		NULL
	),
	BT_GATT_CCC(
		gyro_cccd_callback, //callback for CCC changes
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
	),
);

#pragma endregion

#pragma region BLE_SETUP

static const struct bt_data ad[] = 
{
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = 
{
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, SENSOR_SERVICE_UUID_VAL),
};


/**
 * @brief Callback function that executes after bluetooth subsystem has been initialized. 
 * Responsible for starting advertising, and registering connection event callback functions.
 * Responsible for initializing our custom BLE service. 
*/
static void bt_start(int err)
{
	if (err) {
		printf("Bluetooth init failed (err %d)\n", err);
		return;
	}
	
	printf("Bluetooth initialized\n");

	// Register callback table
	bt_conn_cb_register(&conn_callbacks);
	printf("Bluetooth Callbacks registered\n");

	// Start advertising
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		printf("Advertising failed to start (err %d)\n", err);
		return;
	}

	// Diagnostic print
	size_t count = 1;
	bt_addr_le_t addr = {0};
	char addr_s[BT_ADDR_LE_STR_LEN];
	bt_id_get(&addr, &count);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
	printf("BLE started, advertising as %s\n", bt_get_name());

	// Signal that bluetooth subsystem has started
	k_sem_give(&bt_ready_sem4);
}




int bluetooth_init(void){
	// Initialize Bluetooth Handler sync objects
    k_sem_init(&bt_ready_sem4, 0, 1);
    k_mutex_init(&bt_connection_state_lock);
    int err;
	printf("Starting Bluetooth Subsystem...\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_start);
	if (err) {
		printf("Bluetooth enable failed (err %d)\n", err);
		return -1;
	}
	//wait until the bluetooth susbsystem has successfully started, otherwise stall.
	k_sem_take(&bt_ready_sem4, K_FOREVER);

	// Setup attribute pointer handles for transmit functions
	gyro_attr = bt_gatt_find_by_uuid(
		NULL, 
		0,
		BT_UUID_GYRO
	);

	accel_attr = bt_gatt_find_by_uuid(
		NULL, 
		0,
		BT_UUID_ACCEL
	);
	
	printf("Bluetooth Subsystem and Sensor Service ready\n");
	return 0;
};

#pragma endregion


/**
 * TODO:
 * - Implement the gyro and accel notification functions
 * 		- Need to figure out how to find the attribute data pointer through the GATT api rather than hardcoding it
 * - 
*/