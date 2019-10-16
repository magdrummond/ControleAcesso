//Eletronic Lock key Program for Arduino UNO
//Copyright 2015 by Keytronic
//
//Developer by Marco Aurelio Gorgulho Drummond
//Start Date: 22/01/2015

#include <Keypad.h>
#include <Servo.h>
#include <EEPROMex.h>
#include <avr/sleep.h>
//#include <avr/power.h>

//*********************************************************************************************************** 


//***********************************************************************************************************
// GLOBAL VARIABLES
//***********************************************************************************************************

Servo keytronic_servo;

int position    = 0;
const byte ROWS = 4;
const byte COLS = 4;

char teclas[ROWS][COLS] = {
                             {'1','2','3','A'},
                             {'4','5','6','B'},
                             {'7','8','9','C'},
                             {'*','0','#','D'}
                          };

byte rowPins[ROWS] = {9,8,7,6};
byte colPins[COLS] = {5,4,3,2};

// number of items in an array
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

Keypad keypad = Keypad(makeKeymap(teclas), rowPins, colPins, ROWS, COLS);

//********************** We don't want to do anything except wake *******************************************
EMPTY_INTERRUPT (PCINT0_vect)
EMPTY_INTERRUPT (PCINT1_vect)
EMPTY_INTERRUPT (PCINT2_vect)
//***********************************************************************************************************

int pos = 0;

//int ctrlPower = 11;
int ledPin   = 13;               // choose the pin for the LED
int inputPin = 12;               // choose the input pin (for a pushbutton)

int val = 0;                      // variable for reading the pin status

String pwd;
String password;
char setReadCtrl = '#';

int statusOpen            = 0;
int startProgSenha        = 0;
int startProgSenhaDefault = 0;
int statusKeyBoardCtrlPos = 0;
int onoff_ctrl            = 0;
int seg                   = 0;

String strBuffer          = "";
String opStrType          = "";
boolean acordaFechadura   = false;
boolean startLedOn        = false;
boolean WakeUp            = false;

boolean startTimerClock   = false;

const   int   maxAllowedWrites   = 250; // Number of writes memories configuration
const   int   memBase            = 350; 

//*************************************************************************************************************
// Start function
//*************************************************************************************************************

void setup() {
  
  //Serial.begin(9600);
  
  password = "";
  
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  
  // start reading from position memBase (address 0) of the EEPROM. Set maximumSize to EEPROMSizeUno  
  // Writes before membase or beyond EEPROMSizeUno will only give errors when _EEPROMex_DEBUG is set   
  EEPROM.setMemPool(memBase, EEPROMSizeUno);
  
  pinMode(ledPin, OUTPUT);  
  pinMode(inputPin, INPUT);       // declare pushbutton as input
  
  //************************ pin change interrupt masks (see above list) ******************************************
  PCMSK2 |= _BV (PCINT22);   // pin 6
  PCMSK2 |= _BV (PCINT23);   // pin 7
  PCMSK0 |= _BV (PCINT0);    // pin 8
  PCMSK0 |= _BV (PCINT1);    // pin 9  
  //***************************************************************************************************************
  
  //delay(2000);
  /*
  char str0[7] = "\0\0\0\0\0\0";
  Serial.println(str0);
  EEPROM.writeBlock<char>(0, str0, 7);              

  char str1[7] = "\0\0\0\0\0\0";
  Serial.println(str1);
  EEPROM.writeBlock<char>(100, str1, 7);   

  Serial.print("Escrita str0");Serial.print("\t\t\t"); Serial.println(str0);
  Serial.print("Escrita str1");Serial.print("\t\t\t"); Serial.println(str1);  
  
  char res0[7];
  EEPROM.readBlock<char>(0,res0,7); 

  char res1[7];
  EEPROM.readBlock<char>(100,res1,7); 
  
  Serial.print("Leitura res0");Serial.print("\t\t\t"); Serial.println(res0);
  Serial.print("Leitura res1");Serial.print("\t\t\t"); Serial.println(res1);
  */
  
  //int addressCharArray = EEPROM.getAddress(sizeof(char)*14);
  //Serial.print(addressCharArray); Serial.print(" \t\t\t "); Serial.print(sizeof(char)*14); Serial.println(" (array of 14 chars)");

  //keytronic_servo.attach(11); // attaches the servo on pin 10 to the servo object    
  //digitalWrite(ledPin, HIGH);  
  
  clearPwdString();
}
  
void clearPwdString() {
     pwd = "";
}

void clearPwd_Gravada() {
     password = "";
}

void openkey() {
  //Serial.println("Funcao openkey()");
  keytronic_servo.attach(11);        // attaches the servo on pin 11 to the servo object  
  keytronic_servo.write(0);
  
  val = 0;  
  
  for(pos = 0; pos < 300; pos += 1)  // goes from 0 degrees to 300 degrees
  {                                  // in steps of 1 degree
    keytronic_servo.write(pos);      // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
  keytronic_servo.detach();         // attaches the servo on pin 11 to the servo object        
}

void closekey() {
  //Serial.println("Funcao closekey()");    
  keytronic_servo.attach(11);        // attaches the servo on pin 11 to the servo object  
  
  for(pos = 300; pos>=1; pos-=1)     // goes from 300 degrees to 0 degrees
  {
    keytronic_servo.write(pos);      // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
  keytronic_servo.detach();         // attaches the servo on pin 11 to the servo object      
}

void clearMemPassAdmin() {
  char str0[7] = "\0\0\0\0\0\0";
  //Serial.println(str0);
  EEPROM.writeBlock<char>(0, str0, 7);                
}

void clearMemPassOrd() {
  char str1[7] = "\0\0\0\0\0\0";
  //Serial.println(str1);
  EEPROM.writeBlock<char>(100, str1, 7);     
}  

String readMemPassAdmin() {
        char strMemReadAdmin[7];        
        EEPROM.readBlock<char>(0,strMemReadAdmin, 7);
        String s = String(strMemReadAdmin);
        s.trim();
        //Serial.print("Le em readMemPassAdmin: ");Serial.print("\t\t");Serial.println(s);
        return s;
}

String readMemPassOrd() {
        char strMemReadOrd[7];        
        EEPROM.readBlock<char>(100,strMemReadOrd, 7);
        String o = String(strMemReadOrd);
        o.trim();
        return o;
}

void ctrlFechadura(char key) {
     static unsigned long ult_tempo = 0;
     int tempo = millis();  
  
     if (key == '*' || startProgSenha == 1) {
        // Inicia Programaçao e gravaçao de senha de administrador e usuario
        // limite de 2 usurios 1 administrador e usuario comum
        startProgSenha = 1; // ativa programaçao de senha
                           
        if (onoff_ctrl == 0) {
           if (key == '0') {
               opStrType = "admin";
               startLedOn = true;
               clearPwdString();
               clearPwd_Gravada();
               position = 0;
               onoff_ctrl = 1;
               return;
           }

           if (key == '1') {
               opStrType = "ordin";
               startLedOn = true;
               clearPwdString();
               clearPwd_Gravada();
               position = 0;
               onoff_ctrl = 1;
               return;
           }    
        }
        
        //*****************************************************************************************************************
        // Programacao de Senha Modulo Administrador
        //*****************************************************************************************************************
        if (opStrType == "admin") {
            //Serial.println("Programacao Administrador opcao administrador");    
            digitalWrite(ledPin, HIGH);
            delay(100);
            digitalWrite(ledPin, LOW);            
            if (startProgSenhaDefault == 0) {
                //Serial.println("Le memoria opcao default_password_admin");
                password = readMemPassAdmin();
                //Serial.println(password);
                //Serial.print("Valor da variavel password: ");Serial.print("\t\t");Serial.println(password.length());
            }
            
            if (password == "") {          
                //Serial.println("Solicita entrada de senha default");
                startProgSenhaDefault = 1;
            
                if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || 
                    key == '7' || key == '8' || key == '9' || key == 'A' || key == 'B' || key == 'C' || key == 'D')
                {
                    strBuffer += key;
                }  
            
                if (strBuffer.length() == 6) {
                   //Serial.println("Grava Senha de Administrador na Memoria");
                   char strCharBuffer[7] = "\0\0\0\0\0\0";
                   strBuffer.toCharArray(strCharBuffer,7);
                   //Serial.println(strCharBuffer);
                   EEPROM.writeBlock<char>(0, strCharBuffer, 7);
                   position = 0;
                   startProgSenha = 0;
                   startProgSenhaDefault = 0;
                   onoff_ctrl = 0;
                   opStrType = "";
                   strBuffer = "";
                   clearPwdString();
                   clearPwd_Gravada(); 
                   digitalWrite(ledPin, LOW);                           
                   return;
                }
            } else {
                 startProgSenhaDefault = 1;             
                 if (statusKeyBoardCtrlPos == 0) {
                     startTimerClock = true;                                      
                     if (key == password[position]) {
                         pwd += key;
                         position++;
                     }
                 }           
             
                 // Reset Password            
                 if (startTimerClock == true) {
                     if (tempo - ult_tempo >= 1000) {
                        ult_tempo = tempo;
                        seg++;   
                     }
                     if (seg >= 120) {
                        seg = 0;
                        position = 0;
                        startProgSenha = 0;
                        startProgSenhaDefault = 0;
                        onoff_ctrl = 0;
                        opStrType = "";
                        strBuffer = "";
                        clearPwdString();
                        clearPwd_Gravada();                                       
                        acordaFechadura = false;
                        startTimerClock = false;
                        WakeUp = false;
                        digitalWrite(ledPin, LOW);
                        return;                                       
                     }
                     //Serial.print("Relogio");Serial.print("\t\t");Serial.print(min);Serial.println(seg);
                 }
             
                 if (pwd == password) {
                     statusKeyBoardCtrlPos = 1;
                     //Serial.println("Permissao Administrador");
                     //Serial.println(password);
                     startTimerClock = false;
                     seg = 0;                     
                     if (key == '#') {
                        //Serial.println("Limpa Senha de Administrador da Memoria");
                        digitalWrite(ledPin, HIGH);
                        clearMemPassAdmin();
                        statusKeyBoardCtrlPos = 0;
                        startProgSenha = 0;
                        startProgSenhaDefault = 0;
                        onoff_ctrl = 0;
                        opStrType = "";
                        strBuffer = "";
                        clearPwdString();
                        clearPwd_Gravada();                    
                        delay(500);
                        digitalWrite(ledPin, LOW);                        
                        acordaFechadura = false;                     
                        return;
                     }                
                     
                     if (key == '*') {
                        //Serial.println("Limpa Senha de Ordinaria da Memoria");
                        digitalWrite(ledPin, HIGH);
                        clearMemPassOrd();
                        statusKeyBoardCtrlPos = 0;
                        startProgSenha = 0;
                        startProgSenhaDefault = 0;
                        onoff_ctrl = 0;
                        opStrType = "";
                        strBuffer = "";
                        clearPwdString();
                        clearPwd_Gravada();
                        delay(500);
                        digitalWrite(ledPin, LOW);
                        acordaFechadura = false;                     
                        return;
                     }
                 }
            }
        }// Fim Programacao Modulo Administrador 
                     
        //**************************************************************************************************************
        // Programacao Senha Modulo Ordinario
        //**************************************************************************************************************
        if (opStrType == "ordin") {
            //Serial.println("Programacao senha opcao ordinaria");
            digitalWrite(ledPin, HIGH);
            delay(100);
            digitalWrite(ledPin, LOW);                        
            if (startProgSenhaDefault == 0) {            
                //Serial.println("Le memoria opcao password_ordin");
                password = readMemPassOrd();
                //Serial.println(password); 
            }

            if (password == "") {       
                startProgSenhaDefault = 1;          
                if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || 
                    key == '7' || key == '8' || key == '9' || key == 'A' || key == 'B' || key == 'C' || key == 'D')
                {
                    strBuffer += key;
                } 
            
                if (strBuffer.length() == 6) {
                   //Serial.println("Grava Senha Ordinaria na Memoria");
                   digitalWrite(ledPin, HIGH);
                   char strCharBuffer[7] = "\0\0\0\0\0\0";
                   strBuffer.toCharArray(strCharBuffer,7);               
                   EEPROM.writeBlock<char>(100, strCharBuffer, 7);
                   startProgSenha = 0;
                   startProgSenhaDefault = 0;
                   onoff_ctrl = 0;
                   opStrType = "";
                   strBuffer = "";
                   clearPwdString();
                   clearPwd_Gravada();  
                   delay(500);                  
                   digitalWrite(ledPin, LOW);
                   acordaFechadura = false;                     
                   //Serial.println(strBuffer);  
                   return;          
                }                                 
            }            
        } // Fim Programacao Modulo Ordinario             
     } else {       
           if (setReadCtrl == '#') {               
               if (key == '0') {
                  //Serial.println("Senha Administrador");      
                  startLedOn = true;
                  startTimerClock = true;
                  seg = 0;                     
                  opStrType = "admin";
                  password = readMemPassAdmin();
                  //Serial.println(password);
                  onoff_ctrl = 1;
                  setReadCtrl = ' ';
                  return;
               }

               if (key == '1') {
                  //Serial.println("Senha Usuario Normal");      
                  startLedOn = true;
                  startTimerClock = true;
                  seg = 0;                     
                  opStrType = "ordin";
                  password = readMemPassOrd();
                  //Serial.println(password);           
                  onoff_ctrl = 1;
                  setReadCtrl = ' ';
                  return;
               }
           }
      
           if (startLedOn == true) {
               //Serial.println("Entra aqui para acender o LED");
               digitalWrite(ledPin, HIGH);
               delay(500);
               digitalWrite(ledPin, LOW);
               startLedOn = false;
           }
      
           if (opStrType == "admin") {
              //Serial.print("valor de key = ");Serial.print(key);Serial.print("\t\t\t password fragment");Serial.println(password[position]);
              if (key == password[position]) {
                  pwd += key;
                  position++;
              }
          }
      
          if (opStrType == "ordin") {
              if (key == password[position]) {
                  pwd += key;
                  position++;
              } 
          }
            
          // Reset Password            
          if (startTimerClock == true) {
             if (tempo - ult_tempo >= 1000) {
                 ult_tempo = tempo;
                 seg++;   
             }
             if (seg >= 60) {
                 seg = 0;
                 clearPwdString();
                 statusOpen = 0;
                 onoff_ctrl = 0;
                 position = 0;      
                 opStrType = "";        
                 acordaFechadura = false;
                 startTimerClock = false;
                 WakeUp = false;
                 //digitalWrite(ledPin, LOW);             
                 return;               
             }
          }
          
          //Serial.print(pwd);Serial.print("\t\t");Serial.println(password);
          
          if (opStrType == "admin") { 
              if (pwd == password) {
                  startTimerClock = false;
                  //Serial.println("Admin Open Door");
                  digitalWrite(ledPin, LOW);             
                  seg = 0;
                  openkey(); 
                  clearPwdString();
                  statusOpen = 1;
                  onoff_ctrl = 0;
                  position = 0;      
                  opStrType = "";
                  //digitalWrite(ledPin, LOW);             
              }
          }
          
          if (opStrType == "ordin") { 
             if (pwd == password) {
                 startTimerClock = false;
                 //Serial.println("Ordi Open Door");
                 digitalWrite(ledPin, LOW);             
                 seg = 0;
                 openkey(); 
                 clearPwdString();
                 statusOpen = 1;
                 onoff_ctrl = 0;
                 position = 0;      
                 opStrType = "";
                 //digitalWrite(ledPin, LOW);             
             } 
          }      
     }  
}

void wakeUpNow() {
}

// set pins as keypad library expects them
// or call: kpd.initializePins ();
// however in the library I have that is a private method
void reconfigurePins() {
  byte i;
 
  // go back to all pins as per the keypad library
  for (i = 0; i < NUMITEMS(colPins); i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH);
  }  // end of for each column

  for (i = 0; i < NUMITEMS(rowPins); i++) {
    pinMode(rowPins[i], INPUT);
    digitalWrite(rowPins[i], HIGH);
  } // end of for each row
  
} // end of reconfigurePins

// here we put the arduino to sleep
void sleepNow() {
  byte i;
   
  // set up to detect a keypress
  for (i = 0; i < NUMITEMS(colPins); i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);   // columns low
  }  // end of for each column

  for (i = 0; i < NUMITEMS(rowPins); i++) {
    pinMode(rowPins[i], INPUT);
    digitalWrite(rowPins[i], HIGH);  // rows high (pull-up)
  }  // end of for each row
   
  // now check no pins pressed (otherwise we wake on a key release)
  for (i = 0; i < NUMITEMS(rowPins); i++) {
    if (digitalRead(rowPins[i]) == LOW) {
       reconfigurePins();
       return;
    } // end of a pin pressed
  }  // end of for each row
 
  // overcome any debounce delays built into the keypad library
  delay(50);
 
  // at this point, pressing a key should connect the high in the row to the
  // to the low in the column and trigger a pin change
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_enable();

  byte old_ADCSRA = ADCSRA;
  
  // disable ADC to save power
  ADCSRA = 0; 

  PRR = 0xFF;  // turn off various modules
   
  PCIFR  |= _BV (PCIF0) | _BV (PCIF1) | _BV (PCIF2);   // clear any outstanding interrupts
  PCICR  |= _BV (PCIE0) | _BV (PCIE1) | _BV (PCIE2);   // enable pin change interrupts
   
  // turn off brown-out enable in software
  MCUCR = _BV (BODS) | _BV (BODSE);
  MCUCR = _BV (BODS);
 
  sleep_cpu(); 

  // cancel sleep as a precaution
  sleep_disable();
  PCICR  = 0;  // cancel pin change interrupts
  PRR    = 0;    // enable modules again
  ADCSRA = old_ADCSRA; // re-enable ADC conversion
 
  // put keypad pins back how they are expected to be
  reconfigurePins();
}

void loop() {  

     char k = keypad.getKey();
          
     if (k) {
        acordaFechadura = true;        
        if (WakeUp == false) {
           //Serial.println("Foi beijada e acordou");           
           sleepNow();             
           WakeUp = true;
        }
     } else {
          if (acordaFechadura == false) {
             //Serial.println("Bela Adormecida");
             sleepNow();
          }
     }
     
     if (acordaFechadura == true) {
         val = digitalRead(inputPin);  // read input value
         Serial.println(val);
         ctrlFechadura(k);  
         if (statusOpen == 1) {
             if (val == 1) { // check if the input is LOW          
                //Serial.println("Entra aqui apos val = 1");
                opStrType  = "";
                val        = 0;       
                statusOpen = 0;
                position   = 0;
                //Serial.println("Acende LED Vermelho");
                digitalWrite(ledPin, HIGH);
                delay(500);
                digitalWrite(ledPin, LOW); 
                delay(500);            
                digitalWrite(ledPin, HIGH);
                delay(500);
                digitalWrite(ledPin, LOW);                             
                closekey();
                setReadCtrl = '#';
                clearPwdString();
                clearPwd_Gravada();
                acordaFechadura = false;
                WakeUp = false;
             }
         }         
     }        
     
     delay(100);  
}

