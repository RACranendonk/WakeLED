/*==================
Robert Cranendonk
WakeLED. Personal Arduino project
21-02-2014
====================*/

#include <PinWriter.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <EthernetUdp.h>
#include <HexConv.h>
#define REDPIN 5
#define GREENPIN 6
#define BLUEPIN 9

byte mac[] = { 
  0x6C, 0xF0, 0x49, 0xB8, 0x37, 0x06 };
IPAddress ip(192,168,178,85);
unsigned int localPort=8888;
IPAddress timeServer(213, 84, 134, 226);
const int PACKET_SIZE= 48;

char packetBuffer[PACKET_SIZE];
EthernetUDP Udp;

AlarmID_t alarmID;
String command = "";
uint8_t decRed = 0;
uint8_t decGreen = 0;
uint8_t decBlue = 0;

PinWriter pinWriter(REDPIN, GREENPIN, BLUEPIN);

void setup() {
  //bitSet(TCCR1B, WGM12);  //might be needed if light flickers, SAME AS BELOW LINE
  TCCR1B |= (1 << WGM12); //SAME AS ABOVE LINE
  
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  
  ntp_client();
  
  Alarm.timerRepeat(86400,resync);  //resync clock every 24h
  
}   

void loop() {
  command = udpFunction();
  
  if(command.indexOf('@') != -1){
    command = command.substring(command.indexOf('@'));
    
    if(command.startsWith("@on"))
        lightOn(command);
    if(command.startsWith("@off"))
        lightOff();
    if(command.startsWith("@set"))
        alarmSet(command);
    if(command.startsWith("@clear"))
        alarmClear();
    if(command.startsWith("@timerequest")){}
  }
  digitalClockDisplay();  //DO NOT REMOVE
  Alarm.delay(1000);  //DO NOT REMOVE
}

void lightOn(String commandIn){
  String tag = "&#ff";
  String hex = commandIn.substring(commandIn.indexOf(tag) + tag.length());

  decRed = hexToDec(hex.substring(0, 2));
  decGreen = hexToDec(hex.substring(2, 4));
  decBlue = hexToDec(hex.substring(4));

  pinWriter.write(decRed, decGreen, decBlue);
}

void lightOff(){
  decRed = 0;
  decGreen = 0;
  decBlue = 0;
  pinWriter.write(decRed, decGreen, decBlue);
}


void alarmSet(String commandIn){
  alarmClear();
  tmElements_t alarmElements;  //time elements cache
  String hourTag = "&h";
  String minuteTag = "&m";

  alarmElements.Year = year() - 1970; 
  alarmElements.Month = month();
  alarmElements.Day = day();
  alarmElements.Hour = atoi((commandIn.substring(commandIn.indexOf(hourTag) + hourTag.length(), commandIn.indexOf(hourTag) + hourTag.length() + 2)).c_str());
  alarmElements.Minute = atoi((commandIn.substring(commandIn.indexOf(minuteTag) + minuteTag.length())).c_str());
  alarmElements.Second = 0;

  time_t alarmTime = makeTime(alarmElements);
  
  if(alarmTime <= now()) //if the alarm is set earlier than current time, it's probably meant for the next day
    alarmTime += 86400;
    
  alarmTime -= 900;  //15 minutes previous
  
  if(alarmTime <= now()){ //if the alarm is set to go off in a timeframe of less than 15 minutes, start now
    alarm();
    return;
  }
  
  Serial.println("Alarm will start at:");
    Serial.print(hour(alarmTime));
    printDigits(minute(alarmTime));
    printDigits(second(alarmTime));
    Serial.print(" ");
    Serial.print(year(alarmTime));
    Serial.print("-");
    Serial.print(month(alarmTime));
    Serial.print("-");  
    Serial.println(day(alarmTime));
    Serial.println();

  alarmID = Alarm.alarmOnce(hour(alarmTime), minute(alarmTime), 0, alarm);
}

void alarmClear(){
    Alarm.disable(alarmID);
    confirmFlash();
}

void confirmFlash(){
    pinWriter.write(0, 0, 0);
    Alarm.delay(150);
    pinWriter.write(0, 50, 0);
    Alarm.delay(150);
    pinWriter.write(0, 0, 0);
    Alarm.delay(150);
    pinWriter.write(decRed, decGreen, decBlue);
}

void alarm(){
  Serial.println("Alarm started!");
  
  uint16_t steps = 1800;
  
  uint8_t beginColors[3] = {0, 0, 0};
  float currentColors[3] = {0, 0, 0};
  uint8_t targetColors[6][3] = {{15, 0, 0},  //red
                          {25, 0, 2},  //purple
                          {10, 0, 65},  //blue
                          {35, 15, 0},  //stepping up to
                          {120, 65, 0},  //yellow
                          {255, 255, 255}};  //white
  uint16_t loopSteps[6] = {(steps*0.2), (steps*0.20), (steps*0.2), (steps*0.1), (steps*0.2), (steps*0.1)};  //Sum of multiplier steps: 100%

   for(int i = 0; i < 6; i++){
    Alarm.delay(10);
    if(udpFunction().startsWith("@end")){
      alarmClear();
      break;
    }
     
    Serial.println("\n\n new loop");
    Serial.println("Target colors:");
    Serial.print("red: ");
    Serial.print(targetColors[i][0]);
    Serial.print(" green: ");
    Serial.print(targetColors[i][1]);
    Serial.print(" blue: ");
    Serial.print(targetColors[i][2]);
    Serial.print("\nLoopsteps: ");
    Serial.println(loopSteps[i]);
        
     for(int j = 0; j < loopSteps[i]; j++){      
       pinWriter.write((currentColors[0] += (((float)(targetColors[i][0] - beginColors[0])/loopSteps[i]))), 
                       (currentColors[1] += (((float)(targetColors[i][1] - beginColors[1])/loopSteps[i]))), 
                       (currentColors[2] += (((float)(targetColors[i][2] - beginColors[2])/loopSteps[i]))));
        Alarm.delay(500);
        
        Serial.print("\nRed ");
        Serial.println(currentColors[0]);    
        Serial.print("Green ");
        Serial.println(currentColors[1]);
        Serial.print("Blue ");
        Serial.println(currentColors[2]); 

    }
    beginColors[0] = (uint8_t)(currentColors[0] + 0.5);
    beginColors[1] = (uint8_t)(currentColors[1] + 0.5);
    beginColors[2] = (uint8_t)(currentColors[2] + 0.5);
    
    Serial.println("\nbeginColors for the next loop:\n");
        Serial.print("\nRed ");
        Serial.println(beginColors[0]);    
        Serial.print("Green ");
        Serial.println(beginColors[1]);
        Serial.print("Blue ");
        Serial.println(beginColors[2]);
   }
   decRed = beginColors[0];
   decGreen = beginColors[1];
   decBlue = beginColors[2];
   
   Serial.println("Alarm over");
}

void digitalClockDisplay()
{
  //digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}

void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

String udpFunction(){
  String str = "";
  int packetSize = Udp.parsePacket();
  if(packetSize){
    memset(packetBuffer, 0, PACKET_SIZE); 
    Udp.read(packetBuffer, PACKET_SIZE);
    str += packetBuffer;
  }
  return str;
}

void resync(){
  ntp_client();
}

unsigned long ntp_client(){
  int hourSet, minuteSet, secondSet;
  sendNTPpacket(timeServer);
  Alarm.delay(1000);  
  if ( Udp.parsePacket() ) {  
    Udp.read(packetBuffer,PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    const unsigned long seventyYears = 2208988800UL;     
    unsigned long epoch = secsSince1900 - seventyYears;  
    setTime(epoch);
    adjustTime(adjustDstEurope());
    Serial.print("The UTC time is ");
    digitalClockDisplay();
    Serial.print(" ");
    Serial.print(year());
    Serial.print("-");
    Serial.print(month());
    Serial.print("-");  
    Serial.println(day());
  }
}

int adjustDstEurope(){
  // last sunday of march
  int beginDSTDate=  (31 - (5* year() /4 + 4) % 7);
  //Serial.println(beginDSTDate);
  int beginDSTMonth=3;
  //last sunday of october
  int endDSTDate= (31 - (5 * year() /4 + 1) % 7);
  //Serial.println(endDSTDate);
  int endDSTMonth=10;
  // DST is valid as:
  if (((month() > beginDSTMonth) && (month() < endDSTMonth))
      || ((month() == beginDSTMonth) && (day() >= beginDSTDate)) 
      || ((month() == endDSTMonth) && (day() <= endDSTDate)))
  return 7200;  // DST europe = utc +2 hour
  else return 3600; // nonDST europe = utc +1 hour
}

unsigned long sendNTPpacket(IPAddress& address){
  memset(packetBuffer, 0, PACKET_SIZE); 
  packetBuffer[0] = 0b11100011; 
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;  
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer,PACKET_SIZE);
  Udp.endPacket(); 
}
