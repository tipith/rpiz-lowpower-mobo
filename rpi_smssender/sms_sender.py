
import phone
import time
import logging
import configparser
from contextlib import contextmanager
from motherboard import Motherboard, StartupReason
from logs import setup_logging

logger = logging.getLogger('main')


def get_config(conf_file):
    conf = configparser.ConfigParser()
    conf.read(conf_file)
    logger.info({section: dict(conf[section]) for section in conf.sections()})
    return conf


@contextmanager
def phone_context(cls, pin):
    p = cls(pin)
    yield p
    p.shutdown()


def finnish_alarm(name, reason, vbatt, temperature):
    reason_map = {
        StartupReason.EXT_TRIGGER1: 'aktiivisesta sisäänmenosta (#1)',
        StartupReason.EXT_TRIGGER2: 'aktiivisesta sisäänmenosta (#2)',
        StartupReason.LOW_BATTERY: 'matalasta akkujännitteestä',
    }
    reason_str = reason_map[reason] if reason in reason_map else 'heränneensä tuntemattomalla syyllä'
    vbatt_str = f'{vbatt:.02f} V' if vbatt else 'tuntematon'
    return f'{name} ilmoittaa {reason_str}. Lämpötila {temperature} C. Akkujännite {vbatt_str}.'


def run(conf_file):
    setup_logging('logs')
    conf = get_config(conf_file)
    mb = Motherboard()
    mb.start()
    try:
        b = phone.Battery(vrpi=mb.v_rpi_3v3)
        b.bsi = 82e3
        b.btemp = 50e3
        with phone_context(phone.Phone, conf['phone']['pin']) as p:
            while not p.registered:
                time.sleep(0.5)
            logger.info(p)
            logger.info(mb)
            msg = finnish_alarm(conf['alarm']['name'], mb.startup_reason, mb.v_batt, mb.temperature)
            p.send_sms(conf['phone']['number'], msg)
    finally:
        mb.request_shutdown(10)
        mb.stop()


if __name__ == '__main__':
    run('/boot/trapsms.ini')