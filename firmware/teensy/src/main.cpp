#include <Arduino.h>

#include <Wire.h>

#include "acquisition.h"
#include "ina219.h"

void setup() {
	Serial.begin(115200);

	Wire.begin();
	Wire.setClock(400000);

	INA219_init();
	if (INA219_is_ready()) {
		Serial.println("INA219 OK");
	} else {
		Serial.println("INA219 FAIL");
	}
}

void loop() {
	static uint32_t last_print_ms = 0;
	const uint32_t now_ms = millis();

	if ((now_ms - last_print_ms) >= 500U) {
		last_print_ms = now_ms;

		const int16_t voltage_raw = static_cast<int16_t>(analogRead(VOLTAGE_ADC_PIN));
		const int16_t current_raw = INA219_read_current_raw();

		Serial.print("V_RAW=");
		Serial.print(voltage_raw);
		Serial.print(" I_RAW=");
		Serial.println(current_raw);
	}
}
