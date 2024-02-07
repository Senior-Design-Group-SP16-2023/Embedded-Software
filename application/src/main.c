#include "common.h"
#include "bluetooth.h"

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000
#include <dk_buttons_and_leds.h>

int main(){
    // initialize the leds
	int err = dk_leds_init();
	if (err) {
		printf("LEDs init failed (err %d)\n", err);
		return -1;
	}
    int blink_status = 0;

    // Wait for the bluetooth subsystem to be ready
    err = bluetooth_init();
    if (err) {
        printf("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    while(1){
        dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    }
}

//Testing notifier with a simulated sensor thread
uint8_t fake_sensor[TRANSMIT_SIZE] = {0};

#define NOTIFY_INTERVAL 500

static void simulate_data(void) {
	fake_sensor[0]++;
	if (fake_sensor[0] == 200) {
		fake_sensor[0] = 100;
	}
}

void send_data_thread(void) {
	while (1) {
		/* Simulate data */
		simulate_data();
		/* Send notification, the function sends notifications only if a client is subscribed */
		transmitGyroData(fake_sensor);
		k_sleep(K_MSEC(NOTIFY_INTERVAL));
	}
}

/* STEP 18.2 - Define and initialize a thread to send data periodically */
K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
