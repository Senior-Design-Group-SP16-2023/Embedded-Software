#include "common.h"
#include "bluetooth.h"
LOG_MODULE_REGISTER(BLUETOOTH_C, LOG_LEVEL_INF);

//advertising parameters
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(
		BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY
	), 		/* Connectable advertising and use identity address */
	800, 	/* Min Advertising Interval 500ms (800*0.625ms) */
	801, 	/* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL	/* Set to NULL for undirected advertising */
); 

//advertising buffer
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};
//scan buffer
static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SENSOR_SERVICE_UUID_VAL),
};

//transmission control variables
static bool notify_enabled = false;
static char _sensor_value[MAX_TRANSMIT_SIZE];

// Connection management functions and callback table
static void on_connected(struct bt_conn *conn, uint8_t err){
	if (err) {
		LOG_ERR("Connection failed (err %u)\n", err);
		return;
	}
	LOG_INF("Connected to device\n");
}
static void on_disconnected(struct bt_conn *conn, uint8_t reason){
	LOG_INF("Disconnected (reason %u)\n", reason);
}

static void on_cccd_change(const struct bt_gatt_attr *attr, uint16_t value){
	LOG_INF("CCCD changed to %i\n", value);
	notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}
struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

// Notifier function
int transmitData(char *sensor_value, size_t len){
	if(len > MAX_TRANSMIT_SIZE){
		return -EINVAL;
	}
	memcpy(_sensor_value, sensor_value, len);
	if (!notify_enabled) {
		LOG_WRN("Notifications not enabled\n");
		return -EACCES;
	}
	LOG_INF("Transmitting...\n");
	return bt_gatt_notify(NULL, &service_handle.attrs[1], &_sensor_value, sizeof(_sensor_value));
}

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
	BT_GATT_CCC(on_cccd_change, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);