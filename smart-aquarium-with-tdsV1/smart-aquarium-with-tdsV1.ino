#include "CTBot.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#define LIGHT_ON_CALLBACK  "lightON"  // callback data sent when "LIGHT ON" button is pressed
#define LIGHT_OFF_CALLBACK "lightOFF" // callback data sent when "LIGHT OFF" button is pressed
#define LAMPU_ON_CALLBACK  "lampuON"
#define LAMPU_OFF_CALLBACK "lampuOFF" 
#define MONITOR_SU "MONITOR" 
#define SERV_MAKAN_CALLBACK "servomakan"
#define MON_LAMPU "MonitorLAMP"
#define MON_HEAT "MonitorHEAT"
#define dat_a "datapengambilan"


CTBot myBot;
CTBotInlineKeyboard myKbd;  // custom inline keyboard object helper

String ssid = "";     // REPLACE mySSID WITH YOUR WIFI SSID
String pass = ""; // REPLACE myPassword YOUR WIFI PASSWORD, IF ANY
String token = "";   // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
uint8_t led = 14;              
const int oneWireBus = 13;   
const int heat = 27; 
int servoPin = 26;
int lampu_status;
int heater_status;
bool hasResponded = false;
int tdsstat;
Servo servol;
float Vref = 3.3;
float ec = 0;
unsigned int tds = 0;
float t = 0;
float ecCalibration = 1;
const byte tds_pin = A0;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai Smart Aquarium TelegramBot...");
  lcd.begin();       
  lcd.backlight(); 
  Serial.println("Starting TelegramBot...");
  lcd.setCursor(0,0);
  lcd.print("  Memulai  bot  ");
  lcd.setCursor(0,1);
  lcd.print("    Telegram    ");
  delay(800);
  lcd.clear();
  myBot.wifiConnect(ssid, pass);
  Serial.println("Menghubungkan WiFi ke SSID");
  lcd.setCursor(0,0);
  lcd.print(" Menghubungkan ");
  lcd.setCursor(0,1);
  lcd.print("      WiFi      ");
  delay(800);
  lcd.clear();
  myBot.setTelegramToken(token);
  Serial.println("Menghubungkan ke telegram");
  lcd.setCursor(0,0);
  lcd.println("  Menghubungkan ");
  lcd.setCursor(0,1);
  lcd.print("  Telegram bot.  ");
  delay(800);
  lcd.clear();
  if (myBot.testConnection())
    Serial.println("\nTerhubung ke bot Telegram");
  else
    Serial.println("\nTidak Terhubung silahkan cek koneksi internet");

  // set the pin connected to the LED to act as output pin
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  lampu_status = 0;
  pinMode(heat, OUTPUT);
  digitalWrite(heat, LOW);
  heater_status = 0;
  tdsstat=0;
  sensors.begin();
  servol.attach(servoPin);
  servol.write(0);

  // inline keyboard customization
  // add a query button to the first row of the inline keyboard
  myKbd.addButton("Hidupkan lampu", LAMPU_ON_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Matikan lampu", LAMPU_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Beri pakan", SERV_MAKAN_CALLBACK, CTBotKeyboardButtonQuery);  
  myKbd.addRow();
  myKbd.addButton("Monitor Parameter Aquarium", MONITOR_SU, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Status lampu", MON_LAMPU, CTBotKeyboardButtonQuery);
  myKbd.addButton("Status Heater", MON_HEAT, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Ambil Data", dat_a, CTBotKeyboardButtonQuery);
}

void loop() {
  readTdsQuick();
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    // check what kind of message I received
    if (msg.messageType == CTBotMessageText) {
      // received a text message
      if (msg.text.equalsIgnoreCase("/start")) {
        myBot.sendMessage(msg.sender.id, "Perintah yang tersedia :", myKbd);
      }
      else {
        myBot.sendMessage(msg.sender.id, "Selamat datang, untuk menampilkan perintah tekan --> /start");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      // received a callback query message
      if (msg.callbackQueryData.equals(LAMPU_ON_CALLBACK)) {
        lampu_status = 1;
        myBot.endQuery(msg.callbackQueryID, "Lampu telah dihidupkan", true);
      } 
      else if (msg.callbackQueryData.equals(LAMPU_OFF_CALLBACK)) {
        lampu_status = 0;
        myBot.endQuery(msg.callbackQueryID, "Lampu telah dimatikan", true);
      } 
      else if (msg.callbackQueryData.equals(MON_LAMPU)) {
         if (lampu_status)
          myBot.endQuery(msg.callbackQueryID, "Lampu hidup", true);
        else 
          myBot.endQuery(msg.callbackQueryID, "Lampu mati", true);
      } 
      else if (msg.callbackQueryData.equals(MON_HEAT)) {
        if (heater_status)
          myBot.endQuery(msg.callbackQueryID, "Heater hidup", true);
        else 
          myBot.endQuery(msg.callbackQueryID, "Heater mati", true);         
      } 
      else if (msg.callbackQueryData.equals(MONITOR_SU)) {
        myBot.sendMessage(msg.sender.id, (String)"Kondisi saat ini\nSuhu : " + t + " °C\nTDS : " + tds + " ppm\nEC : " + ec + " µS/cm");
        myBot.endQuery(msg.callbackQueryID, "Hasil terbaca di chat", true);
      }
      else if (msg.callbackQueryData.equals(SERV_MAKAN_CALLBACK)) {
        servol.write(90);
        delay(1000);
        servol.write(0);
        myBot.endQuery(msg.callbackQueryID, "Makanan diberi!.", true);
      }
      else if (msg.callbackQueryData.equals(dat_a)) {
        String heaterStatusString = (heater_status == 1) ? "ON" : "OFF";
        String lampuStatusString = (lampu_status == 1) ? "ON" : "OFF";
        String message = String(t) + " " + String(tds) + " " + String(ec) + " " + heaterStatusString + " " + lampuStatusString;
        myBot.sendMessage(x, message);
      }
    }
    digitalWrite(led, lampu_status);
    digitalWrite(heat, heater_status);
  }
  if (t > 31) {
    heaterOff();
    Serial.print("Suhu : ");
    Serial.print(t);
    Serial.println(" °C ");
    Serial.println("Suhu Panas mematikan Heater");
    } 
  else if (t < 22) {
    heaterOn();
    Serial.print("Suhu : ");
    Serial.print(t);
    Serial.println(" °C ");
    Serial.println("Suhu Dingin menghidupkan Heater");
  } 
  else if (tds < 300 ){
    tdsstat=0;
  }
  else if (tds > 300 && tdsstat < 1) {
    Serial.println("TDS tinggi segera mengganti air aquarium");
    tdsstat=1;
    myBot.sendMessage(msg.sender.id, (String)"TDS Tinggi\nTDS : " + tds + " ppm\nJumlah Zat Padat yang Terlarut dalam aquarium sangat tinggi, Segeralah mengganti air aquarium" );
  }
  if (!hasResponded) {
    if (t < 22) {
      myBot.sendMessage(msg.sender.id, "Suhu Dingin!\nSuhu : " + String(t) + " °C\nHeater dihidupkan.");
    } else if (t > 31) {
      myBot.sendMessage(msg.sender.id, "Suhu Terlalu Panas!\nSuhu : " + String(t) + " °C\nHeater dimatikan.");
    } 
      hasResponded = true;
    }
  delay(1000);
}
void readTdsQuick() {
  sensors.requestTemperatures();
  t = sensors.getTempCByIndex(0);
  float rawEc = analogRead(tds_pin) * Vref / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float temperatureCoefficient = 1.0 + 0.02 * (t - 25.0); // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  ec = (rawEc / temperatureCoefficient) * ecCalibration; // temperature and calibration compensation
  tds = (133.42 * pow(ec, 3) - 255.86 * ec * ec + 857.39 * ec) * 0.5; //convert voltage value to tds value
  Serial.println("--------------------");
  Serial.print("Suhu : "); 
  Serial.print(t);
  Serial.println(" °C ");
  Serial.print("TDS  : "); 
  Serial.print(tds);
  Serial.println(" ppm ");
  Serial.print("EC   : "); 
  Serial.print(ec);
  Serial.println(" µS/cm ");
  Serial.println("--------------------");
  lcd.setCursor(0,0);     
  lcd.print("Suhu : ");
  lcd.print(t);   
  lcd.print(" ");          
  lcd.print(char(223));
  lcd.print("C");
  lcd.setCursor(0,1); 
  lcd.print("TDS  : ");
  lcd.print(tds);           
  lcd.print(" ppm");
}
void heaterOn() {
  if (!heater_status) {
    digitalWrite(heat, HIGH);
    heater_status = 1;
    hasResponded = false; // Reset status respons
    }
  }
void heaterOff() {
  if (heater_status) {
    digitalWrite(heat, LOW);
    heater_status = 0;
    hasResponded = false; // Reset status respons
    }
  }
