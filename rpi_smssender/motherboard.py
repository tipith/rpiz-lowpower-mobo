import time
import struct
import sys
import pigpio
import crcmod
from enum import IntEnum


class StartupReason(IntEnum):
    EXT_TRIGGER1 = 1
    EXT_TRIGGER2 = 2
    LOW_BATTERY = 3
    UNKNOWN = 4

    @classmethod
    def has_value(cls, value):
        return any(value == item.value for item in cls)


class I2CRegister(IntEnum):
    REG_VBATT = 1
    REG_RPI_3V3 = 2
    REG_STARTUP_REASON = 3
    REG_DEBUG = 4


class CommunicationError(Exception):
    pass

class Motherboard:

    def __init__(self, addr=8):
        self.pi = pigpio.pi()
        self.i2c = self.pi.i2c_open(1, addr)
        self.ops = {'total': 0, 'fail': 0}

    def stop(self):
        self.pi.i2c_close(self.i2c)
        self.pi.stop()

    def _read_reg(self, reg: I2CRegister):
        for _ in range(4):
            try:
                self.ops['total'] += 1
                return self.__read_reg(reg)
            except CommunicationError as e:
                self.ops['fail'] += 1
                print(f'error: {e}, ops: {self.ops}')
                time.sleep(1)

    def __read_reg(self, reg: I2CRegister):
        (s, b) = self.pi.i2c_read_i2c_block_data(self.i2c, reg, 4)
        if s >= 0:
            val, expected_crc = struct.unpack('<HH', b)
            crc_func = crcmod.predefined.mkCrcFun('xmodem')
            calculated_crc = crc_func(b[:2])
            if expected_crc != calculated_crc:
                raise CommunicationError('crc mismatch')
            return val
        else:
            raise CommunicationError('unable to communicate')

    @property
    def v_rpi_3v3(self):
        val = self._read_reg(I2CRegister.REG_RPI_3V3)
        return val / 1e3 if val else None

    @property
    def v_batt(self):
        val = self._read_reg(I2CRegister.REG_VBATT)
        return val / 1e3 if val else None

    @property
    def startup_reason(self) -> StartupReason:
        val = self._read_reg(I2CRegister.REG_STARTUP_REASON)
        return StartupReason(val) if StartupReason.has_value(val) else StartupReason.UNKNOWN

    def send_event_to_power_manager(self):
        pass

    def __str__(self):
        return 'Vbatt {} mV, Vrpi {} mV, reason {}'.format(
            self.v_batt, self.v_rpi_3v3, self.startup_reason)