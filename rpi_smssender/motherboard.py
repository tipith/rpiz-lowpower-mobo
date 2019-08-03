import time
import struct
import sys
import pigpio
import crcmod
from enum import IntEnum
from typing import *
from threading import Thread, RLock


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
    REG_TEMPERATURE_KELVIN = 4
    REG_DEBUG_DATA_READ = 5


class CommunicationError(Exception):
    pass

class Motherboard(Thread):

    def __init__(self, addr=8):
        super().__init__()
        self.pi = pigpio.pi()
        self.i2c = self.pi.i2c_open(1, addr)
        self.ops = {'total': 0, 'fail': 0}
        self.i2c_lock = RLock()
        self.running = True
        self.remains = ''

    def run(self):
        while self.running:
            try:
                dbg_lines = self.debug
                for line in dbg_lines:
                    print(line)
            except Exception as e:
                print(e)
            time.sleep(0.5)

    def stop(self):
        self.running = False
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
        with self.i2c_lock:
            (s, b) = self.pi.i2c_read_i2c_block_data(self.i2c, reg, 4)
        if s >= 0:
            val, expected_crc = struct.unpack('<HH', b)
            crc_func = crcmod.predefined.mkCrcFun('xmodem')
            calculated_crc = crc_func(b[:2])
            if expected_crc != calculated_crc:
                raise CommunicationError(f'crc mismatch, reg {reg}, expected {hex(expected_crc)} != calculated {hex(calculated_crc)}')
            #print(f'{reg:<20}: {val})
            return val
        else:
            raise CommunicationError(f'unable to communicate, reg {reg}')

    def _write_reg(self, reg: I2CRegister, val):
        with self.i2c_lock:
            self.pi.i2c_write_byte_data(self.i2c, reg, val)

    def _read_buffer(self, reg: I2CRegister, count):
        with self.i2c_lock:
            (s, b) = self.pi.i2c_read_i2c_block_data(self.i2c, reg, count + 2)
        if s >= 0:
            payload = b[:count] 
            crc = b[count:]
            expected_crc = struct.unpack('<H', crc)[0]
            crc_func = crcmod.predefined.mkCrcFun('xmodem')
            calculated_crc = crc_func(payload)
            if expected_crc != calculated_crc:
                raise CommunicationError(f'crc mismatch, reg {reg}, expected {hex(expected_crc)} != calculated {hex(calculated_crc)}')
            #print(f'{reg:<20}: {payload}')
            return payload
        else:
            raise CommunicationError(f'unable to communicate, reg {reg}')
    
    @property
    def v_rpi_3v3(self):
        val = self._read_reg(I2CRegister.REG_RPI_3V3)
        return val / 1e3 if val is not None else None

    @property
    def v_batt(self):
        val = self._read_reg(I2CRegister.REG_VBATT)
        return val / 1e3 if val is not None else None

    @property
    def temperature(self):
        val = self._read_reg(I2CRegister.REG_TEMPERATURE_KELVIN)
        return val - 273 if val is not None else None

    @property
    def startup_reason(self) -> StartupReason:
        val = self._read_reg(I2CRegister.REG_STARTUP_REASON)
        return StartupReason(val) if StartupReason.has_value(val) else StartupReason.UNKNOWN

    @property
    def debug(self) -> List[str]:
        MAX_DBG_PAYLOAD = 30
        ret = ''
        while True:
            
            buf = self._read_buffer(I2CRegister.REG_DEBUG_DATA_READ, MAX_DBG_PAYLOAD)
            if buf is not None and len(buf) > 0:
                ret += buf.decode('ascii', errors='ignore')
            if all(map(bool, buf)):  # retry if payload is filled
                continue
            break
        previous = self.remains
        *full_rows, self.remains = ret.split('\n')
        if len(full_rows):
            full_rows[0] = previous + full_rows[0]  # prepend the found line with previous
        else:
            self.remains = previous + self.remains  # one long line still continues
        return full_rows

    def send_event_to_power_manager(self):
        pass

    def __str__(self):
        return 'Vbatt {} V, Vrpi {} V, temperature {} C, bootup reason {}'.format(
            self.v_batt, self.v_rpi_3v3, self.temperature, self.startup_reason)