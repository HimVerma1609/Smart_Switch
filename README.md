# Smart_Switch

This project uses an MPR121 Capacitive Touch Sensor and esp8266.   
MPR121 uses 6 pins for touch and 6 pins for output.  
Whenever pins ELE0 to ELE5 are touched ELE6 to ELE11 goes high.  
Also ESP8266 hosts a web server to control pins of MPR121 through I2C protocol.  
