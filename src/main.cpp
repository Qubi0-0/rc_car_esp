#include <Arduino.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <NimBLEDevice.h>

#define PWM 13
#define MOTOR_IN_1 12
#define MOTOR_IN_2 14
#define SERVO_IN 27
#define ECHO 25
#define TRIG 26
#define DISTANCE_TRESHOLD 20 // cm
#define EXE_INTERVAL 5000

// PWM (0-255) - speed of motor
const int motor_speed = 255/3;
const float distance_const = 0.034/2;


unsigned long lastExecutedMillis = 0; 
int servo_position, duration, distance;

static NimBLEServer* pServer;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };

    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());

        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };
    
    uint32_t onPassKeyRequest(){
        Serial.println("Server Passkey Request");

        return 123456; 
    };

    bool onConfirmPIN(uint32_t pass_key){
        Serial.print("The passkey YES/NO number: ");Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true; 
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        /** Check that encryption was successful, if not we disconnect the client */  
        if(!desc->sec_state.encrypted) {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };
    /** Called before notification or indication is sent, 
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", "; 
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};
    
/** Define callback instances globally to use for multiple Charateristics \ Descriptors */ 
static CharacteristicCallbacks chrCallbacks;

Servo myservo;
String voice_command; 
NimBLECharacteristicCallbacks Character;


void setup() {
  Serial.begin(115200);
  pinMode(PWM,OUTPUT) ;  	//we have to set PWM pin as output
  pinMode(MOTOR_IN_1,OUTPUT) ; 	//Logic pins are also set as output
  pinMode(MOTOR_IN_2,OUTPUT) ;  // Sets 
  pinMode(TRIG, OUTPUT); // Sets the TRIGPin as an OUTPUT
  pinMode(ECHO, INPUT); // Sets the ECHOPin as an INPUT
  myservo.attach(SERVO_IN);
// BLE CONFIG

  Serial.println("Starting NimBLE Server");

    /** sets device name */
    NimBLEDevice::init("RC_Car");

    /** Optional: set the transmit power, default is 3db */
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    
    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pDeadService = pServer->createService("DEAD");
    NimBLECharacteristic* pBeefCharacteristic = pDeadService->createCharacteristic(
                                               "BEEF",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                               /** Require a secure connection for read and write access */
                                               NIMBLE_PROPERTY::READ_ENC |  // only allow reading if paired / encrypted
                                               NIMBLE_PROPERTY::WRITE_ENC   // only allow writing if paired / encrypted
                                              );
  
    pBeefCharacteristic->setValue("Burger");
    pBeefCharacteristic->setCallbacks(&chrCallbacks);

    NimBLE2904* pBeef2904 = (NimBLE2904*)pBeefCharacteristic->createDescriptor("2904"); 
    pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
  

    NimBLEService* pBaadService = pServer->createService("BAAD");
    NimBLECharacteristic* pFoodCharacteristic = pBaadService->createCharacteristic(
                                               "F00D",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::NOTIFY
                                              );

    pFoodCharacteristic->setValue("Fries");
    pFoodCharacteristic->setCallbacks(&chrCallbacks);

    /** Custom descriptor: Arguments are UUID, Properties, max length in bytes of the value */
    NimBLEDescriptor* pC01Ddsc = pFoodCharacteristic->createDescriptor(
                                               "C01D",
                                               NIMBLE_PROPERTY::READ | 
                                               NIMBLE_PROPERTY::WRITE|
                                               NIMBLE_PROPERTY::WRITE_ENC, // only allow writing if paired / encrypted
                                               20
                                              );
    pC01Ddsc->setValue("Send it back!");

    /** Start the services when finished creating all Characteristics and Descriptors */  
    pDeadService->start();
    pBaadService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pDeadService->getUUID());
    pAdvertising->addServiceUUID(pBaadService->getUUID());

    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");

    myservo.write(servo_position = 90);
    analogWrite(PWM,motor_speed);
}

// the loop function runs over and over again forever
void loop() {
  unsigned long currentMillis = millis();
  // Clears the TRIGPin
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  // Sets the TRIGPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  // Reads the ECHOPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO, HIGH);
  // Calculating the distance
  distance = duration * distance_const;

  /** Do your thing here, this just spams notifications to all connected clients */
    if(pServer->getConnectedCount()) {
        NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
        if(pSvc) {
            NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
            if(pChr) {
              voice_command = pChr->getValue().c_str();
              pChr->setValue("No Command");
              Serial.println(voice_command);

              if (distance <= DISTANCE_TRESHOLD) { // Stops the RC car if too near obstacle
              digitalWrite(MOTOR_IN_1,LOW) ;
              digitalWrite(MOTOR_IN_2,LOW) ;
 
              }
              else if (voice_command == "forward" || voice_command == "Forward") {
                myservo.write(servo_position = 90);
                digitalWrite(MOTOR_IN_1,HIGH) ;
                digitalWrite(MOTOR_IN_2,LOW) ;
                analogWrite(PWM,motor_speed) ; 
                }
              else if (voice_command == "left" || voice_command == "Left") {
                myservo.write(servo_position = 120);
                delay(100);
                digitalWrite(MOTOR_IN_1,HIGH) ;
                digitalWrite(MOTOR_IN_2,LOW) ;
                }
              else if (voice_command == "right" || voice_command == "Right") {
                myservo.write(servo_position = 60);
                delay(100);
                digitalWrite(MOTOR_IN_1,HIGH) ;
                digitalWrite(MOTOR_IN_2,LOW) ;
                }
              else if (voice_command == "backward" || voice_command == "Backward") {
                myservo.write(servo_position = 90);
                delay(100);
                digitalWrite(MOTOR_IN_1,LOW) ;
                digitalWrite(MOTOR_IN_2,HIGH) ;
              }
              else if (voice_command == "stop" || voice_command == "Stop") {
                myservo.write(servo_position = 90);
                delay(100);
                digitalWrite(MOTOR_IN_1,LOW) ;
                digitalWrite(MOTOR_IN_2,LOW) ;
              }
            }
          } 
    }

  if (currentMillis - lastExecutedMillis >= EXE_INTERVAL) {
  lastExecutedMillis = currentMillis; // save the last executed time
  digitalWrite(MOTOR_IN_2,LOW) ; 
  digitalWrite(MOTOR_IN_1,LOW) ;
  myservo.write(servo_position = 90);
  }

}