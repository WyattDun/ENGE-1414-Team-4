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

unsigned long messageStartTime = 0;
const long messageDisplayTime = 30000; // 30 seconds for the message to be displayed

void setup() {
  Serial.begin(115200);
  pinMode(output5, OUTPUT);
  pinMode(externalButton, INPUT_PULLUP);
  digitalWrite(output5, LOW);
  servo1.attach(3); // Attach servo1 to pin GPIO4
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
              messageStartTime = millis(); // Record the start time of the message display
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
    client.println("<!DOCTYPE html><html>");

    // HTML Head with Style
    client.println("<head>");
    client.println("<style>");
    client.println("body {background-color: #9E1C32; color: white; font-family: Arial, sans-serif;}");
    client.println(".container {width: 80%; margin: auto;}");
    client.println(".prompt-box {background-color: #333; padding: 20px; margin-top: 20px;}");
    client.println("</style>");
    client.println("</head>");

    // HTML Body
    client.println("<body>");
    client.println("<div class=\"container\">");
    client.println("<h1>Hokie-Passport Locker System</h1>");

    // HTML Form for User Input with JavaScript Refresh
    client.println("<div class=\"prompt-box\">");
    client.println("<form method=\"get\" action=\"/submit\" onsubmit=\"refreshPage()\">");
    client.println("Enter your Hokie-Passport number:");
    client.println("<input type=\"text\" name=\"number\" maxlength=\"9\" pattern=\"[0-9]{9}\" required>");
    client.println("<input type=\"submit\" value=\"Submit\">");
    client.println("</form>");
    client.println("<script>function refreshPage() {location.reload();}</script>");
    client.println("</div>");

    // Display Locker Open Message
    if (servoOpen) {
      client.println("<div class=\"prompt-box\">");
      client.print("<p>Locker Number ");
      client.print(servoIndex + 1);  // Adding 1 because servoIndex is zero-based
      client.println(" is now open.</p>");
      client.print("<p>Please close the locker when you're done.</p>");
      client.println("</div>");

      // Check if 30 seconds have passed since the message started displaying
      if (millis() - messageStartTime >= messageDisplayTime) {
        servoOpen = false; // Close the locker automatically after 30 seconds
      }
    }

    client.println("</div>");
    client.println("</body></html>");
    client.println();
    client.stop();
  }

  // Check if the servo needs to move, prevents micro-stuttering and improves security
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
