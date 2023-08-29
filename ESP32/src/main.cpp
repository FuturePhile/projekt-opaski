#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SoftwareSerial.h>
#include "BLEDevice.h"

// Zdalna usługa do której chcemy się podłączyć.
//static BLEUUID serviceUUID("00000000-0000-1000-8000-00805f9b34fb");
static BLEUUID serviceUUID("00001234-0000-1000-8000-00805f9b34fb");
// Charakterystyka usługi do której się podłączamy.
//static BLEUUID charUUID("0000");
static BLEUUID charUUID("0001");

// Przypisanie USART do konkretnych pinów
SoftwareSerial SerialGSM(16,17);
// Uproszczenie funkcji tft
TFT_eSPI tft = TFT_eSPI();
// Deklaracja pinów ESP32
const int buttonPin = 34;
const int ledPin = 21;
//const int BTbutton = 33;

// Deklaracja zmiennych do BLE
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
uint32_t counter = 0;
float buffer_temp;
int buffer_puls;

// Inicjalizacja funkcji
void updateSerial();
void sendSMS();
void upadek();
void wyswietlacz();
float temperatura();
int puls();

//Funkcje związanie z BLE
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    counter = (pData[0]<<24)|(pData[1]<<16)|(pData[2]<<8)|pData[3];
    Serial.print("data otrzymana z serwera: ");
    Serial.println(counter,HEX);
    upadek();
    wyswietlacz();

}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Tworzenie polaczenia do ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Stworzono klienta");

    pClient->setClientCallbacks(new MyClientCallback());

    pClient->connect(myDevice);
    Serial.println(" - Podlaczono do serwera");
    pClient->setMTU(517);
  
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Nie odnaleziono UUID uslugi: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Odnaleziono usluge");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Nie odnaleziono UUID charakterystki: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Odnaleziono charakterystyke");

    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("Wartosc charakterystyki: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
  void onResult(BLEAdvertisedDevice advertisedDevice) 
  {
    Serial.print("Urzadzenia rozglaszajace BLE: ");
    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }
  }
};

void setup()
{
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_WHITE);
    pinMode(buttonPin, INPUT_PULLUP);
    //pinMode(BTbutton, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin,LOW);
    Serial.begin(9600);
    SerialGSM.begin(9600);
    Serial.println("\n");
    Serial.println("Inicjalizacja...");
    delay(1000);
    SerialGSM.println("AT");
    updateSerial();
    Serial.println("Uruchomienie klienta BLE...");
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void loop()
{
    if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Polaczono z Serwerem BLE!");
    } else {
      Serial.println("Polaczenie sie nie powiodlo.");
    }
    doConnect = false;
  }

  if (connected) {

  }else if(doScan){
    BLEDevice::getScan()->start(0);
  }
  
  delay(1000);
}

void wyswietlacz()
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,4);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.println("Siema, siema");
    tft.print("BPM: "); tft.println(puls());
    //tft.print("Sp02: "); tft.println(rand() % 10 + 90);
    tft.print("Temperatura: "); tft.println(temperatura());
    delay(1000);
}

void upadek()
{
    if(counter == 0xAAFFFFFF)
    {
        Serial.println("Wykryto upadek!");
        sendSMS();
        while(digitalRead(buttonPin) == HIGH)
        {
            digitalWrite(ledPin, HIGH);
            tft.fillScreen(TFT_RED);
            delay(100);
            digitalWrite(ledPin, LOW);
            tft.fillScreen(TFT_BLACK);
            delay(100);
        }
        digitalWrite(ledPin, LOW);  
    }
    else
    {
      Serial.println("Nie wykryto upadku");
    }
}

void updateSerial()
{
    delay(500);
    while(Serial.available())
    {
        SerialGSM.write(Serial.read());
    }
    while(SerialGSM.available())
    {
        Serial.write(SerialGSM.read());
    }
    
}

void sendSMS()
{
    SerialGSM.println("AT+CMGF=1");
    updateSerial();
    SerialGSM.println("AT+CMGS=\"+48509642475\"");
    updateSerial();
    SerialGSM.println("WYKRYTO UPADEK!");
    updateSerial();
    SerialGSM.write(26);
}

float temperatura()
{
    uint8_t type1 = (counter >> 24) & 0xFF;
    float temp;
    Serial.print("temp: ");
    Serial.println(type1, HEX);
    if(type1 == 0xAC)
    {
        uint8_t d1 = (counter >> 8) & 0xFF;
        uint8_t d2 = counter & 0xFF;
        temp = float(d1) + float(d2) / 16.0f;
        Serial.print("Zmierzona temperatura: ");
        Serial.print(temp);
        Serial.println(" *C"); 
        buffer_temp = temp;
        return temp;
    }
    else
    {
      return buffer_temp;
    }
}

int puls()
{
    uint8_t type2 = (counter >> 24) &0xFF;
    uint8_t tetno;
    Serial.print("puls: ");
    Serial.println(type2, HEX);
    if(type2 == 0xAB)
    {
        tetno = counter & 0xFF;
        Serial.print("Zmierzony puls: ");
        Serial.print(tetno);
        Serial.println(" BPM");
        buffer_puls = tetno;
        return tetno;
    }
    else
    {
      return buffer_puls;
    }
}