#include "common.h"
#include "bluetooth.h"

int main(){
    // Wait for the bluetooth subsystem to be ready
    bluetooth_init();
    
    // Main loop
    while(1){
        // If the device is connected to a central device
        if(getConnectionStatus()){
            printf("Connected to device\n");
        }
        k_sleep(K_MSEC(1000));
    }
}