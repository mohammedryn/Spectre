#include "ina219.h"

#include <Arduino.h>
#include <Wire.h>

namespace {

constexpr uint8_t INA219_ADDR = 0x40;
constexpr uint8_t INA219_REG_CONFIG = 0x00;
constexpr uint8_t INA219_REG_BUS_VOLTAGE = 0x02;
constexpr uint8_t INA219_REG_CURRENT = 0x04;
constexpr uint16_t INA219_CONFIG_FASTEST_AVG_128 = 0x3FFF;
constexpr uint16_t INA219_READY_BIT = 0x0002;

volatile bool g_ina219_ready = false;

bool write_register16(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(INA219_ADDR);
    Wire.write(reg);
    Wire.write(static_cast<uint8_t>((value >> 8) & 0xFF));
    Wire.write(static_cast<uint8_t>(value & 0xFF));
    return Wire.endTransmission() == 0;
}

bool read_register16(uint8_t reg, uint16_t& out_value) {
    Wire.beginTransmission(INA219_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    const uint8_t bytes_read = Wire.requestFrom(static_cast<int>(INA219_ADDR), 2);
    if (bytes_read != 2) {
        return false;
    }

    const uint8_t msb = Wire.read();
    const uint8_t lsb = Wire.read();
    out_value = static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | lsb);
    return true;
}

}  // namespace

void INA219_init() {
    uint16_t readback = 0;
    const bool write_ok = write_register16(INA219_REG_CONFIG, INA219_CONFIG_FASTEST_AVG_128);
    const bool read_ok = read_register16(INA219_REG_CONFIG, readback);
    g_ina219_ready = write_ok && read_ok && (readback == INA219_CONFIG_FASTEST_AVG_128);
}

int16_t INA219_read_current_raw() {
    uint16_t raw = 0;
    if (!read_register16(INA219_REG_CURRENT, raw)) {
        return 0;
    }
    return static_cast<int16_t>(raw);
}

bool INA219_is_ready() {
    uint16_t bus_voltage = 0;
    if (!g_ina219_ready || !read_register16(INA219_REG_BUS_VOLTAGE, bus_voltage)) {
        return false;
    }
    return (bus_voltage & INA219_READY_BIT) != 0;
}
