// Mislav Stipić 2021 Sustav za pametnu kuću (dio za arduino UNO)
// projekt uključuje alarm (laser, indikator i zvučnik), automatcko otvaranje vrata pomoću servo motora
// pir senzor za automatsko zatvaranje vrata

#include <Servo.h>

Servo mojServo; // stvori servo objekt za kontrolu servo motora

const int zvucnikPin = 8;       // pin za zvucnik (const je oznaka za varijablu koja se ne može mijenjati tokom rada programa)
const int motor_vrata = 2;      // pin za motor
const int PirPin = 5;           // pin za pir senzor
const int vrataPin = 6;         // pin za primanje naredbe za otvaranje vrata sa esp8266 (D8)
const int alarmPin = 7;         // pin za primanje naredbe paljenja alarma sa esp8266 (RX)

int status_vrata = 0;   // pomoćna varijabla za primanje naredbe otvaranja vrata (0 vrata zatvorena, 1 vrata otvorena)
int status_alarm = 0;   // pomoćna varijabla za primanje naredbe aktiviranja alarma (0 alarm neaktivan, 1 alarm aktiviran)

int poz_motora = 0; // početna pozicija motora


unsigned long predhodno_v = 0;        // pomoćna varijabla za pratiti predhodno vrijeme

const long trajanje_otvoreno = 3000;  // interval trajanja provjere (ako je prošlo 3000 ms  - zatvori vrata)



// setup funkcija se izvršava jednom prije početka glavnog koda
void setup() {

  mojServo.attach(motor_vrata);     // pin za servo motor  //ZA OTVORIT VRATA SERVO PIN MORA BITI PWM
  pinMode(zvucnikPin, OUTPUT);      // postavi pin za zvučnik kao izlaz
  pinMode(PirPin, INPUT);           // postavi pin za Pir senzor kao ulaz
  
  pinMode(vrataPin, INPUT);         // postavi pin za provjeru otvaranja vrata kao ulaz
  pinMode(alarmPin, INPUT);         // postavi pin za provjeru alarma kao ulaz

  mojServo.write(poz_motora); //zatvori vrata ako su bila otvorena ili u nekoj poziciji
}

void loop() {



 if(digitalRead(PirPin) == HIGH || digitalRead(vrataPin) == HIGH)  // ako je aktiviran senzor ili ako smo uprili dugme "otvori vrata"
    status_vrata = 1; // promini status vrata

 if(digitalRead(alarmPin) == HIGH)    // provjeri ESP o alarmu
    status_alarm = 1;
 if(digitalRead(alarmPin) == LOW)    // provjeri reset alarma sa ESP
    status_alarm = 0;   

 if(status_alarm == 1)     // ako smo primili nešto , upali alarm
    upali_alarm();

 if(status_vrata == 1)    // ako smo primili nešto sa senzora ili mobitela otvori vrata
   otvori_v();

if(status_vrata == 1 && digitalRead(PirPin)== LOW){ // provjeravaj samo ako su vrata otvorena i PIR je low
  
    unsigned long trenutno_v = millis(); // spremi trenutno vrijeme
    if (trenutno_v - predhodno_v >= trajanje_otvoreno ){  // provjeravaj dali su vrata otvorena svaki interval vremena (trajanje_otvoreno)
      
        predhodno_v = trenutno_v;  // resetiraj brojač svako trajanje_otvoreno vrijeme
        
              zatvori_v();        // ako su vrata otvorena zatvori ih
              status_vrata = 0;   // promjeni status vrata na zatvorena (nakon što smo ih zatvorili)            
      
    }
  }

}
///////////////// kraj glavne petlje //////////////////////////

void upali_alarm(){     // funkcija za zvučni alarm tj zvučnik

  tone(zvucnikPin, 1000);     // upali zvučnik, postavi frekvenciju tona na 1000 Hz
  delay(500);                 // razmak između uključenog tona i ugašenog tona (500 ms)
  noTone(zvucnikPin);         // isključi ton
  delay(500);                 // razmak između uključenog tona i ugašenog tona (500 ms)
  
}

void otvori_v(){
  
  for(poz_motora = 0; poz_motora <=90; poz_motora ++){
    
    mojServo.write(poz_motora); // otvaraj vrata po jedan stupanj dok ne dođe do 90 stupnjeva
    delay(15);    
    }
  
}

void zatvori_v(){
  
    for(poz_motora = 90; poz_motora >= 0; poz_motora --){
    
    mojServo.write(poz_motora);  // zatvaraj vrata po jedan stupanj dok ne dođe do 0 stupnjeva
    delay(15);    
    }
  
}



//
