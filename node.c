#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Servo.h> // including servo library.

const char* ssid = "TPLink";   // your network SSID (name) 
const char* password = "Excitel@103";   // your network password

const int BUZZER = D7;
const int rs = D0, en = D2, d4 = D3, d5 = D4, d6 = D5, d7 = D6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define BOTtoken "5862893780:AAFxzzRvycRqO_RI_xpzi02WAfj6xKW48VQ"
//#define BOTtoken "519073934:AAHD2MNwbqRg5m-pjDl6COl07zRL6n2Vy9E"

#define CHAT_ID "1300329526"
//#define CHAT_ID "599690730"

Servo servo_1; // Giving name to servo

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

WiFiServer server(80);
struct tm * timeinfo1;

char input[12];
int count = 0;
int setHH=0,setMM=0,setSS=0;

unsigned long bot_lasttime;          // last time messages' scan has been done
bool Start = false;

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String text1;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/start")
    {
      String welcome = "Welcome to MediMate - your personalized medication management assistant.\n";

      bot.sendMessage(chat_id, welcome);
    }

    text1 = text.substring(0,9);
    Serial.println(text1);
    if (text1 == "/setalarm")
    {
      long int val=0;
      String text2;
      text2 = text.substring(10,16);
      Serial.print("text2: ");
      Serial.println(text2);
      val = text2.toInt();
      setHH = val/100;
      setMM = val%100;
      
      Serial.print("HH: ");
      Serial.println(setHH);
      Serial.print("MM: ");
      Serial.println(setMM);
      bot.sendMessage(chat_id, "Alarm Value Set");
    }
    
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(BUZZER,OUTPUT);
  servo_1.attach(5); // Attaching Servo to D1 or GPIO5
  
  configTime(5.5*3600, 0, "pool.ntp.org");      // get UTC time via NTP
  printLocalTime();
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org  

  lcd.setCursor(0, 0);
  lcd.print("MediMate: Caring");
  lcd.setCursor(0, 1);
  lcd.print("  Pill Partner  ");

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  
  // Print the IP address
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Application Started", "");  
  delay(4000);
  lcd.clear();
  servo_1.write(0);  
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print(" MediMate: Caring   ");
  lcd.setCursor(0, 1);
  lcd.print(" * Pill Partner * ");

  if(setHH!=0 && setMM!=0)
  {
    getLocalTime();
    Serial.print("setHH: ");
    Serial.print(setHH);
    Serial.print("setMM: ");
    Serial.println(setMM);
    
    Serial.println(timeinfo1->tm_hour);
    Serial.println(timeinfo1->tm_min);

    
    if((timeinfo1->tm_hour == setHH) && (timeinfo1->tm_min == setMM) && (timeinfo1->tm_sec < 5))
    {
      Serial.println("Alarm Set");
      digitalWrite(BUZZER,1);
      bot.sendMessage(CHAT_ID, "Alarm: Time to take the medicine", "");      
      delay(1000);
      digitalWrite(BUZZER,0);      
    }
  }
  
  // i wanna change it to 30 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(analogRead(A0) > 50)
  {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  WARNING!!!!   ");
      lcd.setCursor(0, 1);
      lcd.print("DISTRESS ALERT !");
      digitalWrite(BUZZER,1);
      bot.sendMessage(CHAT_ID, "WARNING! Distress Alert", "");      
      delay(1000);
      digitalWrite(BUZZER,0);
      lcd.clear();
  }

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    Serial.println("checking for new messages");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    Serial.println("number of new messages");
    Serial.println(numNewMessages);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

  if (Serial.available())
  {
    count = 0;
    while (Serial.available() && count < 12)
    {
      input[count] = Serial.read();
      count++;
      delay(5);
    }
    if (count == 12)
    {
    printLocalTime();
    digitalWrite(BUZZER,1);
    delay(750);  
    digitalWrite(BUZZER,0);
        
   Serial.println(input);
    if ((strncmp(input, "08007C456352", 12) == 0))
    {
      Serial.println("CareTaker");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Medicine changed");
      lcd.setCursor(0, 1);
      lcd.print("  by caretaker  ");
      bot.sendMessage(CHAT_ID, "Medicine has been changed", "");  

      servo_1.write (180); // Servo will move to 45 degree angle.
      delay (3000);
      servo_1.write (0);  // servo will move to 90 degree angle.      
      delay (1000);
    }
    else if ((strncmp(input, "4600155E1A17", 12) == 0))
    {
      Serial.println("Patient");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   Pills taken   ");
      lcd.setCursor(0, 1);
      lcd.print("   by Patient    ");      
      bot.sendMessage(CHAT_ID, "Pills taken by patient", "");  

      servo_1.write (180); // Servo will move to 45 degree angle.
      delay (3000);
      servo_1.write (0);  // servo will move to 90 degree angle.      
      delay (1000);
    }
    memset(input,0,sizeof(input));
    delay(500);
    lcd.clear();
    }
  }
}

void printLocalTime()
{
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  Serial.println(asctime(timeinfo));
  Serial.println(timeinfo->tm_hour);
  Serial.println(timeinfo->tm_min);
  Serial.println(timeinfo->tm_sec);
  
  delay(100);
}

void getLocalTime()
{
  time_t rawtime;
  time (&rawtime);
  timeinfo1 = localtime (&rawtime);  
  delay(100);
}
