// Arduino Uno SoftSerial Tx = 9, Rx = 8, unusable = 10

#include <AltSoftSerial.h>
#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>
#include <Wire.h>
#include <TimeLib.h>

AltSoftSerial mySerial;
Si5351 si5351;
JTEncode jtencode;
tmElements_t tm;

//#define WSPR_FREQ_2200        136000UL
//#define WSPR_FREQ_630        474200UL
#define WSPR_FREQ_160      1838080UL
#define WSPR_FREQ_80       3570010UL
#define WSPR_FREQ_40       7040095UL
#define WSPR_FREQ_30      10140140UL
#define WSPR_FREQ_20      14097030UL
#define WSPR_FREQ_17      18106100UL
#define WSPR_FREQ_15      21096050UL
#define WSPR_FREQ_12      24926120UL
#define WSPR_FREQ_10      28126100UL
//#define WSPR_FREQ_6       50294500UL
//#define WSPR_FREQ_4      70091000UL
//#define WSPR_FREQ_4      70151000UL
//#define WSPR_FREQ_2     144490500UL

#define WSPR_TONE_SPACING       146          // ~1.46 Hz
#define WSPR_DELAY              683          // Delay value for WSPR

#define LED_PIN                13

unsigned int minutes = 0, seconds = 0, mseconds = 0;
char call[] = "UR5RBJ";
char loc[] = "KO51";
uint8_t dbm = 30; // 10*log10(P,mw)
uint8_t tx_buffer[255];
int32_t cal_factor = 90680; //258900 - самодельный маленький; 234200 - самодельный с фильтрами; 90680 - готовый китайский модуль на wspr;
unsigned int band = 160; //начнём с низких

void encode() {
  unsigned long freq, freq_band;
  //включаем генератор
  si5351.output_enable(SI5351_CLK0, 1);
  digitalWrite(LED_PIN, HIGH);
  Serial.print("TX at ");
  Serial.println(band);
  //выбор частоты и переход к следующему диапазону
  switch (band) {
    case 160: {
      freq_band = WSPR_FREQ_160;
      band = 80;
    } break;
    case 80: {
      freq_band = WSPR_FREQ_80;
      band = 40;
    } break;
    case 40: {
      freq_band = WSPR_FREQ_40;
      band = 30;
    } break;
    case 30: {
      freq_band = WSPR_FREQ_30;
      band = 20;
    } break;
    case 20: {
      freq_band = WSPR_FREQ_20;
      band = 17;
    } break;
    case 17: {
      freq_band = WSPR_FREQ_17;
      band = 15;
    } break;
    case 15: {
      freq_band = WSPR_FREQ_15;
      band = 12;
    } break;
    case 12: {
      freq_band = WSPR_FREQ_12;
      band = 10;
    } break;
    case 10: {
      freq_band = WSPR_FREQ_10;
      band = 160;
    } break;
  }
  //собственно передача
  for(uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++) {
    freq = (freq_band * 100) + (tx_buffer[i] * WSPR_TONE_SPACING);
    si5351.set_freq(freq, SI5351_CLK0);
    delay(WSPR_DELAY);
  }
  //выключаем генератор
  si5351.output_enable(SI5351_CLK0, 0);
  digitalWrite(LED_PIN, LOW);
}

void set_tx_buffer() {
  memset(tx_buffer, 0, 255);
  jtencode.wspr_encode(call, loc, dbm, tx_buffer);
}

void setup()  
{
  //Инициализируем последовательные интерфейсы
  Serial.begin(115200);
  mySerial.begin(115200);
  //Сишка
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0); //8 пикофарад нагрузки на кварц
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO); //калибровка кварца
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA); //ФАПЧ А
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); //Максимальная мощность
  si5351.output_enable(SI5351_CLK0, 0); //выкл передачу
  //контролька
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  //формирование строки на передачу
  set_tx_buffer();
  Serial.println("Setup OK");
}

void gps() {
  String myString, minS, secS, mseS;
  char c;
  byte x;
  boolean f = 1;
  do {
    //выбираем однну строку из вывода GPS-а
    do {
      if (mySerial.available()) {
        x = mySerial.read();
        c = (char)x;
        myString += c;
        if (myString.length() > 80) { //если переполнение
          //длиннейшее: $GPRMC,213058.000,A,5130.3939,N,03119.7558,E,0.79,262.87,040903,,,A*68
          myString = "";
        }
      }
    } while (x != 10);
    myString.trim(); //отбросим символ конца строки и что там ещё лишнее
    //вытянем время из ранних строк, поздние иногда приходят с опозданием
    //GPRMC - Recommended Minimum Specific GNSS Data
    if (myString.indexOf("$GPRMC,") == 0) {
      Serial.println(myString);
      myString = myString.substring(7, 17);
      if (myString.toInt()) { //if its correct digits
//        Serial.println(myString);
        minS = myString.substring(2, 4);
        secS = myString.substring(4, 6);
        mseS = myString.substring(7, 10);
        minutes = minS.toInt();
        seconds = secS.toInt();
        mseconds = mseS.toInt();
        f = 0;
      }
    }
/*
    //print all commands
    if (myString.indexOf("$GP") == 0) {
      Serial.println(myString);
    }
*/
    myString = "";
  } while (f);
}

void loop()
{
  gps(); //читаем GPS и вытягиваем время
  //передача должна начаться с первой секунды чётной минуты
  if ((minutes % 2 ) && (seconds == 59 )) {  //ориентируемся по последней секунде нечётной минуты,
    delay(1700 - mseconds);                  //и дождёмся первой чётной
    //от сих до начала передачи +300 мсек
    encode();   //поехали
    Serial.println("Sent");
  }
  Serial.println("Wait...");
}
