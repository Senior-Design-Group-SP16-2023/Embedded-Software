#include "common.h"
#include "bluetooth.h"

#pragma region BLE SETUP AND INITIALIZATION
static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(
		BT_LE_ADV_OPT_CONNECTABLE |
	 	BT_LE_ADV_OPT_USE_IDENTITY
	), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL
); /* Set to NULL for undirected advertising */

LOG_MODULE_REGISTER(Bluetooth_c, LOG_LEVEL_INF);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define CON_STATUS_LED DK_LED2
#define USER_LED DK_LED3

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SENSOR_SERVICE_UUID_VAL),
};

// Connection management functions and callback table
static void on_connected(struct bt_conn *conn, uint8_t err){
	if (err) {
		printf("Connection failed (err %u)\n", err);
		return;
	}

	printf("Connected to device\n");

	dk_set_led_on(CON_STATUS_LED);
}
static void on_disconnected(struct bt_conn *conn, uint8_t reason){
	printf("Disconnected (reason %u)\n", reason);

	dk_set_led_off(CON_STATUS_LED);
}
struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

// Initialize the Bluetooth subsystem
int bluetooth_init(void) {
	//synchronous enable
	int	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");

	//register the bt connection callbacks
	bt_conn_cb_register(&connection_callbacks);

	LOG_INF("Bluetooth initialized\n");
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}

	LOG_INF("Advertising successfully started\n");
	return 0;
}
#pragma endregion

#pragma region CUSTOM BLE SERVICE
static bool notify_enabled;

// Config change callback for sensor characteristic
static void cccd_change_callback(const struct bt_gatt_attr *attr, uint16_t value){
	printf("CCCD changed to %i\n", value);
	notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}
// Service Declaration 
BT_GATT_SERVICE_DEFINE(
	service_handle, BT_GATT_PRIMARY_SERVICE(BT_UUID_SENSOR_SERVICE),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_GYRO, 
		BT_GATT_CHRC_NOTIFY, 
		BT_GATT_PERM_NONE, 
		NULL,
		NULL, 
		NULL
	),
	BT_GATT_CCC(cccd_change_callback, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);


// Notifier function
static char _sensor_value[MAX_TRANSMIT_SIZE];

int transmitData(char *sensor_value, size_t len){
	if(len > MAX_TRANSMIT_SIZE){
		return -EINVAL;
	}
	
	memcpy(_sensor_value, sensor_value, len);
	// printf("notify_enabled: %i\n", notify_enabled);
	if (!notify_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &service_handle.attrs[1], &_sensor_value, sizeof(_sensor_value));
}

#pragma endregion