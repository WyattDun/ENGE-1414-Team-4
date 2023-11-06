#include <ESP8266WiFi.h>
#include <Servo.h>

const char* ssid = "Wyatt";
const char* password = "6uiyc3ytqi1k0";

Servo myservo;
WiFiServer server(80);

const int output5 = 5;
const int externalButton = 2;

bool servoOpen = false;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

// Define your valid numbers
String validNumbers[] = {
  "123456789", // Add your valid 9-digit numbers here
  "987654321",
  "906588687"
};

void setup() {
  Serial.begin(115200);
  pinMode(output5, OUTPUT);
  pinMode(externalButton, INPUT_PULLUP);
  digitalWrite(output5, LOW);
  myservo.attach(0);
  myservo.write(0);

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
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();

      if (client.available()) {
        char c = client.read();
        currentLine += c;
        if (c == '\n') {
          if (currentLine.startsWith("GET /submit?number=")) {
            String number = currentLine.substring(19, 28); // Extract number from GET request
            number.trim(); // Remove leading and trailing spaces
            number.replace("\n", ""); // Remove newline characters, if any
            Serial.println("Input Number: " + number);
            if (isValidNumber(number)) {
              Serial.println("Valid Number");
              myservo.write(90);
              servoOpen = true;
            } else {
              Serial.println("Invalid Number");
            }
          }
          break;
        }
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><body>");
    client.println("<form method=\"get\" action=\"/submit\">");
    client.println("Enter a 9-digit number: <input type=\"text\" name=\"number\" maxlength=\"9\" pattern=\"[0-9]{9}\" required><br>");
    client.println("<input type=\"submit\" value=\"Submit\">");
    client.println("</form>");
    client.println("</body></html>");
    client.println();
    client.stop();
  }

  // Check for external button press to close the servo
  if (digitalRead(externalButton) == LOW && servoOpen) {
    myservo.write(0);
    servoOpen = false;
  }
}

bool isValidNumber(String number) {
  for (int i = 0; i < sizeof(validNumbers) / sizeof(validNumbers[0]); i++) {
    if (number == validNumbers[i]) {
      return true;
    }
  }
  return false;
}
