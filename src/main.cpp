#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <FlashStorage_STM32.h>

#include <ledControl.h>

std::array<Adafruit_NeoPixel, 6> lines{
    Adafruit_NeoPixel(16, PB1, NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(17, PB0, NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(17, PA7, NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(13, PA6, NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(13, PA5, NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(11, PA3, NEO_GRB + NEO_KHZ800),
};

LedControl<6> control(lines);
LedControlState lastState;

void timerCallback()
{
  control.UpdateEffect();
}

void saveCallback()
{
  LedControlState state = control.SaveState();
  if (state == lastState)
  {
    return;
  }

  EEPROM.put(0, 0xF0F0F0F0);
  EEPROM.put(4, state.Type);
  if (state.Type == 0)
  {
    EEPROM.put(5, state.ArraySize);
    for (int i = 0; i < state.ArraySize; i++)
    {
      EEPROM.put(9 + (i * sizeof(Pixel)), state.Pixels[i]);
    }
  }
  else
  {
    EEPROM.put(5, state.EffectIndex);
  }
  EEPROM.commit();
}

void setup()
{
  Serial.begin(115200);
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, HIGH);
  HardwareTimer* timer = new HardwareTimer(TIM1);
  timer->setOverflow(30, TimerFormat_t::HERTZ_FORMAT);
  timer->attachInterrupt(timerCallback);
  timer->resume();

  int sign = 0;
  EEPROM.get(0, sign);
  if (sign == 0xF0F0F0F0)
  {
    LedControlState state;
    EEPROM.get(4, state.Type);
    if (state.Type == 0)
    {
      EEPROM.get(5, state.ArraySize);
      for (int i = 0; i < state.ArraySize; i++)
      {
        Pixel pixel;
        EEPROM.get(9 + (i * sizeof(Pixel)), pixel);
      }
    }
    else
    {
      EEPROM.get(5, state.EffectIndex);
    }

    control.LoadState(state);
  }

  HardwareTimer* saveTimer = new HardwareTimer(TIM2);
  saveTimer->setOverflow(10 * 1000 * 1000, TimerFormat_t::MICROSEC_FORMAT);
  saveTimer->attachInterrupt(saveCallback);
  saveTimer->resume();
}

void loop()
{
  digitalWrite(PC13, HIGH);
  if (Serial.available())
  {
    int type = Serial.read() - '0';
    switch (type)
    {
    case 0:
    {
      auto packet = LinePacket(
        Serial.read(),
        Color{
            .Red = (uint8_t)Serial.read(),
            .Green = (uint8_t)Serial.read(),
            .Blue = (uint8_t)Serial.read()
        });
      packet.Apply(control);
      break;
    }
    case 1:
    {
      std::vector<Pixel> pixels;
      while (Serial.available())
      {
        pixels.push_back(
          Pixel{
              .Line = Serial.read(),
              .Pixel = Serial.read(),
              .PixelColor = Color{
                  .Red = (uint8_t)Serial.read(),
                  .Green = (uint8_t)Serial.read(),
                  .Blue = (uint8_t)Serial.read()
                  }
          });
      }
      auto packet = PixelsPacket(pixels);
      packet.Apply(control);
      break;
    }
    case 2:
    {
      auto packet = FillPacket(
        Color{
                  .Red = (uint8_t)Serial.read(),
                  .Green = (uint8_t)Serial.read(),
                  .Blue = (uint8_t)Serial.read()
        });
      packet.Apply(control);
      break;
    }
    case 3:
    {
      auto packet = EffectPacket(Serial.read());
      packet.Apply(control);
      break;
    }
    case 4:
    {
      auto packet = SettingsPacket((uint8_t)Serial.read());
      packet.Apply(control);
      break;
    }
    case 5:
    {
      Serial.write("ok!");
      break;
    }
    }
    Serial.write("!");
  }
  digitalWrite(PC13, LOW);
  delay(100);
}
