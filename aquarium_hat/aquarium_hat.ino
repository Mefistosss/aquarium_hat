#include <iarduino_RTC.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

 
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack
iarduino_RTC time(RTC_DS1302, 10, 13, 12);

const int menuButtonPin = 3;
const int selectButtonPin = 6;
const int lightsPin = 5;
const int socketPin = 7;
const int fanPin = 8;
const int fanBoardPin = 9;

int menuButtonState = 0;
int menuTypeState = -1;
int menuTypeState2 = -1;
int selectButtonState = 0;
int selectTypeState = -1;

int lightState = 0;
int socketState = 0;
int fanState = 0;
int fanBoradState = 0;

String nameOfSelectType = "";

/* datas[]
 *  datas[*][0] - hours
 *  datas[*][1] - minutes
 *  
 *  datas[0][*] - light on
 *  datas[1][*] - light off
 *  datas[2][*] - socket on
 *  datas[3][*] - socket off
*/
int datas[4][2] = {{7, 0}, {20, 0}, {5, 0}, {23, 0}};
int copyDatas[4][2] = {{7, 0}, {20, 0}, {5, 0}, {23, 0}};
String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

int lcdState = HIGH;
unsigned long lastLight = 0;

const int timeOfDisplayLight = 30000;

void setup()
{
//  Serial.begin(9600);
//  Serial.print("\t");
  delay(300);
  time.begin();
//  time.settime(0,20,20);
  //  time.blinktime(2);
  
  // activate LCD module
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(lcdState);

  time.gettime();
  if (time.seconds == 0 && time.minutes == 0 && time.Hours == 0)
  {
    time.settime(0,0,0);
    lcd.home ();
    lcd.print(time.gettime("H:i:s"));
//    time.blinktime(2);
  }

  if (EEPROM.read(0) == 255) {
    EEPROM.write(0, datas[0][0]);
    EEPROM.write(1, datas[0][1]);
    EEPROM.write(2, datas[1][0]);
    EEPROM.write(3, datas[1][1]);
    EEPROM.write(4, datas[2][0]);
    EEPROM.write(5, datas[2][1]);
    EEPROM.write(6, datas[3][0]);
    EEPROM.write(7, datas[3][1]);
  } else {
    datas[0][0] = EEPROM.read(0);
    datas[0][1] = EEPROM.read(1);
    datas[1][0] = EEPROM.read(2);
    datas[1][1] = EEPROM.read(3);
    datas[2][0] = EEPROM.read(4);
    datas[2][1] = EEPROM.read(5);
    datas[3][0] = EEPROM.read(6);
    datas[3][1] = EEPROM.read(7);
  }

  pinMode(menuButtonPin, INPUT); 
  pinMode(selectButtonPin, INPUT);
  pinMode(lightsPin, OUTPUT);
  pinMode(socketPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(fanBoardPin, OUTPUT);
  digitalWrite(fanBoardPin, HIGH);
}

void loop()
{
  unsigned long currentMillis = millis();
  
  buttonsClick(currentMillis);
  
  menu(currentMillis);

  if(digitalRead(menuButtonPin) == LOW && menuButtonState == 1) {
    menuButtonState = 0;
  }

  if(digitalRead(selectButtonPin) == LOW && selectButtonState == 1) {
    selectButtonState = 0;
  }

  checkStates(currentMillis);

  if (currentMillis % 1000 == 0) {
    switchState(lightsPin, lightState);
    switchState(socketPin, socketState);
    switchState(fanPin, fanState);
    
    if (currentMillis - lastLight >= timeOfDisplayLight && lcdState == HIGH) {
      eppromWrite();
      menuTypeState = -1;
      menuTypeState2 = -1;
      lcdState = LOW;
      lcd.setBacklight(lcdState);
    }
  }

  delay(1);// show time not more than one time in 1ms
}

void switchState(int pin, int state) {
  if (state == 0) {
    if (digitalRead(pin) == HIGH) {
      digitalWrite(pin, LOW);  
    }
  } else {
    if (digitalRead(pin) == LOW) {
      digitalWrite(pin, HIGH);  
    }
  }
}

int checkButtonState(int buttonPin, int buttonState)
{
  if(digitalRead(buttonPin) == HIGH && buttonState == 0) {
    return 1;
  } else {
    return 0;
  }
}

void buttonsClick(long currentMillis)
{ 
  if (checkButtonState(menuButtonPin, menuButtonState) || checkButtonState(selectButtonPin, selectButtonState)) {
    if (checkButtonState(menuButtonPin, menuButtonState)) {
      menuButtonState = 1;
    } else {
      selectButtonState = 1;
    }
    if (lcdState == HIGH) {
//      if (checkButtonState(menuButtonPin, menuButtonState)) {
      if (menuButtonState == 1) {
        if (menuTypeState2 > -1) {
          menuTypeState2++;
          if (menuTypeState < 4 && menuTypeState2 > 1 || menuTypeState2 > 2) {
            menuTypeState2 = -1;
            eppromWrite();
          }
        } else {
          menuTypeState++;
          if (menuTypeState > 4) {
            menuTypeState = -1;
            mainDisplay(currentMillis, 1);
          }
        }
        menuButtonState = 1;
      } else {
        if (menuTypeState > -1) {
          if (menuTypeState2 > -1) {
            select();
          } else {
            menuTypeState2++;
          }
        }
        selectButtonState = 1;
      }
    } else {
      lcdState = HIGH;
      lcd.setBacklight(lcdState);
    }
    lastLight = currentMillis;
  }
}

void menu(long currentMillis)
{
  if (menuTypeState2 < 0) {
    if (menuTypeState < 0) {
      mainDisplay(currentMillis, 0);
    } else {
      firstMenu();
    }
  } else {
    secondMenu();
  }
}

void firstMenu()
{
  String menuStr = "";
  menuStr = "Menu:  ";
  nameOfSelectType = "Turn o";
  switch(menuTypeState) {
    case 0:
      menuStr += "*----";
      nameOfSelectType += "n light   ";
    break;
    case 1:
      menuStr += "-*---";
      nameOfSelectType += "ff light  ";
    break;
    case 2:
      menuStr += "--*--";
      nameOfSelectType += "n socket  ";
    break;
    case 3:
      menuStr += "---*-";
      nameOfSelectType += "ff socket ";
    break;
    case 4:
      menuStr += "----*";
      nameOfSelectType = "Set time        ";
    break;
  }
  
  menuStr += "    ";
  
  lcd.home();
  lcd.print(menuStr);
  lcd.setCursor (0, 1);
  lcd.print(nameOfSelectType);
}

String getSecondMenuStr(int value,  int state)
{
  String str, space = "", stateOfMenu, v = (String)value;
  if (value < 10 && state < 2) {
    v = "0" + v;
  }

  switch(state) {
    case 0:
      str = "Hours: " + v;
      if (menuTypeState == 4) {
        stateOfMenu = "*--";
        space = "    ";
      } else {
        stateOfMenu = "*-";
        space = "     ";
      }
      break;
    case 1:
      stateOfMenu = "-*";
      str = "Minutes: " + v;
      if (menuTypeState == 4) {
        stateOfMenu = "-*-";
        space = "  ";
      } else {
        stateOfMenu = "-*";
        space = "   ";
      }
      break;
    case 2:
      stateOfMenu = "--*";
      space = "     ";
      str = "Day: " + weekDays[value];
      break;
  }

  str = str  + space + stateOfMenu;
  return str;
}

void secondMenu()
{
  lcd.home();
  lcd.print(nameOfSelectType);
  lcd.setCursor(0, 1);

  if (menuTypeState == 4) {
    timeMenu();
  } else {
    lcd.print(getSecondMenuStr(datas[menuTypeState][menuTypeState2], menuTypeState2));
  }
}

void timeMenu()
{
  int value;
  time.gettime();
  switch(menuTypeState2) {
    case 0:
      value = time.Hours;
      break;
    case 1:
      value = time.minutes;
      break;
    case 2:
      value = time.weekday;
      break;
  }
  lcd.print(getSecondMenuStr(value, menuTypeState2));
}

void select()
{
  int m, h, d;
  if (menuTypeState == 4) {
    time.gettime();
    m = time.minutes;
    h = time.Hours;
    d = time.weekday;
    switch(menuTypeState2) {
      case 0:
         h = h + 1;
        if (h == 24) { h = 0; }
        break;
      case 1:
        m = m + 1;
        if (m == 60) { m = 0; }
        break;
      case 2:
        d = d + 1;
        if (d == 7) { d = 0; }
        break;
    }

    time.settime(-1, m, h, -1, -1, -1, d);
  } else {
    datas[menuTypeState][menuTypeState2]++;
    if (menuTypeState2) {
      if (datas[menuTypeState][menuTypeState2] > 59) {
        datas[menuTypeState][menuTypeState2] = 0;
      }
    } else {
      if (datas[menuTypeState][menuTypeState2] > 23) {
        datas[menuTypeState][menuTypeState2] = 0;
      }
    }
  }
}

void mainDisplay(long currentMillis, int force)
{
  if(currentMillis % 1000 == 0 || force) {
    lcd.home ();
    lcd.print(time.gettime("H:i:s, D"));
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
}

void eppromWrite()
{
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 2; j++) {
      if (datas[i][j] != copyDatas[i][j]) {
        copyDatas[i][j] = datas[i][j];
        EEPROM.write(i * 2 + j, datas[i][j]);
      }
    }
  }
}

boolean checkRightDirection(int h1, int h2, int m1, int m2)
{
  if(h1 == h2) {
    return m1 < m2;
  } else {
    return h1 < h2;
  }
}

boolean isMore(int currentH, int currentM, int h, int m)
{
  if (currentH >= h) {
    if (currentM >= m) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

int getFanTime(int mas[], boolean on)
{
  int fan = 1, h = mas[0], m = mas[1];

  if(on) {
    m = m - fan;
    if (m < 0) {
      h = h - 1;
      m = 59 + m;
      if (h < 0) {
        h = 23;
      }
    }
  } else {
    m = m + fan;
    if (m > 59) {
      h = h + 1;
      m = 0;
      if (h > 23) {
        h = 0;
      }
    }
  }
  mas[0] = h;
  mas[1] = m;
}

void checkStates(long currentMillis)
{
  int m, h;
  if(currentMillis % 1000 == 0) {
    int onFan[2], offFan[2];
    time.gettime();
    m = time.minutes;
    h = time.Hours;
    onFan[0] = datas[0][0];
    onFan[1] = datas[0][1];
    offFan[0] = datas[1][0];
    offFan[1] = datas[1][1];
    getFanTime(onFan, true);
    getFanTime(offFan, false);

    if (checkRightDirection(datas[0][0], datas[1][0], datas[0][1], datas[1][1])) {
      if (isMore(h, m, datas[0][0], datas[0][1]) && !isMore(h, m, datas[1][0], datas[1][1])) {
        lightState = 1;
      } else {
        lightState = 0;
      }

      if (isMore(h, m, onFan[0], onFan[1]) && !isMore(h, m, offFan[0], offFan[1])) {
        fanState = 1;
      } else {
        fanState = 0;
      }
    } else {
      if (isMore(h, m, datas[1][0], datas[1][1]) && !isMore(h, m, datas[0][0], datas[0][1])) {
        lightState = 0;
      } else {
        lightState = 1;
      }

      if (isMore(h, m, offFan[0], offFan[1]) && !isMore(h, m, onFan[0], onFan[1])) {
        fanState = 0;
      } else {
        fanState = 1;
      }
    }

    if (checkRightDirection(datas[2][0], datas[3][0], datas[2][1], datas[3][1])) {
      if (isMore(h, m, datas[2][0], datas[2][1]) && !isMore(h, m, datas[3][0], datas[3][1])) {
        socketState = 1;
      } else {
        socketState = 0;
      }
    } else {
      if (isMore(h, m, datas[3][0], datas[3][1]) && !isMore(h, m, datas[2][0], datas[2][1])) {
        socketState = 0;
      } else {
        socketState = 1;
      }
    }
  }
}
