#include <VMA11.h>
#include <Wire.h>
#include <stdint.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

#define SW_BOUNCE   300 //(mSec)
#define KEY_WAIT    2000 //(mSec)
int resetPin = 2;
int SDIO = A4;
int SCLK = A5;

int Pg1 = 3, Pg2 = 4, Pg3 = 5, Pg4 = 6;
int volume_pin = A0;
int tuning_pin = A1;
int vol_input = 0, tuning_input = 0;
int prev_vol_input = 0, prev_tuning_input = 0;;
int tuning_chan = 2;
int LED_PIN_BT1 = A3;
int LED_PIN_BT4 = 12;

VMA11 radio(resetPin, SDIO, SCLK);
int channel,mem_channel;
int volume;

uint16_t p1;

uint16_t stations_store[4];
uint16_t Default_Stations[4];
uint16_t stations_read[4];
uint16_t programmed = 65535;

unsigned char t_flag = 0;

const int rs = 11, en = A2, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
  Serial.begin(9600);

  pinMode(Pg1, INPUT);
  pinMode(Pg2, INPUT);
  pinMode(Pg3, INPUT);
  pinMode(Pg4, INPUT);
  

  pinMode(LED_PIN_BT1, OUTPUT);
  pinMode(LED_PIN_BT4, OUTPUT);

  Default_Stations[0] = 877;
  Default_Stations[1] = 1022;
  Default_Stations[2] = 1025;
  Default_Stations[3] = 942;

  lcd.begin(20, 4);

  lcd.print(" Radio Arduino v1.0 ");
  lcd.setCursor(0, 2);
  lcd.print(" By Pasquale's LAB  ");

  digitalWrite(LED_PIN_BT1, HIGH);
  digitalWrite(LED_PIN_BT4, HIGH);
  delay(2000);
  digitalWrite(LED_PIN_BT1, LOW);
  digitalWrite(LED_PIN_BT4, LOW);

  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("   EEPROM READING   ");

  EEPROM.get(20,programmed);
  if(programmed != 0)
  {
    Serial.println("EEPROM not programmed");
    for(int i = 0; i < 4; i++)
    {
      stations_read[i] = Default_Stations[i];
    }
  }

else
{
  Serial.println("EEPROM programmed");
  EEPROM.get(0,stations_read);
}  

  delay(500);
  lcd.setCursor(0, 2);
  lcd.print("        DONE        ");
  lcd.clear();
  
  lcd.setCursor(0, 1);
  lcd.print("   Radio power ON   ");
  radio.powerOn();
  radio.setVolume(5);

  lcd.setCursor(0, 2);
  lcd.print("    Radio tuning    ");
  channel = 0;
  while(channel != 877)
  {
    
    channel = radio.seekUp();
  }
  
  lcd.setCursor(0, 2);
  lcd.print("        DONE        ");
  lcd.clear();

t_flag = 1; 

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Channel: ");
  lcd.setCursor(0, 2);
  lcd.print("Volume: ");


}

void loop()
{
  if (t_flag == 1)
  {
    tuning_input = analogRead(tuning_pin);
    tuning_chan = map(tuning_input,0,1023,877,1080);
    radio.setChannel(tuning_chan);
    displayInfo();
    prev_tuning_input = tuning_input;
    channel = tuning_chan;
    t_flag = 0;
  }
  

  vol_input = analogRead(volume_pin);
  volume = map(vol_input,0,740,0,15);
  if((abs(vol_input - prev_vol_input)) >= 70)
  {
      radio.setVolume(volume);
      displayInfo();
      prev_vol_input = vol_input;
  } 
  
  tuning_input = analogRead(tuning_pin);
  tuning_chan = map(tuning_input,0,1023,877,1080);
  if((abs(tuning_input - prev_tuning_input)) >= 10)
  {
      radio.setChannel(tuning_chan);
      displayInfo();
      prev_tuning_input = tuning_input;
      channel = tuning_chan;
  }

  read_switches();

  
}

void read_switches(void)
{
  if(digitalRead(Pg1) == 0)
  {
    Set_Store_channel(1,stations_read);
    displayInfo();
  }

  else if(digitalRead(Pg2) == 0)
  {
    Set_Store_channel(2,stations_read);
    displayInfo();
  }

  else if(digitalRead(Pg3) == 0)
  {
    Set_Store_channel(3,stations_read);
    displayInfo();
  }

  else if(digitalRead(Pg4) == 0)
  {
    Set_Store_channel(4,stations_read);
    displayInfo();
  }

  
}

void displayInfo()
{
  char vol[3];
  char chan[5];

  vol[2] = 0x00;
  chan[4] = 0x00;

  sprintf(vol,"%02d",volume);
  sprintf(chan,"%04d",channel);
   lcd.setCursor(9, 0);
   lcd.print(chan); 
   lcd.setCursor(9, 2);
   lcd.print(vol); 
   
}

void Set_Store_channel(int position,uint16_t* work_channels)
{
  unsigned long time = 0;
  switch(position)
  {
    case 1:
      time = millis();
      while(digitalRead(Pg1 == 0) && ((millis() - time) < KEY_WAIT));
      if(digitalRead(Pg1) == 1)
      {
        channel = stations_read[0];
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        delay(200);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      else
      {
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        work_channels[0] = channel;
        memcpy(stations_store,work_channels,(5 * sizeof(uint16_t)));
        EEPROM.put(0,stations_store);
        programmed = 0;
        EEPROM.put(20,programmed);
        delay(500);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      break;

    case 2:
      time = millis();
      while(digitalRead(Pg2 == 0) && ((millis() - time) < KEY_WAIT));
      if(digitalRead(Pg2) == 1)
      {
        channel = stations_read[1];
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        delay(200);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      else
      {
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        work_channels[1] = channel;
        memcpy(stations_store,work_channels,(5 * sizeof(uint16_t)));
        EEPROM.put(0,stations_store);
        programmed = 0;
        EEPROM.put(20,programmed);
        delay(500);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      break;

    case 3:
      time = millis();
      while(digitalRead(Pg3 == 0) && ((millis() - time) < KEY_WAIT));
      if(digitalRead(Pg3) == 1)
      {
        channel = stations_read[2];
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        delay(200);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      else
      {
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        work_channels[2] = channel;
        memcpy(stations_store,work_channels,(5 * sizeof(uint16_t)));
        EEPROM.put(0,stations_store);
        programmed = 0;
        EEPROM.put(20,programmed);
        delay(500);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      break;

    case 4:
      time = millis();
      while(digitalRead(Pg4 == 0) && ((millis() - time) < KEY_WAIT));
      if(digitalRead(Pg4) == 1)
      {
        channel = stations_read[3];
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        delay(200);
        digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      else
      {
        digitalWrite(LED_PIN_BT1, HIGH);
        digitalWrite(LED_PIN_BT4, HIGH);
        work_channels[3] = channel;
        memcpy(stations_store,work_channels,(5 * sizeof(uint16_t)));
        EEPROM.put(0,stations_store);
        programmed = 0;
        EEPROM.put(20,programmed);
        delay(500);
       digitalWrite(LED_PIN_BT1, LOW);
        digitalWrite(LED_PIN_BT4, LOW);
      }
      break;

    

    default:
      break;  
  }

    radio.setChannel(channel);

}


