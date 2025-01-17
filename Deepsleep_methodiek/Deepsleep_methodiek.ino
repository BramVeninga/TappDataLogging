#include <nrf_rtc.h>
#include <nrf_power.h>
#include <bluefruit.h>

#define WAKEUP_INTERVAL_MS 900000

// Persistent variable in retained RAM
__attribute__((section(".noinit"))) bool firstBoot = true;

void configureRTCWakeup(uint32_t wakeup_interval_ms) {
  // Stop RTC1 to configure
  NRF_RTC1->TASKS_STOP = 1;

  // Use the 32.768 kHz clock, prescaler = 0 (no scaling)
  NRF_RTC1->PRESCALER = 0;

  // Convert milliseconds to RTC ticks (32.768 kHz / 1000 * wakeup_interval_ms)
  uint32_t rtc_ticks = wakeup_interval_ms;

  // Configure compare register
  NRF_RTC1->CC[0] = rtc_ticks;

  // Enable compare event and interrupt
  NRF_RTC1->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Msk;

  // Clear event and start RTC1
  NRF_RTC1->EVENTS_COMPARE[0] = 0;
  NRF_RTC1->TASKS_START = 1;

  // Enable RTC1 interrupt in NVIC
  NVIC_EnableIRQ(RTC1_IRQn);  // Enable the interrupt in NVIC


}

void RTC1_IRQHandler(void) {
    if (NRF_RTC1->EVENTS_COMPARE[0]) {
        NRF_RTC1->EVENTS_COMPARE[0] = 0;
        Serial.println("RTC interrupt triggered!");
    }
}


void enterDeepSleep() {
  // Ensure compare event is set as a wake-up source
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;  // Set deep sleep bit

  // Trigger system-off (deep sleep)
  __WFI();  // Wait for interrupt

  Serial.println("Entering deep sleep...");
  Serial.flush();
  delay(1000);
  NRF_POWER->SYSTEMOFF = 1;
  Serial.println("This should never print!");
  Serial.flush();
}

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  Serial.println("Hello, world!");
  delay(1000);

  // Print boot message
  Serial.println("Booting...");

  // Check if it is the first boot
  if (firstBoot) {
    Serial.println("First boot.");
    firstBoot = false;  // Set to false after the initial boot
  } else {
    Serial.println("Woke up from deep sleep.");
  }

  delay(500);

  int nummertje = 0;
  for (int i = 0; i < 1000; i++) {
    nummertje += i;
  }

  Serial.print("Output: ");
  Serial.println(nummertje);
  Serial.flush();

  delay(1000);

  configureRTCWakeup(WAKEUP_INTERVAL_MS);
  enterDeepSleep();
}

void loop() 
{
  Serial.println("Loopin...");
  delay(1000);
}
