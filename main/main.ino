/*
PONTIFICIA UNIVERSIDADE CATOLICA DE MINAS GERAIS - PUCMG
ICEI (INSTITUTO DE CIENCIAS EXATAS E INFORMATICA) - ENGENHARIA DE COMPUTAÇÃO
SISTEMAS EMBARCADOS

LEMBROL 2.0
*/

//INCLUSÃO DE BIBLIOTECA
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>      
#include <MFRC522.h>  
#include <TimeLib.h> 

// PINOS PARA ARDUINO
// #define SS_PIN 10 //PINO SDA
// #define RST_PIN 9 //PINO DE RESET

// PINOS PARA MIFARE ESP32 | VINI
//#define SS_PIN  21  // ESP32 PINO SDA
//#define RST_PIN 22 // ESP32 PINO RESET

// PINOS PARA MIFARE ESP32 | GABI
#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);  //PASSAGEM DE PARÂMETROS REFERENTE AOS PINOS
LiquidCrystal_I2C lcd(0x27, 16, 2);
// LiquidCrystal lcd(26, 25, 33, 17, 16, 15);

// RELAÇÃO VINI -- > LiquidCrystal lcd(26, 25, 33, 17, 16, 15);
// RELAÇÃO ORIGINAL --> LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

#define up 33
#define down 26
#define enter 32
#define back 25
#define buzzer 27

#define tempo_deBounce 800
static unsigned int tempo = 0;

unsigned int menu_sub = 0;
unsigned int menu_week = 0;
unsigned int menu_current = 0;
unsigned int menu_tags = 0;

int tagsRead = 0;
int buzzerCount = 0;

bool buzzActive = false;
bool setDay = false;

String strID = "";
String currentTag = "";

String azul = "F7:95:0C:E1";
String branco = "57:9F:2F:E1";
String rosa = "47:CE:30:E1";


typedef struct day {
  String dayList  = {""};
  int tags[3] = {0,0,0};
  int totalTags = 0; 
} day;

day Current;
day Monday;
day Tuesday;
day Wednesday;
day Thursday;
day Friday;
day Saturday;
day Sunday;

void readBadge();
void select_menu();
void select_menu_week();
void select_tags();
void select_buttons();
void checkTagsByDay();
void verifyDay();
void showTagsNotFound();
void reset();

void setup() {
  Serial.begin(9600);  //INICIALIZA A SERIAL

  lcd.init();
  lcd.clear();         
  lcd.backlight();      
  Serial.println("--- LEMROL 2.0 ---");

  SPI.begin();         //INICIALIZA O BARRAMENTO SPI
  rfid.PCD_Init();     //INICIALIZA MFRC522


  pinMode(enter, INPUT_PULLUP);
  pinMode(back, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

}


void loop() {

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    readBadge();
    verifyDay();
  }

  switch (menu_sub) {
    case 0:
      select_menu();
      select_buttons();
      break;
    case 1:
      select_menu_week();
      select_buttons();
      break;
    case 2:
      select_tags();
      select_buttons();
      break;
  }

  
  if (buzzActive == true ){
    buzzerCount++;
    if ( buzzerCount >= 50) {
      activeBuzzer();
    }
  } 
  delay(200);
}

void readBadge() {
  strID = "";
  for (byte i = 0; i < 4; i++) {
    
    strID +=
      (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX) + (i != 3 ? ":" : "");
  }
  strID.toUpperCase();
  /***FIM DO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA***/

  Serial.print("Identificador (UID) da tag: ");  //IMPRIME O TEXTO NA SERIAL
  Serial.println(strID);                         //IMPRIME NA SERIAL O UID DA TAG RFID

  rfid.PICC_HaltA();       //PARADA DA LEITURA DO CARTÃO
  rfid.PCD_StopCrypto1();  //PARADA DA CRIPTOGRAFIA NO PCD
}


void select_menu() {
  switch (menu_current) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("       MENU      ");
      lcd.setCursor(0, 1);
      lcd.print("   WEEK DAYS   >");
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("<     RESET      ");
      break;
  }
}

void select_menu_week() {
  lcd.setCursor(4, 0);
  lcd.print("WEEK DAYS    ");
  switch (menu_week) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("     MONDAY    >");
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("<   TUESDAY    >");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("<   WEDNESDAY  >");
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print("<   THURSDAY   >");
      break;
    case 4:
      lcd.setCursor(0, 1);
      lcd.print("<    FRIDAY    >");
      break;
    case 5:
      lcd.setCursor(0, 1);
      lcd.print("<   SATURDAY   >");
      break;
    case 6:
      lcd.setCursor(0, 1);
      lcd.print("<    SUNDAY     ");
      break;
  }
}

void select_tags() {
  switch(menu_week) {
    case 0:
      switch (menu_tags) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("   [ ] AZUL    >");
        if (Monday.tags[0] == 1){
          lcd.setCursor(0, 5);
          lcd.print("   [X");
        }
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] BRANCO  >");
        if (Monday.tags[1] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] ROSA     ");
        if (Monday.tags[2] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
    }
    break;
    case 1:
      switch (menu_tags) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("   [ ] AZUL    >");
        if (Tuesday.tags[0] == 1){
          lcd.setCursor(0, 5);
          lcd.print("   [X");
        }
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] BRANCO  >");
        if (Tuesday.tags[1] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] ROSA     ");
        if (Tuesday.tags[2] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
    }
    break;
    case 2:
      switch (menu_tags) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("   [ ] AZUL    >");
        if (Wednesday.tags[0] == 1){
          lcd.setCursor(0, 5);
          lcd.print("   [X");
        }
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] BRANCO  >");
        if (Wednesday.tags[1] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] ROSA     ");
        if (Wednesday.tags[2] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
    }
    break;
    case 3:
      switch (menu_tags) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("   [ ] AZUL    >");
        if (Thursday.tags[0] == 1){
          lcd.setCursor(0, 5);
          lcd.print("   [X");
        }
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] BRANCO  >");
        if (Thursday.tags[1] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("      TAGS      ");
        lcd.setCursor(0, 1);
        lcd.print("<  [ ] ROSA     ");
        if (Thursday.tags[2] == 1){
          lcd.setCursor(0, 1);
          lcd.print("<  [X");
        }
        break;
    }
    break;
  }
}

void checkTagsByDay() {
  switch(menu_week) {
    case 0: 
    switch(menu_tags) {
      case 0:
      if (Monday.dayList.indexOf(azul) != -1){
        Monday.dayList.replace(azul,"");
        Monday.tags[0] = 0;
        Monday.totalTags--;
      }
      else{
        Monday.dayList = Monday.dayList + azul;
        Monday.tags[0] = 1;
        Monday.totalTags++;
      }
      break;
      case 1:
      if (Monday.dayList.indexOf(branco) != -1){
        Monday.dayList.replace(branco, "");
        Monday.tags[1] = 0;
        Monday.totalTags--;
      }
      else{
        Monday.dayList = Monday.dayList + branco;
        Monday.tags[1] = 1;
        Monday.totalTags++;
      }
      break;
      case 2:
      if (Monday.dayList.indexOf(rosa) != -1){
        Monday.dayList.replace(rosa, "");
        Monday.tags[2] = 0;
        Monday.totalTags--;
      }
      else{
        Monday.dayList = Monday.dayList + rosa;
        Monday.tags[2] = 1;
        Monday.totalTags++;
      }
      break;
    }
    break;
    case 1: 
    switch(menu_tags) {
      case 0:
      if (Tuesday.dayList.indexOf(azul) != -1){
        Tuesday.dayList.replace(azul, "");
        Tuesday.tags[0] = 0;
      }
      else{
        Tuesday.dayList = Tuesday.dayList + azul;
        Tuesday.tags[0] = 1;
      }
      break;
      case 1:
      if (Tuesday.dayList.indexOf(branco) != -1){
        Tuesday.dayList.replace(branco, "");
        Tuesday.tags[1] = 0;
      }
      else{
        Tuesday.dayList = Tuesday.dayList + branco;
        Tuesday.tags[1] = 1;
      }
      break;
      case 2:
      if (Tuesday.dayList.indexOf(rosa) != -1){
         Tuesday.dayList.replace(rosa, "");
        Tuesday.tags[2] = 0;
      }
      else{
        Tuesday.dayList = Tuesday.dayList + rosa;
        Tuesday.tags[2] = 1;
      }
      break;
    }
    break;
    case 2: 
    switch(menu_tags) {
      case 0:
      if (Wednesday.dayList.indexOf(azul) != -1){
        Wednesday.dayList.replace(azul,"");
        Wednesday.tags[0] = 0;
        Wednesday.totalTags--;
      }
      else{
        Wednesday.dayList = Wednesday.dayList + azul;
        Wednesday.tags[0] = 1;
        Wednesday.totalTags++;
      }
      break;
      case 1:
      if (Wednesday.dayList.indexOf(branco) != -1){
        Wednesday.dayList.replace(branco, "");
        Wednesday.tags[1] = 0;
        Wednesday.totalTags--;
      }
      else{
        Wednesday.dayList = Wednesday.dayList + branco;
        Wednesday.tags[1] = 1;
        Wednesday.totalTags++;
      }
      break;
      case 2:
      if (Wednesday.dayList.indexOf(rosa) != -1){
        Wednesday.dayList.replace(rosa, "");
        Wednesday.tags[2] = 0;
        Wednesday.totalTags--;
      }
      else{
        Wednesday.dayList = Wednesday.dayList + rosa;
        Wednesday.tags[2] = 1;
        Wednesday.totalTags++;
      }
      break;
    }
    break;
    case 3: 
    switch(menu_tags) {
      case 0:
      if (Thursday.dayList.indexOf(azul) != -1){
        Thursday.dayList.replace(azul,"");
        Thursday.tags[0] = 0;
        Thursday.totalTags--;
      }
      else{
        Thursday.dayList = Thursday.dayList + azul;
        Thursday.tags[0] = 1;
        Thursday.totalTags++;
      }
      break;
      case 1:
      if (Thursday.dayList.indexOf(branco) != -1){
        Thursday.dayList.replace(branco, "");
        Thursday.tags[1] = 0;
        Thursday.totalTags--;
      }
      else{
        Thursday.dayList = Thursday.dayList + branco;
        Thursday.tags[1] = 1;
        Thursday.totalTags++;
      }
      break;
      case 2:
      if (Thursday.dayList.indexOf(rosa) != -1){
        Thursday.dayList.replace(rosa, "");
        Thursday.tags[2] = 0;
        Thursday.totalTags--;
      }
      else{
        Thursday.dayList = Thursday.dayList + rosa;
        Thursday.tags[2] = 1;
        Thursday.totalTags++;
      }
      break;
    }
    break;
  }
  select_tags();
}


void select_buttons() {

  bool select_up = true;
  bool select_down = true;
  bool select_enter = true;
  bool select_back = true;
  
  select_up = digitalRead(up);
  select_down = digitalRead(down);
  select_enter = digitalRead(enter);
  select_back = digitalRead(back);
  
  if (menu_sub == 0){
    if (!select_down && ((millis() - tempo) > tempo_deBounce)){
      menu_current = 1;
      delay(200);
    }
    if (!select_up && ((millis() - tempo) > tempo_deBounce)){
      menu_current = 0;
      delay(200);
    }
    if (!select_enter && ((millis() - tempo) > tempo_deBounce)){
      if (menu_current == 1){
        reset();
      }
      menu_sub =+ 1;
      delay(200);
    }
  }
  
  else if(menu_sub == 1){
    if (!select_down && ((millis() - tempo) > tempo_deBounce)){
      menu_week += 1;
      delay(200);
    }
    if (!select_up && ((millis() - tempo) > tempo_deBounce)){
      menu_week -= 1;
      delay(200);
    }
    if (!select_back && ((millis() - tempo) > tempo_deBounce)){
      menu_sub = 0;
      menu_current = 0;
      menu_week = 0;
      delay(200);
    }
    if (!select_enter && ((millis() - tempo) > tempo_deBounce)) menu_sub = 2;

  }
  
  else if(menu_sub == 2){
    if (!select_down && ((millis() - tempo) > tempo_deBounce)){
      menu_tags += 1;
      delay(200);
    }
    if (!select_up && ((millis() - tempo) > tempo_deBounce)){
      menu_tags -= 1;
      delay(200);    
    }
    if (!select_back && ((millis() - tempo) > tempo_deBounce)){
      menu_sub = 1;
      menu_tags = 0;
      delay(200); 
    }
    if (!select_enter && ((millis() - tempo) > tempo_deBounce)){
    checkTagsByDay();
    }
  }
}

void currentDay(){

}

void verifyDay(){
  buzzActive = true;
  currentTag = strID;
  Serial.println(buzzActive);
  Serial.println(buzzerCount);

  if (setDay == false){

    currentDay();  // verifica que dia é | Criar função para isso
    String day = "Wednesday";
    if (day == "Monday") Current = Monday;
    else if (day == "Tuesday") Current = Tuesday;
    else if (day == "Wednesday") Current = Wednesday;
    else if (day == "Thursday") Current = Thursday;
    else if (day == "Friday") Current = Friday;
    else if (day == "Saturday") Current = Saturday;
    else if (day == "Sunday") Current = Sunday;    
    setDay = true;
  }

  if (Current.dayList.indexOf(currentTag) != -1){
      tagsRead++;
      Current.dayList.replace(currentTag,"");
  } 
  delay(500);

  if (tagsRead == Current.totalTags){
  tagsRead = 0;
  buzzActive = false;
  setDay = false;
  Serial.println("ALL TAGS SCANNED!");
  }
}

void activeBuzzer(){
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer, LOW);
  delay(2000);
  buzzerCount = 0;
  tagsRead = 0;
  buzzActive = false;
  setDay = false;
  showTagsNotFound();
  }

void showTagsNotFound(){
  lcd.setCursor(0, 0);
  lcd.print(" TAGS NOT FOUND ");
  lcd.setCursor(0, 1);
  String tagsNotFound = "";
  lcd.print("                ");
  if (Current.dayList.indexOf(azul) != -1){
    tagsNotFound = tagsNotFound + "AZUL";
  }
  if (Current.dayList.indexOf(branco) != -1){
    tagsNotFound = tagsNotFound +  " BRANCO";
  }
  if (Current.dayList.indexOf(rosa) != -1){
    tagsNotFound = tagsNotFound + " ROSA";
  }
  lcd.setCursor(0, 1);
  lcd.print(tagsNotFound);

  bool select_enter = true;
  select_enter = digitalRead(enter);

  while (select_enter && ((millis() - tempo) > tempo_deBounce)){
    delay(500);
    select_enter = digitalRead(enter);
  }
}

void reset(){
  ESP.restart();
}
