#include <Wire.h>
#include <util/crc16.h>

uint16_t get_temperature(void)
{
  analogReference(INTERNAL1V1);
  delay(1);
  ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;
  ADC0.CTRLC &= ~(ADC_INITDLY_gm);
  ADC0.CTRLD = ADC_INITDLY_DLY256_gc;
  ADC0.SAMPCTRL = 31;
  ADC0.CTRLC |= ADC_SAMPCAP_bm;
  ADC0.COMMAND = ADC_STCONV_bm;
  while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  uint16_t adc_reading = ADC0.RES;
  analogReference(INTERNAL2V5);

  int8_t sigrow_offset = SIGROW.TEMPSENSE1;
  uint8_t sigrow_gain = SIGROW.TEMPSENSE0;

  uint32_t temp = adc_reading - sigrow_offset;
  temp *= sigrow_gain; // Result might overflow 16 bit variable (10bit+8bit)
  temp += 0x80; // Add 1/2 to get correct rounding on division below
  temp >>= 8; // Divide result to get Kelvin
  uint16_t temperature_in_K = temp;

  return temperature_in_K;
}

unsigned int append_crc(uint8_t* data, uint32_t len)
{
#define CRC_LEN 2
  uint16_t crc = 0;
  for (unsigned int i = 0; i < len; ++i)
  {
    crc = _crc_xmodem_update(crc, data[i]);
  }
  
  data[len] = crc & 0xff;
  data[len + 1] = (crc >> 8) & 0xff; 
  return len + CRC_LEN;
}

enum iopin {
  PIN_IO0 = 9,
  PIN_IO1 = 2,
  PIN_IO2 = 10,
  PIN_IO3 = 1,
  PIN_RPI_EVENT = 5,
  PIN_PWR_EN = 3,
  PIN_LED = 4,
  PIN_SCL = 7,
  PIN_SDA = 6,
  PIN_ADC_VBATT = 8,
  PIN_ADC_RPI_3V3 = 0,
};

enum i2c_reg {
  REG_VBATT = 1,
  REG_VRPI,
  REG_STARTUP_REASON,
  REG_DEBUG_DATA,
  REG_TEMPERATURE,
};

#define I2C_ADDR 8
char data[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned int out_len = 8;
unsigned long vbatt, vrpi, temperature;

void requestEvent() 
{
   digitalWrite(PIN_LED, digitalRead(PIN_LED) ? LOW : HIGH);
   Wire.write(data, out_len);
}

void receiveEvent(int howMany) 
{
  int reg = Wire.read();
  out_len = 0;
  
  switch (reg)
  {
    case REG_VBATT:
      data[out_len++] = vbatt & 0xff;
      data[out_len++] = (vbatt >> 8) & 0xff;
      break;
    case REG_VRPI:
      data[out_len++] = vrpi & 0xff;
      data[out_len++] = (vrpi >> 8) & 0xff;
      break;
    case REG_STARTUP_REASON:
      data[out_len++] = 2;
      data[out_len++] = 0;
      break;
    case REG_DEBUG_DATA:
      strcpy(data, "moi");
      out_len = strlen(data);
      break;
    case REG_TEMPERATURE:
      data[out_len++] = temperature & 0xff;
      data[out_len++] = (temperature >> 8) & 0xff;
      break;
  }

  out_len = append_crc(data, out_len);
}

void setup() 
{
  Wire.begin(I2C_ADDR);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  analogReference(INTERNAL2V5);

  pinMode(PIN_PWR_EN, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  digitalWrite(PIN_PWR_EN, LOW);
}

void loop() 
{
  delay(1000);
  vbatt = map(analogRead(PIN_ADC_VBATT), 0, 1024, 0, 5250);
  vrpi = map(analogRead(PIN_ADC_RPI_3V3), 0, 1024, 0, 3750);
  temperature = get_temperature();
}
