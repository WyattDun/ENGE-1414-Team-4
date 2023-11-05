#include <ESP8266WiFi.h>
#include <Servo.h>

const char* ssid = "Wyatt";
const char* password = "6uiyc3ytqi1k0";

WiFiServer server(80);
String header;
String output5State = "off";
String output4State = "off";
const int output5 = 5;
const int output4 = 4;

Servo myservo;
int servoPin = 5;
String database[] = {"123456789", "987654321", "147258369"};

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  myservo.attach(servoPin);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /9/") >= 0) {
              String hokiePNumber = header.substring(header.indexOf("GET /9/") + 6, header.indexOf(" HTTP/1.1"));
              Serial.println("Hokie-P Number: " + hokiePNumber);
              if (checkDatabase(hokiePNumber)) {
                client.println("<p>Access granted. Operating the servo.</p>");
                operateServo();
              } else {
                client.println("<p>Access denied.</p>");
              }
            }
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            client.println("<body><h1>ESP8266 Web Server</h1></body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

bool checkDatabase(String input) {
  for (int i = 0; i < sizeof(database) / sizeof(database[0]); i++) {
    if (input.equals(database[i])) {
      return true;
    }
  }
  return false;
}

void operateServo() {
  myservo.write(90);
  delay(60000);
  myservo.write(0);
}
