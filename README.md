# si5351-wspr
si5351-based wspr Tx with GPS/RTC/PC variants
## Using RTC
Here you can use code for 1302 or 1307 chip, just renaming what you need by *.ino.
## Using GPS
GPS serial connected to software serial pins, see code.
## Using PC
You must run python code to provide time via Arduino USB connection. This code written for Ubuntu, so you must change it a little, if you want to run it under Windows.
## Notes
- Provide your own amateur callsign before compile code. Yes, you need to have it to transmit something.
- Use filters with si5351.
- You will need some libraries, which can be found over Arduino IDE.
