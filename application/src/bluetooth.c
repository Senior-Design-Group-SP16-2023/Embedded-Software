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
static ble_packet_buffer_t _sensor_value;

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
struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

// Data characteristic callbacks
static void on_data_cccd_change(const struct bt_gatt_attr *attr, uint16_t value){
	LOG_INF("CCCD changed to %i\n", value);
	notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

// Config characteristic callbacks

static config_write_callback_t* config_write_cb = NULL;
void set_config_write_callback(config_write_callback_t *callback){
	LOG_INF("Setting write callback: %p\n", callback);
	config_write_cb = callback;
}

static ssize_t on_config_write(
	struct bt_conn *conn, 
	const struct bt_gatt_attr *attr, 
	const void *buf, 
	uint16_t len, 
	uint16_t offset, 
	uint8_t flags
	)
{
	LOG_INF("Config write\n");
	if(config_write_cb == NULL){
		LOG_WRN("No config write callback function set\n");
		return -1;
	}
	LOG_INF("Calling Config write callback\n");
	(*config_write_cb)(buf, len, offset, flags);
	return 0;
}

// Service Declaration 
BT_GATT_SERVICE_DEFINE(
	service_handle, BT_GATT_PRIMARY_SERVICE(BT_UUID_SENSOR_SERVICE),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DATA, 
		BT_GATT_CHRC_NOTIFY, 
		BT_GATT_PERM_NONE, 
		NULL,
		NULL, 
		NULL
	),
	BT_GATT_CCC(on_data_cccd_change, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_CONFIG, 
		BT_GATT_CHRC_WRITE, 
		BT_GATT_PERM_WRITE, 
		NULL,
		on_config_write, 
		NULL
	),
);

// Notifier function for sensor data
int8_t transmitData(ble_packet_buffer_t data){
	memcpy(_sensor_value, data, sizeof(ble_packet_buffer_t));
	if (!notify_enabled) {
		LOG_WRN("Notifications not enabled\n");
		return -EACCES;
	}
	LOG_INF("Transmitting...\n");
	return bt_gatt_notify(NULL, &service_handle.attrs[1], &_sensor_value, sizeof(_sensor_value));
}

// Initialize the Bluetooth subsystem
int8_t bluetooth_init(void) {
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

