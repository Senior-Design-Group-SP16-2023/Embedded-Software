#include <zephyr/kernel.h>
#include <stdio.h>
#include "bluetooth.h"
int main(void)
{
    bluetooth_main();
    while(1){
        // printf("Hello World!\n");
        k_sleep(K_MSEC(1000));
    }
}
