import time
import struct
import sys
import pigpio
import crcmod
import logging
from enum import IntEnum
from typing import *
from threading import Thread, RLock
from pins import *


logger = logging.getLogger('mobo')

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
    REG_REQUEST_SHUTDOWN = 6

class SleepingException(Exception):
    pass

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
        self.pi.set_mode(PIN_SLEEP_STATUS, pigpio.INPUT)

    def run(self):
        while self.running:
            try:
                dbg_lines = self.debug
                for line in dbg_lines:
                    logger.info(line)
            except SleepingException:
                pass
            except CommunicationError as e:
                logger.warn(e)
            time.sleep(0.01)

    def stop(self):
        self.running = False
        self.join(1)
        self.pi.i2c_close(self.i2c)
        self.pi.stop()

    def _read_reg_word(self, reg: I2CRegister):
        for _ in range(4):
            try:
                payload = self._read_buffer(reg, 2)
                return struct.unpack('<H', payload)[0]
            except CommunicationError as e:
                logger.warn(f'error: {e}, ops: {self.ops}')
                time.sleep(1)
            except SleepingException:
                break

    def _write_buffer(self, reg: I2CRegister, data):
        if self.sleeping:
            raise SleepingException(f'power manager is sleeping')
        with self.i2c_lock:
            crc_func = crcmod.predefined.mkCrcFun('xmodem')
            data += struct.pack('<H', crc_func(data))
            logger.debug(f'{str(reg):<20}: {data}')
            self.pi.i2c_write_i2c_block_data(self.i2c, reg, data)

    def _read_buffer(self, reg: I2CRegister, count):
        if self.sleeping:
            raise SleepingException(f'power manager is sleeping')
        with self.i2c_lock:
            self.ops['total'] += 1
            (s, b) = self.pi.i2c_read_i2c_block_data(self.i2c, reg, count + 2)
        if s >= 0:
            payload = b[:count] 
            crc = b[count:]
            expected_crc = struct.unpack('<H', crc)[0]
            crc_func = crcmod.predefined.mkCrcFun('xmodem')
            calculated_crc = crc_func(payload)
            if expected_crc != calculated_crc:
                self.ops['fail'] += 1
                raise CommunicationError(f'crc mismatch, reg {reg}, expected {hex(expected_crc)} != calculated {hex(calculated_crc)}\n - data: {b}')
            logger.debug(f'{str(reg):<20}: {payload}')
            return payload
        else:
            self.ops['fail'] += 1
            raise CommunicationError(f'unable to communicate, reg {reg}, status {s}')

    @property
    def sleeping(self):
        return self.pi.read(PIN_SLEEP_STATUS) == 1

    @property
    def v_rpi_3v3(self):
        val = self._read_reg_word(I2CRegister.REG_RPI_3V3)
        return val / 1e3 if val is not None else None

    @property
    def v_batt(self):
        val = self._read_reg_word(I2CRegister.REG_VBATT)
        return val / 1e3 if val is not None else None

    @property
    def temperature(self):
        val = self._read_reg_word(I2CRegister.REG_TEMPERATURE_KELVIN)
        return val - 273 if val is not None else None

    @property
    def startup_reason(self) -> StartupReason:
        val = self._read_reg_word(I2CRegister.REG_STARTUP_REASON)
        return StartupReason(val) if StartupReason.has_value(val) else StartupReason.UNKNOWN

    @property
    def debug(self) -> List[str]:
        MAX_DBG_PAYLOAD = 30
        ret = ''
        while True:
            buf = self._read_buffer(I2CRegister.REG_DEBUG_DATA_READ, MAX_DBG_PAYLOAD)
            if buf is not None and len(buf) > 0:
                ret += buf.decode('ascii', errors='ignore')
            if all(map(bool, ret)) and len(ret) < 128:  # retry if payload is filled
                time.sleep(0.01)
                continue
            break
        previous = self.remains
        *full_rows, self.remains = ret.split('\n')
        if len(full_rows):
            full_rows[0] = previous + full_rows[0]  # prepend the found line with previous
        else:
            self.remains = previous + self.remains  # one long line still continues
        return full_rows

    def request_shutdown(self, secs):
        logger.info(f'request shutdown in {secs}')
        for _ in range(4):
            try:
                packed = struct.pack('<H', secs)
                self._write_buffer(I2CRegister.REG_REQUEST_SHUTDOWN, packed)
                return
            except CommunicationError as e:
                logger.warn(f'error: {e}')
            except SleepingException:
                pass
            time.sleep(1)

    def send_event_to_power_manager(self):
        pass

    def __str__(self):
        return 'Vbatt {} V, Vrpi {} V, temperature {} C, bootup reason {}'.format(
            self.v_batt, self.v_rpi_3v3, self.temperature, self.startup_reason)