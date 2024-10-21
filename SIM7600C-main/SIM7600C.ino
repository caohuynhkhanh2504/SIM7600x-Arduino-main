#define TINY_GSM_MODEM_SIM7600 // define SIM

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define MODEM_EN 15  // Chân EN của SIM7600
#define MODEM_TX 17  // Chân TX
#define MODEM_RX 16  // Chân RX
#define MODEM_BAUD 115200

// Khai báo thông tin của nhà mạng
const char phone_number[] = "+84942570530";
const char apn[] = "v-internet";  // APN mạng viettel
const char user[] = "";          // User
const char pass[] = "";          // Pass
char sim_data[100];

// Thông tin MQTT
const char *mqtt_server = "test.mosquitto.org";  // Broker
const char *mqtt_user = "";        // User
const char *mqtt_pass = "";        // Pass
const char *mqtt_topic = "topic/huka";                // Topic
char mqtt_data[100];

// Khởi tạo modem và client
TinyGsm modem(Serial2);          // Sử dụng Serial2 để giao tiếp với SIM7600
TinyGsmClient gsmClient(modem);
PubSubClient mqttClient(gsmClient);


void reconnect() {
  // Nếu chưa kết nối, cố gắng kết nối
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Tạo một ID cho client
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Cố gắng kết nối
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // subscribe topic
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      if(mqttClient.state() == -2)
      {
        Serial.println("Check if the broker is online and the address is correct.");
        reconnectGPRS();
      }
      delay(5000);
    }
  }
}

void reconnectGPRS() {
  Serial.println("Reconnecting GPRS...");
  modem.gprsDisconnect(); // Ngắt kết nối GPRS
  delay(1000); 
  modem.gprsConnect(apn, user, pass); // Kết nối lại GPRS

  // Kiểm tra lại kết nối
  if (modem.isNetworkConnected()) {
    Serial.println("Reconnected to the network.");
  } else {
    Serial.println("Failed to reconnect to the network.");
  }
}

void sendSMS(const char *message) {
  Serial.println("Sending SMS...");
  modem.sendSMS(phone_number, message);  // Gửi tin nhắn SMS
  Serial.println("SMS sent.");
}

void makeCall(const char *number) {
  Serial.print("Calling... ");
  modem.callNumber(number);  // Gọi số điện thoại
  Serial.println("Call initiated.");
}

bool checkInternet() {
    Serial.println("Checking internet connection...");
    
    // check internet
    modem.sendAT("+PING=\"8.8.8.8\""); // IP Google DNS
    if (modem.waitResponse(2000) > 0) {
        Serial.println("Internet connection is OK.");
        return true;
    } else {
        Serial.println("No internet connection.");
        return false;
    }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Create modem
  Serial2.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);  
  modem.restart();
  modem.gprsConnect(apn, user, pass);

  // check connect
  if (modem.isNetworkConnected()) {
    Serial.println("Connected to the network.");
  } else {
    Serial.println("Failed to connect to the network.");
    while (true);
  }
 // strcpy(sim_data, "Hung kem mapdjt");
 // sendSMS("sim_data");
  delay(5000);  //delay truoc khi gọi
  makeCall(phone_number); 
  delay(10000);   // đảm bảo gọi xong thì mới reconnect được
  mqttClient.setServer(mqtt_server, 1883);
}


void loop() {
  if (!mqttClient.connected()) {
    reconnectGPRS();  //mạng
    if(checkInternet())
    {
      delay(5000);
      reconnect();    //mqtt
    }
    else
    {
      Serial.println("Still no internet connection, skipping MQTT reconnect.");
    }
  }
  mqttClient.loop();

  // pub lên MQTT
  strcpy(mqtt_data, "Cao Huynh Khanh");
  mqttClient.publish(mqtt_topic, mqtt_data);
  
  delay(1000); // delay 1s
}
