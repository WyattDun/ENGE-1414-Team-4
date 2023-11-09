#include <ESP8266WiFi.h>
#include <Servo.h>

const char* ssid = "Wyatt";
const char* password = "6uiyc3ytqi1k0";

Servo servo1;
Servo servo2;
Servo servo3;
WiFiServer server(80);

const int output5 = 5;
const int externalButton = 2;

bool servoOpen = false;
int servoIndex = -1; // Variable to store the index of the open servo
String number; // Declare number outside the if block

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

String validNumbers[] = {
  "906588685", 
  "906588686",
  "906588687"
};

void setup() {
  Serial.begin(115200);
  pinMode(output5, OUTPUT);
  pinMode(externalButton, INPUT_PULLUP);
  digitalWrite(output5, LOW);
  servo1.attach(4); // Attach servo1 to pin GPIO4
  servo2.attach(16); // Attach servo2 to pin GPIO16
  servo3.attach(0); // Attach servo3 to pin GPIO0

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
            number = currentLine.substring(19, 28); // Extract number from GET request
            number.trim(); // Remove leading and trailing spaces
            number.replace("\n", ""); // Remove newline characters, if any
            Serial.println("Input Number: " + number);
            if (isValidNumber(number)) {
              Serial.println("Valid Number");
              // Determine the index of the valid number and open the corresponding servo
              if (number == validNumbers[0]) {
                servo1.write(90);
                servoIndex = 0;
              } else if (number == validNumbers[1]) {
                servo2.write(90);
                servoIndex = 1;
              } else if (number == validNumbers[2]) {
                servo3.write(90);
                servoIndex = 2;
              }
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

  // Check if the servo needs to move, prevents microstuddering and improves security
  if (servoOpen) {
    if (servoIndex == 0 && abs(servo1.read() - 90) > 2) {
      servo1.write(90);
    } else if (servoIndex == 1 && abs(servo2.read() - 90) > 2) {
      servo2.write(90);
    } else if (servoIndex == 2 && abs(servo3.read() - 90) > 2) {
      servo3.write(90);
    }
  }

  // Check for external button press to close the servo once the locker door is closed
  if (digitalRead(externalButton) == LOW && servoOpen) {
    if (servoIndex == 0) {
      servo1.write(0);
    } else if (servoIndex == 1) {
      servo2.write(0);
    } else if (servoIndex == 2) {
      servo3.write(0);
    }
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
