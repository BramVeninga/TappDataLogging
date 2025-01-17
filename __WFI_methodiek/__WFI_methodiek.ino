#include <bluefruit.h>

const unsigned long SLEEP_DURATION = 900000; // Sleep in milliseconds

void setup() {
    Serial.begin(115200);
    Bluefruit.autoConnLed(false); // Disable BLE LED for lower power
    Serial.println("Adafruit nRF52 Low-Power Example");
}

void loop() {
    Serial.println("Entering low-power mode...");

    int number = 0;
    for(int i = 0; i < 1000; i++)
    {
      number += i + number;
    }

    Serial.println(number);
    Serial.flush();

    Serial.end();

    // Enter low-power delay
    delay(1000);

    vTaskDelay(pdMS_TO_TICKS(SLEEP_DURATION));

    Serial.begin(115200);
    Serial.println("Woke up from low-power mode!");
    delay(100);
}


void vApplicationIdleHook(void) {
    // Put CPU into WFI to save power
    __WFI();
}
