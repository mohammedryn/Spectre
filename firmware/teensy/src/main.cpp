#include <Arduino.h>

#include <Wire.h>

#include "acquisition.h"
#include "ina219.h"

namespace {

constexpr uint8_t BUFFER_READY_GPIO_PIN = LED_BUILTIN;

}  // namespace

void setup() {
	Serial.begin(115200);
	pinMode(BUFFER_READY_GPIO_PIN, OUTPUT);
	digitalWriteFast(BUFFER_READY_GPIO_PIN, LOW);

	Wire.begin();
	Wire.setClock(400000);

	INA219_init();
	acquisition_init();

	if (INA219_is_ready()) {
		Serial.println("INA219 OK");
	} else {
		Serial.println("INA219 FAIL");
	}
}

void loop() {
	if (INA219_is_ready()) {
		g_latest_current_raw = INA219_read_current_raw();
	}

	bool buffer_ready = false;
	noInterrupts();
	if (g_buffer_ready) {
		g_buffer_ready = false;
		buffer_ready = true;
	}
	interrupts();

	if (buffer_ready) {
		digitalToggleFast(BUFFER_READY_GPIO_PIN);
	}
}
