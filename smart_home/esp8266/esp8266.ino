// Mislav Stipić 2021 "Sustav za pametnu kuću" (dio za esp8266)
// projekt uključuje mjerenje temperature i vlage, kontrola alarma, uređaja, i vrata preko BLYNK aplikacije, 
// uključuje "tihi" alarm za notifikacije na mobitelu

// Projekt koristi dva mikrokontrolera zbog uštede ožičenja i manjka pinova na esp8266 ploči


#define BLYNK_PRINT Serial          
#include <BlynkSimpleEsp8266.h>   // biblioteka za povezivanje s aplikacijom
#include <ESP8266WiFi.h>          // biblioteka za povezivanje s internetom
#include <SPI.h>                  // biblioteka za SPI protokol kod povezivanja s DHT11
#include <DHT.h>                  // biblioteka za DHT senzor
#include <Adafruit_Sensor.h>      // biblioteka za prilagodbu DHT biblioteke ovom mikrokontorleru

#define DHTPIN 5            // Digital pin D1 za DHT11

// Odkomentiraj senzor koji se koristi (u našem slučaju to je DHT11)

#define DHTTYPE DHT11       // DHT 11
//#define DHTTYPE DHT22     // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21     // DHT 21, AM2301

DHT dht(DHTPIN, DHTTYPE);   // pridjeli pin i tip senzora objektu dht

WidgetLED led1(V6);   // za notifikaciju aktivnosti alarma na mobitelu

// definicije pinova za spajanje 
#define RelayPin1 14  //D5    // releji
#define RelayPin2 12  //D6
#define RelayPin3 13  //D7

#define vrataPin 15   //D8    // spoji na arduino pin 6
#define alarmPin 3    //RX    // spoji na arduino pin 7

#define SwitchPin1 10  //SD3  // pinovi za sklopkice
#define SwitchPin2 0   //D3 
#define SwitchPin3 4   //D2

#define wifiLed   16   //D0   // led dioda za status wifija

#define VPIN_BUTTON_1    V1   // virtualni pinovi za povezivanje aplokacije sa programom
#define VPIN_BUTTON_2    V2
#define VPIN_BUTTON_3    V3 
#define vrata_button     V4
#define alarm_button     V5

int rel_status_1 = 1; // statusi releja (0 je aktivno , 1 neaktivno)
int rel_status_2 = 1; 
int rel_status_3 = 1; 

int vrata_status = 1; // status za otvoreno/zatvoreno za vrata

int wifiFlag = 0;     // zasstavica za provjeru povezanosti s internetom

int ldr_pin = A0;          // pin za senzor alarma
int alarm_status = 0;      // varijabla koja služi za dojavu o statusu alarma
float analogValue;         // pomoćna varijabla za alarm (za očitanje sa senzora)

/*
#define AUTH "_MJrfol9yUZU3CSqUUmQ8dAhoiQWSHET"  // Token za Blynk aplikaciju.  
#define WIFI_SSID "AndroidAP5552"             // Wifi ime
#define WIFI_PASS "enoc2257"          // wifi šifra
*/

char auth[] = "_MJrfol9yUZU3CSqUUmQ8dAhoiQWSHET";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "AndroidAP5552";   //enter your wi-fi name
char pass[] = "enoc2257";

BlynkTimer timer; // inicijalizacija tajmera iz BlynkSimpleEsp8266.h biblioteke

void posaljiSenzor(){     //funkcija koja čita sa senzora i šalje podatke na mobitel
    
  float h = dht.readHumidity();    // čitaj vlažnost zraka sa DHT11 senzora
  float t = dht.readTemperature(); // čitaj temperaturu zraka sa DHT11 senzora

  Blynk.virtualWrite(V12, h);  //V11 is for Humidity
  Blynk.virtualWrite(V11, t);  //V12 is for Temperature
}




void mob_alarm(){     // funkcija za slanje notifikacije na mob o statusu alarma i paljenje alarma

 analogValue = analogRead(ldr_pin)* (3.3 / 1023.0); // provjeri senzor
 //Serial.println(analogValue);

 if(analogValue <= 3){  // ako je osvjetljenost senzora pala ispod 3.0 izvrši sljedeće
    led1.on();          // upali led diodu u aplikaciji
    alarm_status = 1;   // promjeni status alarma u "aktivno"
    digitalWrite(alarmPin, alarm_status);   // promjeni stanje na izlazu za uno
 }
     
 if(alarm_status == 0)   // ako alarm nije aktiviran nemoj raditi ništa
    led1.off();
     
   delay(100);
}


void relayOnOff(int relay){     // sortiranje ručnog i on line paljenja i gašenja releja

    switch(relay){        // aplikacija radi negirano (1 je LOW, 0 je HIGH)
      case 1:             // ako smo odabrali prvi relej promjeni njegovo stanje
             if(rel_status_1 == 1){         
              digitalWrite(RelayPin1, LOW); // upali relej 1
              rel_status_1 = 0;             // promjeni status prvog releja
              Serial.println("ON");
              }
             else{
              digitalWrite(RelayPin1, HIGH); // ugasi relej 1
              rel_status_1 = 1;              // promjeni status prvog releja
              Serial.println("OFF");
              }
             delay(100);
      break;
      case 2:             // ako smo odabrali drugi relej promjeni njegovo stanje
             if(rel_status_2 == 1){
              digitalWrite(RelayPin2, LOW); // upali relej 2
              rel_status_2 = 0;             // promjeni status drugog releja
              Serial.println("ON");
              }
             else{
              digitalWrite(RelayPin2, HIGH); // ugasi relej 2
              rel_status_2 = 1;              // promjeni status drugog releja
              Serial.println("OFF");
              }
             delay(100);
      break;
      case 3:             // ako smo odabrali treći relej promjeni njegovo stanje
             if(rel_status_3 == 1){
              digitalWrite(RelayPin3, LOW); // upali relej 3
              rel_status_3 = 0;             // promjeni status trećeg releja
              Serial.println("ON");
              }
             else{
              digitalWrite(RelayPin3, HIGH); // ugasi relej 3
              rel_status_3 = 1;              // promjeni status trećeg releja
              Serial.println("OFF");
              }
             delay(100);
      break;

      default : break;      // u svim ostalim slučajevima nemoj ništa raditi
      }
  
}

void releji(){  // ručno paljenje releja
   
    if (digitalRead(SwitchPin1) == LOW){ // ako je sklopka 1, LOW
      delay(200);
      relayOnOff(1);   // promjeni stanje
      Blynk.virtualWrite(VPIN_BUTTON_1, rel_status_1);   // osvježi dugme 1 na aplikaciji
    }
    else if (digitalRead(SwitchPin2) == LOW){ // ako je sklopka 2, LOW
      delay(200);
      relayOnOff(2);   // promjeni stanje             
      Blynk.virtualWrite(VPIN_BUTTON_2, rel_status_2);   // osvježi dugme 2 na aplikaciji
    }
    else if (digitalRead(SwitchPin3) == LOW){ // ako je sklopka 3, LOW
      delay(200);
      relayOnOff(3);   // promjeni stanje
      Blynk.virtualWrite(VPIN_BUTTON_3, rel_status_3);   // osvježi dugme 3 na aplikaciji
    }

}




// sinkronizacija sa aplikacijom u slučaju prekida interneta sa esp pločom
BLYNK_CONNECTED() {
  // zatraži zadnju vrijednost sa servera
  Blynk.syncVirtual(VPIN_BUTTON_1);   //dugmad za releje
  Blynk.syncVirtual(VPIN_BUTTON_2);
  Blynk.syncVirtual(VPIN_BUTTON_3);
  
  Blynk.syncVirtual(vrata_button);  // dugme za vrata
  Blynk.syncVirtual(alarm_button);  // dugme za reset alarma
}

// kada upremo dugme promijeni stanja

BLYNK_WRITE(VPIN_BUTTON_1) {      
  rel_status_1 = param.asInt();            // spremi vrijednost u stanje 1 sa mobitela
  digitalWrite(RelayPin1, rel_status_1);   // promjeni stanje na izlazu
}

BLYNK_WRITE(VPIN_BUTTON_2) {
  rel_status_2 = param.asInt();            // spremi vrijednost u stanje 2 sa mobitela
  digitalWrite(RelayPin2, rel_status_2);   // promjeni stanje na izlazu
}

BLYNK_WRITE(VPIN_BUTTON_3) {
  rel_status_3 = param.asInt();            // spremi vrijednost u stanje 3 sa mobitela
  digitalWrite(RelayPin3, rel_status_3);   // promjeni stanje na izlazu
}


BLYNK_WRITE(vrata_button) {
  vrata_status = param.asInt();           // spremi vrijednost u vrata_status sa mobitela
  digitalWrite(vrataPin, vrata_status);   // promjeni stanje na izlazu
}

BLYNK_WRITE(alarm_button) {
  alarm_status = param.asInt();           // spremi vrijednost u alarm_status sa mobitela
  digitalWrite(alarmPin, alarm_status);   // promjeni stanje na izlazu
}


void checkBlynkStatus(){ // ova funkcija se poziva svako 3 sekunde sa SimpleTimer-om

  bool isconnected = Blynk.connected();   // provjera da li je ploča spojena s internetom
  if (isconnected == false) {    // ako ploča nije spojena s internetom
    wifiFlag = 1;                // promjeni stanje zastavice
    digitalWrite(wifiLed, HIGH); // isključi ledicu za status povezanosti s internetom
  }
  if (isconnected == true) {    // ako je sloča spojena s internetom
    wifiFlag = 0;               // promjeni stanje zastavice
    digitalWrite(wifiLed, LOW); // uključi ledicu za status povezanosti s internetom
  }
}

// setup funkcija je funkcija koja se izvršava samo jedan put
void setup()
{
  Serial.begin(9600); // usb povezivanje s računalom 9600 mbps

  Blynk.begin(auth, ssid, pass);
  pinMode(RelayPin1, OUTPUT); // definicija relejni pinova kao izlazi
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  
  pinMode(vrataPin, OUTPUT);  // definicija pina za status otvorenosti vrata kao izlaz za povezivanjem sa uno-om
  pinMode(alarmPin, OUTPUT);  // definicija pina za status alarma kao izlaz za povezivanjem sa uno-om

  pinMode(wifiLed, OUTPUT);   // led dioda za status povezanosti kao izlaz

  pinMode(SwitchPin1, INPUT_PULLUP);  // definicja pinova za sklopkice za releje
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  
  digitalWrite(RelayPin1, rel_status_1); //tijekom pokretanja isključi sve releje
  digitalWrite(RelayPin2, rel_status_2);
  digitalWrite(RelayPin3, rel_status_3);
  
  digitalWrite(vrataPin, vrata_status); // također isključi pinove sa uno-om za vrata i alarm
  digitalWrite(alarmPin, alarm_status);
  
  dht.begin();  // pokreni biblioteku za dht senzor
  
            // pridjeli šifru i ime mreže biblioteci ESP8266WiFi.h
  timer.setInterval(3000L, checkBlynkStatus); // tajmer koji gleda svako 3 sekunde povezanost s internetom
  timer.setInterval(1000L, posaljiSenzor);    // funkcija za slanje podataka senzora za temp i vlagu na mob
  /*
  Blynk.config(AUTH); 
  WiFi.begin(WIFI_SSID, WIFI_PASS);                         // pridjeli token biblioteci BlynkSimpleEsp8266.h
  */
}

void loop(){

  
  if (WiFi.status() != WL_CONNECTED) // provjeri povezanost s internetom
  {
    Serial.println("WiFi nije spojen");
  }
  else
  {
    Serial.println("WiFi spojen");
    Blynk.run();                        // ako je povezano onda dopusti rad BlynkSimpleEsp8266.h biblioteke
  }

  timer.run(); // inicijalizacija tajmera
  
  if (wifiFlag == 0){ // ako je zastavica u 0 pokreni funkcije

    releji();  // kontrola releja preko interneta
    mob_alarm();
    posaljiSenzor();
   
  }
    
}
////////////kraj glavne petlje//////////////////////////////////////////////////////////////////////
