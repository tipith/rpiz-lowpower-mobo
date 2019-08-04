import datetime, time

import gammu
import pigpio

from pins import *

class Battery:

    PWM_FREQ = 100000

    def __init__(self, bsi_pin=PIN_PWM0, btemp_pin=PIN_PWM1, vref=2.8, vrpi=3.3):
        self.pi = pigpio.pi()
        self.pin_bsi = bsi_pin
        self.pin_btemp = btemp_pin
        self.vref = vref
        self.bsi_resistance = 0
        self.btemp_resistance = 0
        self.v_rpi = vrpi if vrpi else 3.3

    def _calc_dc(self, res_value):
        vtarget = res_value / (100e3 + res_value) * self.vref
        return int(1e6 * vtarget / self.v_rpi), vtarget

    @property
    def bsi(self):
        return self.bsi_resistance

    @bsi.setter
    def bsi(self, res_value):
        self.bsi_resistance = res_value
        dc, vtarget = self._calc_dc(self.bsi)
        print(f'bsi {self.bsi} ohm ({dc / 1e6:.2f}, {vtarget:.2f} V)')
        self.pi.hardware_PWM(self.pin_bsi, Battery.PWM_FREQ, dc)

    @property
    def btemp(self):
        return self.btemp_resistance

    @btemp.setter
    def btemp(self, res_value):
        self.btemp_resistance = res_value
        dc, vtarget = self._calc_dc(self.btemp)
        print(f'btemp {self.btemp} ohm ({dc / 1e6:.2f}, {vtarget:.2f} V)')
        self.pi.hardware_PWM(self.pin_btemp, Battery.PWM_FREQ, dc)

class DummyPhone:

    def __init__(self, pin):
        pass

    def shutdown(self):
        pass

    @property
    def registered(self):
        return True

    @property
    def ready(self):
        return True

    def send_sms(self, number, text):
        print(f'sms to {number}, contents: {text}')

    def __str__(self):
        return 'dummy phone'


class Phone:

    def __init__(self, pin):
        self._ready = False
        self.sm = gammu.StateMachine()
        self.sm.ReadConfig(Filename='gammu.ini')
        self._startup()
        self._enter_pin(pin)

    def _startup(self):
        self.sm.Init()
        print('hw={}, imei={}, manuf={}, charge={}'.format(
            self.sm.GetHardware(), self.sm.GetIMEI(),
            self.sm.GetManufacturer(), self.sm.GetBatteryCharge()['BatteryPercent']))
        self.sm.PressKey(Key='p', Press=True)
        time.sleep(5)
        self.sm.Terminate()
        for _ in range(3):
            try:
                self.sm.Init()
                break
            except gammu.ERR_TIMEOUT:
                pass

    def _enter_pin(self, pin):
        for _ in range(5):
            try:
                if self.sm.GetSecurityStatus() == 'PIN':
                    print('security - status={}, pin={}'.format(self.sm.GetSecurityStatus(), pin))
                    self.sm.EnterSecurityCode('PIN', pin)
                self._ready = True
                break
            except gammu.ERR_TIMEOUT:
                print('pin failed')
                time.sleep(3.0)

    def shutdown(self):
        self.sm.PressKey(Key='p', Press=True)

    def reset(self):
        self.sm.Reset()

    @property
    def registered(self):
        net = self.sm.GetNetworkInfo()
        return net['State'] == 'HomeNetwork' or net['State'] == 'RoamingNetwork'

    @property
    def ready(self):
        return self._ready

    def send_sms(self, number, text):
        message = {'Text': text, 'SMSC': {'Location': 1}, 'Number': number}
        self.sm.SendSMS(message)

    def __str__(self):
        bat = self.sm.GetBatteryCharge()
        try:
            dt = self.sm.GetDateTime()
        except gammu.ERR_EMPTY:
            dt = 'no time'
        net = self.sm.GetNetworkInfo()
        sig = self.sm.GetSignalQuality()
        netname = gammu.GSMNetworks.get(net["NetworkCode"], 'searching')
        ret = f'Network {netname} ({net["NetworkCode"]}), lac={net["LAC"]}, cid={net["CID"]}, ' \
              f'sig={sig["SignalStrength"]}, sig%={sig["SignalPercent"]}, ' \
              f'{dt}, battery: {bat["BatteryPercent"]} {bat["ChargeState"]}'
        return ret


def test():
    pass


if __name__ == '__main__':
    test()
