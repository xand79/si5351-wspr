/* WSPR с использованием библиотеки JTEncode
 * Si5351 + DS1302
*/

#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>
//#include "Wire.h"
#include <TimeLib.h>
#include <RTClib.h>

Si5351 si5351;
JTEncode jtencode;
tmElements_t tm;
DS1302 rtc(PA8, PA10, PA9); //RST-CLK-DAT
char bufT[20]; // buffer for DateTime.tostr
String s; //also

//#define WSPR_FREQ        136000UL
//#define WSPR_FREQ        474200UL
//#define WSPR_FREQ       1836600UL
//#define WSPR_FREQ       3570010UL
//#define WSPR_FREQ       7040095UL
//#define WSPR_FREQ      10140140UL
//#define WSPR_FREQ      14097030UL
//#define WSPR_FREQ      18106100UL
//#define WSPR_FREQ      21094600UL
//#define WSPR_FREQ      24924600UL
//#define WSPR_FREQ      28124600UL
#define WSPR_FREQ      50294500UL
//#define WSPR_FREQ      70091000UL
//#define WSPR_FREQ      70151000UL
//#define WSPR_FREQ     144489000UL

#define WSPR_TONE_SPACING       146          // ~1.46 Hz
#define WSPR_DELAY              683          // Delay value for WSPR

#define LED_PIN                PC13

char call[] = "UT4RC";
char loc[] = "KO51ol";
uint8_t dbm = 37;
uint8_t tx_buffer[255];
int32_t cal_factor = 234260; //258900 - самодельный маленький; 234200 - самодельный с фильтрами; 90680 - готовый китайский модуль на wspr;

void encode() {
  unsigned long freq;
  si5351.output_enable(SI5351_CLK0, 1);
  digitalWrite(LED_PIN, HIGH);

  for(uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++) {
    freq = (WSPR_FREQ * 100) + (tx_buffer[i] * WSPR_TONE_SPACING);
    si5351.set_freq(freq, SI5351_CLK0);
    delay(WSPR_DELAY);
  }

  si5351.output_enable(SI5351_CLK0, 0);
  digitalWrite(LED_PIN, LOW);
}

void set_tx_buffer() {
  memset(tx_buffer, 0, 255);
  jtencode.wspr_encode(call, loc, dbm, tx_buffer);
}

void setup() {
  Serial.begin(115200);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 0); // Disable the clock initially
  Serial.println("Si5351 setup OK");

  rtc.begin();
  DateTime now = rtc.now();
  s = now.tostr(bufT);
  Serial.println(s);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  set_tx_buffer();
  Serial.println("Setup OK");
}

void loop() {
  DateTime now = rtc.now();
  s = now.tostr(bufT);
  int mi = s.substring(14, 16).toInt();
  int se = s.substring(17, 19).toInt();
  if (!(mi % 2 ) && (se == 1 )) { //передача с первой секунды чётной минуты
    Serial.println("Tx...");
    encode();
    Serial.println("Sent");
  }
}
