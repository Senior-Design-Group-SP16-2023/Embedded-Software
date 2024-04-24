#include "common.h"
#include "bluetooth.h"
#include "imu.h"

LOG_MODULE_REGISTER(MAIN_C, LOG_LEVEL_INF);

int main(){
    // initialize 
    LOG_INF("initializing\n");

    // Wait for the bluetooth subsystem to be ready
    uint8_t err = bluetooth_init();
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    imu_main();
}


/*
TODO:
- Add timestamping to setup + packet transmission
- Add calibration SPI Calls
- Calibration:
    - Write 1 to start calibration
    - Write 0 to stop calibration
*/