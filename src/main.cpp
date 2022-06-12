#include <PZEM004Tv30.h>
#include "TFT_eSPI.h"
#include "RTC_SAMD51.h"
#include "DateTime.h"
#include <Free_Fonts.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#include <SPI.h>
//#include "seeed_line_chart.h"

#define uart Serial1

int page =1; //Khai bao so trang hien thi
int dem;

int i = 0; // la bien dem mac dinh cho thu tu chu so trong password
int k = 0;
int pwd[3] = {1,2,3}; // khai bao mat khau
int currentpwd[3] = {0,0,0};
int checkpwd = 0;

char dataStr[100] = "";
char buffer[7];


int yea, mon, da, hou, minu, sec;// bien hien thi thoi gian
float a,b,s; // Khai bao bien do nang luong
float v,c,p,e,f,pf; // Khai bao bien do cua Pzem
float v_SD = 0,c_SD = 0,p_SD = 0,f_SD = 0,pf_SD = 0, m = 0; // khai bao bien luu gia tri do duoc vao the nho

unsigned long startMillisPZEM;
unsigned long currentMillisPZEM;
const unsigned long delayPZEM = 1000;

unsigned long startMillisTimer;
unsigned long currentMillisTimer;
const unsigned long delayTimer = 1000;

unsigned long startMillisSD;
unsigned long currentMillisSD;
const unsigned long delaySD = 1000;

#define max_size 50 //maximum size of data
double data; //Initilising a doubles type to store data

File myFile;


TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

PZEM004Tv30 pzem;

RTC_SAMD51 rtc;

void readDataPZEM();
void Screen1();
void Screen2();
void Screen3();
void Screen4();
void Screen5();
void getTime();
void checkPassword();
void saveSdCard();

void setup(){
  tft.begin();
  tft.setRotation(3);
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
  
  
  pinMode(WIO_KEY_B,    INPUT_PULLUP);
  pinMode(WIO_KEY_C,    INPUT_PULLUP);
  pinMode(WIO_BUZZER,   OUTPUT);
  pinMode(WIO_5S_DOWN,  INPUT);
  pinMode(WIO_5S_UP,    INPUT);
  pinMode(WIO_5S_RIGHT, INPUT);
  pinMode(WIO_5S_LEFT,  INPUT);
  pinMode(WIO_5S_PRESS, INPUT);


  rtc.begin();
  DateTime now = DateTime(F(__DATE__), F(__TIME__));
  rtc.adjust(now);

  pzem.begin(&uart);
  pzem.setAddress(0x42);

  startMillisPZEM = millis();
  startMillisTimer = millis();

  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) 
  {
      Serial.println("initialization failed!");
      while(1);
  }
  
  // clear out old data file
  if(SD.exists("DataLogger.csv"))
  {
    Serial.println("Removing simple .csv");
    SD.remove("DataLogger.csv");
    Serial.println("Done");
  }
  
  //write csv headers to file
  myFile = SD.open("DataLogger.csv", FILE_APPEND); 
  if(myFile)
  {
    Serial.println("Writing headers to DataLogger.csv");
    myFile.println("Ngay,Thoi gian,Dien ap,Dong dien,Cong suat,Nang luong,Tan so,He so cong suat");
    myFile.close();
    Serial.println("Headers written");
  }
  else
  {
    Serial.println("Error opening DataLogger.csv");
  }

 
}

void loop(){
  spr.fillRect(0,0,320,240,TFT_WHITE);

  readDataPZEM();
  
  if(digitalRead(WIO_KEY_C) == LOW){
    page = page + 1; 
    if(page == 5){
      page = 1;
    }
    delay(500);
  }
  
 if(page == 1){
  Screen1();
  getTime();    
 }
 if(page == 2){
  Screen2();  
  getTime();
 }
 if(page == 3){
   Screen3();
   getTime();
 }
 if(page == 4){
   Screen4();
   getTime();
 }
//  if(page == 5){
//    Screen5();
//  }

 //alarmPower();
 spr.pushSprite(0,0);
}

void readDataPZEM(){
  currentMillisPZEM = millis();
  if(currentMillisPZEM - startMillisPZEM >= delayPZEM){
    v  = pzem.voltage();
    c  = pzem.current();
    p  = pzem.power();
    e  = pzem.energy();
    f  = pzem.frequency();
    pf = pzem.pf();
    m++;
    if(m == 15) { saveSdCard(); v_SD = 0; c_SD = 0; p_SD = 0; f_SD = 0; pf_SD = 0; m = 0;}
    v_SD += v;
    c_SD += c;
    p_SD += p;
    f_SD += f;
    pf_SD += pf;
    startMillisPZEM = currentMillisPZEM;
  }
}

void Screen1(){
  spr.setTextColor(TFT_BLACK);
  spr.setTextSize(1);
  spr.drawFloat(p,1,90,45,7);
  spr.drawFloat(e,3,80,120,7);
  spr.setTextSize(2);
  spr.drawString("W",280,65);
  spr.drawString("kWh",280,140);
  spr.drawCentreString("Tien dien: ",45,200,1);
  if(e<50){spr.drawFloat(e*1678,3,130,200);}
  if((e>=50)&&(e<100)){spr.drawFloat((e-50)*1734 + 83900,3,130,200);}
  if((e>=100)&&(e<200)){spr.drawFloat((e-100)*2014 + 257300,3,130,200);}
  if((e>=200)&&(e<300)){spr.drawFloat((e-200)*2536 + 458700,3,130,200);}
  if((e>=300)&&(e<400)){spr.drawFloat((e-300)*2834 + 712300,3,130,200);}
  if(e>+400){spr.drawFloat((e-400)*2927 + 995700,3,130,200);}
}

void Screen2(){
  spr.setTextColor(TFT_BLACK);
  spr.setTextSize(1);
  spr.drawFloat(v,1,180-spr.textWidth("000.0",7),45,7);
  spr.drawFloat(c,3,180-spr.textWidth("0.000",7),150,7);
  spr.drawFloat(f,1,200,45,7);
  spr.drawFloat(pf,2,200,150,7);
  spr.setTextSize(2);
  spr.drawString("V",165,105);
  spr.drawString("A",165,205);
  spr.drawString("Hz",290,105);
  spr.drawString("PF",290,205);
   
}

void Screen3(){
    spr.setTextColor(TFT_BLACK);
    spr.setTextSize(2);
    spr.drawString("1.Start",15,50);
    spr.drawString("2. Energy",15,100);
    spr.drawString("3. Used Energy",15,150);
    spr.drawCentreString("STOP WATCH",160,210,1);
    
    if(digitalRead(WIO_KEY_B) == LOW){
      delay(500);
      dem = dem + 1;
    }
    if(dem == 5){
      dem = 0;
      a = 0;
      s = 0;
    }
    if(dem == 1){
      a = e;
      dem = dem + 1;
    }
    if(dem == 3){
       b = e;
       s = b - a;
       dem = dem + 1;
    }
    
    spr.setTextSize(3);
    spr.setTextColor(TFT_BLACK);
    spr.drawFloat(a,3,50,70);
    spr.drawFloat(e,3,50,120);
    spr.drawFloat(s,3,50,170);
}

void Screen4(){
    //spr.setTextSize(2);
    spr.setTextColor(TFT_BLACK);

    // ve cac hinh tam giac
    spr.drawTriangle(50,110,75,85,100,110,TFT_RED);
    spr.drawTriangle(135,110,160,85,185,110,TFT_RED);
    spr.drawTriangle(220,110,245,85,270,110,TFT_RED);
    spr.drawTriangle(50,180,75,205,100,180,TFT_RED);
    spr.drawTriangle(135,180,160,205,185,180,TFT_RED);
    spr.drawTriangle(220,180,245,205,270,180,TFT_RED);

    // kiem tra nut phai co bam hay khong?
    if(digitalRead(WIO_5S_RIGHT) == LOW)
    {
      delay(200);
      i++;
      if(i == 3)
      {
        i = 0;
      }
    }
// Kiem tra nut trai co bam hay khong
    if(digitalRead(WIO_5S_LEFT) == LOW)
    {
      delay(200);
      i--;
      if(i < 0)
      {
        i = 2;
      }
    }
// kiem tra nut len co bam hay khong
    if(digitalRead(WIO_5S_UP) == LOW)
    {
      delay(200);
      currentpwd[i] += + 1;
      if(currentpwd[i] == 10)
      {
        currentpwd[i] = 0;
      }
    }
// kiem tra nut xuong co bam hay khong
    if(digitalRead(WIO_5S_DOWN) == LOW)
    {
      delay(200);
      currentpwd[i] = currentpwd[i] - 1;
      if(currentpwd[i] < 0 )
       {
        currentpwd[i] = 9;
      }
    }
// kiem tra nut giua co bam hay khong
    if(digitalRead(WIO_5S_PRESS) == LOW)
    {
      delay(200);
      checkPassword();
    }

//    if(digitalRead(WIO_KEY_B) == LOW)
//    {
//      delay(200);
//      spr.setTextSize(3);
//      spr.setTextColor(TFT_BLACK);
//      spr.drawCentreString("Setup Password", 160, 60,1);
//
//    }

// to mau cho mui ten o 1 chu so cua password
    switch (i)
    {
    case 0:
      spr.fillTriangle(50,110,75,85,100,110,TFT_RED);
      spr.fillTriangle(50,180,75,205,100,180,TFT_RED);
      break;
    case 1:
      spr.fillTriangle(135,110,160,85,185,110,TFT_RED);
      spr.fillTriangle(135,180,160,205,185,180,TFT_RED);
      break;
    case 2:
      spr.fillTriangle(220,110,245,85,270,110,TFT_RED);
      spr.fillTriangle(220,180,245,205,270,180,TFT_RED);
      break;
    }

    //hien thi test_password 
    spr.setTextSize(1);
    spr.setTextColor(TFT_BLACK);
    spr.drawString(String(currentpwd[0]),60,120,7);
    spr.drawString(String(currentpwd[1]),145,120,7);
    spr.drawString(String(currentpwd[2]),225,120,7);


    // spr.drawCentreString("PRESS BUTTON B TO",160,85,1);
    // spr.drawCentreString("RESET ENERGY",160,135,1);
    // spr.drawCentreString("kWh",160,170,1);
    spr.setTextSize(3);
    spr.drawFloat(e,3,120,50);
    //Reset Energy
//    if(digitalRead(WIO_KEY_B) == LOW){
//      delay(20);
//      pzem.resetEnergy();
//      
//    }
}

void Screen5(){
  spr.fillSprite(TFT_WHITE);
//   if (data.size() == max_size) {
//       data.pop();//this is used to remove the first read variable
//   }
//   data.push(double(c)); //read variables and store in data

//   //Settings for the line graph title
//       auto header =  text(0, 0)
//                 .value("Dong dien")
//                 .align(center)
//                 .valign(vcenter)
//                 .width(tft.width())
//                 .thickness(3);
 
//     header.height(header.font_height() * 2);
//     header.draw(); //Header height is the twice the height of the font

// //Settings for the line graph

//       auto content = line_chart(20, header.height()); //(x,y) where the line graph begins
//          content
//                 .height(tft.height() - header.height() * 1.5) //actual height of the line chart
//                 .width(tft.width() - content.x() * 2) //actual width of the line chart
//                 .based_on(0.0) //Starting point of y-axis, must be a float
//                 .show_circle(false) //drawing a cirle at each point, default is on.
//                 .value(data) //passing through the data to line graph
//                 .color(TFT_PURPLE) //Setting the color for the line
//                 .draw();

//   spr.pushSprite(0, 0);
  delay(50);
}

void getTime(){

  currentMillisTimer = millis();
  if(currentMillisTimer - startMillisTimer >= delayTimer){
    DateTime now = rtc.now();
    yea = now.year();
    mon = now.month();
    da  = now.day();
    hou =  now.hour();
    minu= now.minute();
    sec = now.second();
    startMillisTimer = currentMillisTimer;
  }
  String time;
  if (hou <10)
  {
    time = "0" + String(hou) + ":";
  }
  else time = String(hou) + ":";
  if(minu < 10)
  {
    time = time + "0" + String(minu) + ":" + String(sec);
  }
  else time = time + String(minu) + ":" + String(sec);
    spr.setTextSize(2);
    spr.setTextColor(TFT_RED);
    spr.drawCentreString(time, 160, 20,1);
}

void checkPassword()
{
  for (int j = 0; j < 3; j++)
  {
    if(currentpwd[i] == pwd[i])
    {
      checkpwd += 1;
    }
  }
  if(checkpwd == 3)
  {
    spr.setTextSize(3);
    spr.setTextColor(TFT_BLUE);
    spr.drawCentreString("Right Password", 160, 225,1);
    pzem.resetEnergy();
    
  }
  else 
  {
    spr.setTextSize(3);
    spr.setTextColor(TFT_RED);
    spr.drawCentreString("Wrong Password", 160, 225,1);
  }
  checkpwd = 0;
  
}

void saveSdCard()
{
  v_SD = v_SD/15; c_SD = c_SD/15; p_SD = p_SD/15; f_SD = f_SD/15; pf_SD = pf_SD/15;
  dataStr[0] = 0;
  itoa(da, buffer,10);
  strcat(dataStr, buffer);
  strcat(dataStr, "/");
  
  itoa(mon, buffer,10);
  strcat(dataStr, buffer);
  strcat(dataStr, "/");

  itoa(yea, buffer,10);
  strcat(dataStr, buffer);

  strcat(dataStr,",");

  itoa(hou, buffer,10);
  strcat(dataStr, buffer);
  strcat(dataStr, ":");

  itoa(minu, buffer,10);
  strcat(dataStr, buffer);
  strcat(dataStr, ":");

  itoa(sec, buffer,10);
  strcat(dataStr, buffer);

  strcat(dataStr, ",");

  dtostrf(v_SD, 5, 1, buffer);
  strcat(dataStr, buffer);
  strcat(dataStr, ",");

  dtostrf(c_SD, 6, 3, buffer);
  strcat(dataStr, buffer);
  strcat(dataStr, ",");

  dtostrf(p_SD, 5, 1, buffer);
  strcat(dataStr, buffer);
  strcat(dataStr, ",");

  dtostrf(e, 6, 3, buffer);
  strcat(dataStr, buffer);
  strcat(dataStr, ",");

  dtostrf(f_SD, 5, 1, buffer);
  strcat(dataStr, buffer);
  strcat(dataStr, ",");

  dtostrf(pf_SD, 5, 2, buffer);
  strcat(dataStr, buffer);
  strcat(dataStr, ",");
  
  myFile = SD.open("DataLogger.csv",FILE_APPEND);
  Serial.println(dataStr);
  /*myFile.print(a);
  myFile.print(",");
  myFile.print(a);
  myFile.println();*/
  myFile.println(dataStr);
  myFile.close();

}
/*
void alarmPower(){

  if(p >= 0.010){
    for(k=0;k<10;k++){
      analogWrite(WIO_BUZZER, 128);
      delay(1000);
      analogWrite(WIO_BUZZER, 0);
      delay(1000);
    }
  }
}
*/