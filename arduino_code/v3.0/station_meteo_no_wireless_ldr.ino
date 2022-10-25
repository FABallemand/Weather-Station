// I2C
#include <Wire.h>

// LCD Screen
#include <LiquidCrystal.h>

// Temperature & Humidity Sensor
#include "AHT10.h"

// Time
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// LCD Screen
#define LDR A0
int ldr_value;
int ldr_values[4] = {0, 10, 180, 270};
// FOr blue screen
int a_values[4] = {10, 128, 128, 128}; // Backlight
int vo_values[4] = {100, 128, 128, 128}; // Text
// For green screen
// int a_values[4] = {0, 8, 128, 255}; // Backlight
// int vo_values[4] = {0, 0, 0, 0}; // Text
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2, vo = 7, a = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Temperature & Humidity Sensor
uint8_t readStatus_int = 0;
float temperature_int = 0;
float humidity_int = 0;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

// Time
tmElements_t tm;

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void setup()
{
  // I2C
  Serial.begin(9600);
  delay(200);

  // LCD Screen
  pinMode(LDR, INPUT);
  // pinMode(a, OUTPUT);
  // pinMode(vo, OUTPUT); // Not required
  adjustScreenLuminosity();
  lcd.begin(16, 2);
  lcd.print("Station meteo !!");

  // Time
  bool parse = false;
  bool config = false;

  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__))
  {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm))
    {
      config = true;
    }
  }

  while (!Serial)
    ; // wait for Arduino Serial Monitor
  delay(200);
  if (parse && config)
  {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  }
  else if (parse)
  {
    Serial.println("DS1307 Communication Error");
  }
  else
  {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.print("\", Date=\"");
    Serial.print(__DATE__);
    Serial.println("\"");
  }

  // Temperature & Humidity Sensor
  while (myAHT10.begin() != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }

  delay(200);
}

void loop()
{
  // Temperature & Humidity Sensor
  readStatus_int = myAHT10.readRawData();

  if (readStatus_int != AHT10_ERROR)
  {
    temperature_int = myAHT10.readTemperature(AHT10_USE_READ_DATA);
    humidity_int = myAHT10.readHumidity(AHT10_USE_READ_DATA);
  }
  else
  {
    myAHT10.softReset(); // reset 1-success, 0-failed
  }

  // Time
  if (!RTC.read(tm))
  {
    if (RTC.chipPresent())
    {
      Serial.println("The DS1307 is stopped");
      Serial.println();
    }
    else
    {
      Serial.println("DS1307 read error!");
      Serial.println();
    }
  }

  // LCD Screen
  adjustScreenLuminosity();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date");
  lcd.setCursor(2, 1);
  print2digits(tm.Day);
  lcd.print("/");
  print2digits(tm.Month);
  lcd.print("/");
  lcd.print(tmYearToCalendar(tm.Year));
  delay(10000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Heure");
  lcd.setCursor(2, 1);
  print2digits(tm.Hour);
  lcd.print(":");
  print2digits(tm.Minute);
  delay(10000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature int");
  lcd.setCursor(9, 1);
  lcd.print(temperature_int);
  lcd.print(" C");
  delay(10000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidite int");
  lcd.setCursor(9, 1);
  lcd.print(humidity_int);
  lcd.print(" %");
  delay(10000);
}

// Functions
void adjustScreenLuminosity()
{
  ldr_value = analogRead(LDR);
  Serial.println(ldr_value);
  for (int i = 0; i < 4; i++)
  {
    if (ldr_value > ldr_values[i])
    {
      Serial.println(i);
      analogWrite(a, a_values[i]);
      analogWrite(vo, vo_values[i]);
    }
  }
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3)
    return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3)
    return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++)
  {
    if (strcmp(Month, monthName[monthIndex]) == 0)
      break;
  }
  if (monthIndex >= 12)
    return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number)
{
  if (number >= 0 && number < 10)
  {
    lcd.print('0');
  }
  lcd.print(number);
}
