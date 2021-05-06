//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "mpr121.h"             // for touch Sensors
#include <Wire.h>               // I2C communication

int irqpin = 14;  // Digital 5 on Node MCU (GPIO 14)
uint8_t gpioPast = 0x00;  // to initialize it with a zero value, all switches are electrically open 


ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

void handleRoot();              // function prototypes for HTTP handlers
void handleLogin();
void handleMenu();
void handleSubmit();
void handleStatus();
void handleNotFound();

void setup(){
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP("Smart Switch");                // Creating passage for connecting to WiFi Router
  delay(1000);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);

  server.on("/", HTTP_GET, handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/login", HTTP_POST, handleLogin); // Call the 'handleLogin' function when a POST request is made to URI "/login"
  server.on("/menu",HTTP_GET, handleMenu);    // Call the 'handleMenu' function when a POST request is madde to URI "/menu"
  server.on("/submit",HTTP_POST,handleSubmit);
  server.onNotFound(handleNotFound);           // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.on("/menu/status",HTTP_GET, handleStatus);
  
  server.begin();                              // Actually start the server
  Serial.println("HTTP server started");
  

  pinMode(irqpin, INPUT);                      // for I2C and MPR121 touch sensor
  digitalWrite(irqpin, HIGH);                  //enable pullup resistor
  Wire.begin();
  mpr121_setup(); 
  
  
}

void loop(){
  server.handleClient();                     // Listen for HTTP requests from clients
  readTouchInputs();
  
}

void handleRoot() {                          // When URI / is requested, send a web page with a button to toggle the LED

server.send(200, "text/html", "<form action=\"/login\" method=\"POST\"><input type=\"text\" name=\"username\" placeholder=\"Username\"></br><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Login\"></form>");

}
void handleLogin() {                         // If a POST request is made to URI /login
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(server.arg("username"), server.arg("password"));

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(("."));
  }
  
  server.send(200, "text/html",WiFi.localIP().toString());
  

  
 /* if(server.arg("username") == "John Doe" && server.arg("password") == "password123") { // If both the username and the password are correct
    server.send(200, "text/html", "<h1>Welcome, " + server.arg("username") + "!</h1><p>Login successful</p>");
  } else {                                                                              // Username and password don't match
    server.send(401, "text/plain", "401: Unauthorized");
  }*/
}

void handleMenu()
{
   WiFi.mode(WIFI_STA);
   server.send(200,"text/html","<form action=\"/submit\" method=\"POST\"><input type=\"checkbox\" id=\"SW1\" name=\"Switch1\" value=\"on\"><label for=\"SW1\">Switch 1</label></br><input type=\"checkbox\" id=\"SW2\" name=\"Switch2\" value=\"on\"><label for=\"SW2\">Switch 2</label></br><input type=\"checkbox\" id=\"SW3\" name=\"Switch3\" value=\"on\"><label for=\"SW3\">Switch 3</label></br><input type=\"checkbox\" id=\"SW4\" name=\"Switch4\" value=\"on\"><label for=\"SW4\">Switch 4</label><br><input type=\"checkbox\" id=\"SW5\" name=\"Switch5\" value=\"on\"><label for=\"SW5\">Switch 5</label></br><input type=\"checkbox\" id=\"SW6\" name=\"Switch6\" value=\"on\"><label for=\"SW6\">Switch 6</label></br><input type=\"submit\" value=\"Submit\"></form>");
}

void handleSubmit(){
  server.send(200,"text/plain","Done!!");  
  uint8_t gpioDigitalPresent=0x00;
  
  if(server.arg("Switch1")=="on"){
    uint8_t sw1 = 0b00000100;
    gpioDigitalPresent=(gpioDigitalPresent | sw1);
     }
  if(server.arg("Switch2")=="on"){
    uint8_t sw2 = 0b00001000;
    gpioDigitalPresent=(gpioDigitalPresent | sw2);
      }
  if(server.arg("Switch3")=="on"){
    uint8_t sw3 = 0b00010000;
    gpioDigitalPresent=(gpioDigitalPresent | sw3);
      }
  if(server.arg("Switch4")=="on"){
    uint8_t sw4 = 0b00100000;
    gpioDigitalPresent=(gpioDigitalPresent | sw4);
      }
  if(server.arg("Switch5")=="on"){
    uint8_t sw5 = 0b01000000;
    gpioDigitalPresent=(gpioDigitalPresent | sw5);
     }
  if(server.arg("Switch6")=="on"){
    uint8_t sw6 = 0b10000000; 
    gpioDigitalPresent=(gpioDigitalPresent | sw6);
      }
  gpioDigitalPresent=(gpioDigitalPresent ^ gpioPast);
  set_register(0x5A,GPIO_DATA,gpioDigitalPresent);
  gpioPast=gpioDigitalPresent;  
}
void handleStatus()
{
  if(gpioPast & 0b10000000)      
  
  
}
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

 
void readTouchInputs()
{ 
    if(!checkInterrupt()){
    Wire.requestFrom(0x5A,1);  //read the touch state from the MPR121, just one byte from ELE0 to ELE5
    
    byte LSB = Wire.read();
    
    uint8_t touched = (LSB); // 8bits that make up the touch states (0 to 5 electrodes) 
    uint8_t gpioPresent = (LSB << 2);   // right shifting for output D2 to D7 bits

    gpioPresent = (gpioPast ^ gpioPresent);  // using logical XOR for preserving past and updating present bits 0 ^ 1 == 1 and 1 ^ 1 == 0
    set_register(0x5A,GPIO_DATA,gpioPresent);
    gpioPast=gpioPresent;                   // updating the gpioPast byte
    }
}


void mpr121_setup(void){

  

  set_register(0x5A, ELE_CFG, 0x00); 
  
  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);
  
  // Section C - Sets touch and release thresholds for each electrode
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);
 
  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);
  
  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);
  
  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);
  
  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);
  
  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);
  
 
  
  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x24);
  
  // Section E
  // GPIO Configuration
  // 0x5C is in default value 0x10
  set_register(0x5A, GPIO_EN, 0xFD);
  set_register(0x5A, GPIO_DIR, 0xFD);
  set_register(0x5A, GPIO_CTRL0, 0xFD);
  set_register(0x5A, GPIO_CTRL1, 0xFD);
  
  
  // Section F
  // Enable Auto Config and auto Reconfig
  set_register(0x5A, ATO_CFG0, 0x0B);  // FFI is same as AFES in ACR
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   
  set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V
  
  set_register(0x5A, ELE_CFG, 0x86);  // Enables 6 Electrodes (0 to 5)
  Serial.println("MPR121 Setup ready");
  
  
}


boolean checkInterrupt(void){
  return digitalRead(irqpin);
}


void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}
