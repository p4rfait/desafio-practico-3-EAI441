#include "WiFiS3.h"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiServer server(80);
int status = WL_IDLE_STATUS;

// PINES:
const int motorPin2 = 2;
const int motorPin3 = 3;

const int ultrasonicTrigPin = 10;
const int ultrasonicEchoPin = 9;

bool manualControl = false;

enum spinMotorDirection {
  Left,
  Right,
  Stop
};

spinMotorDirection currentMotorSpinDirection = Stop;

struct DistanceRange {
  float min;
  float max;
};

// TODO: Reemplazar a valores de la guia (No tengo 3 metros de cuarto lol)
DistanceRange targetDistances[] = {
  { 100.00, 200.00 },
  { 200.00, 300.00 }
};

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  pinMode(ultrasonicTrigPin, OUTPUT);
  pinMode(ultrasonicEchoPin, INPUT);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  server.begin();
  printWifiStatus();
}

void loop() {
  /*float distance = getUltrasonicSensorcurrentMotorSpinDirectionDistance();
  //Serial.println(distance);
  if (distance >= targetDistances[0].min && distance <= targetDistances[0].max) {
    //Serial.println("Entre "+ String(targetDistances[0].min) + " y " + String(targetDistances[0].max));
    spinMotor(Right);
  } else if (distance > targetDistances[1].min && distance <= targetDistances[1].max) {
    //Serial.println("Entre "+ String(targetDistances[1].min) + " y " + String(targetDistances[1].max));
    spinMotor(Left);
  } else {
    //Serial.println("Fuera de rango");
    spinMotor(Stop);
  };
  */
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    String currentLine = "";
    bool sendData = false;
    while (client.connected()) {
      delayMicroseconds(10);
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (sendData) {
              float distance = getUltrasonicSensorDistance();
              /*client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Access-Control-Allow-Origin: *");
              client.println();
              client.println("<h1>Sensor Ultrasonico:</h1>");
              client.print("<p>Rango de distancias 1: ");
              client.print(targetDistances[0].min);
              client.print(" cm");
              client.print(" - ");
              client.print(targetDistances[0].max);
              client.println(" cm</p>");
              client.print("<p>Rango de distancias 2: ");
              client.print(targetDistances[1].min);
              client.print(" cm");
              client.print(" - ");
              client.print(targetDistances[1].max);
              client.println(" cm</p>");
              client.println("<p>Distancia Actual: " + String(distance) + " cm</p>");

              client.println("<h1>Motor:</h1>");
              client.print("<p>Modo: ");
              if (manualControl == true) {
                client.println("Manual</p>"); }
              else {
                client.println("Automatico</p>");}
              client.print("<p>Direccion: ");
              switch (currentMotorSpinDirection) {
                case Left:
                  client.println("Izquierda</p>");
                  break;
                case Right:
                  client.println("Derecha</p>");
                  break;
                case Stop:
                  client.println("Detenido</p>");
                  break;
                default:
                  client.println("?</p>");
                  break;
              }
              client.println();
              */
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: application/json");
              client.println("Access-Control-Allow-Origin: *");
              client.println();

              client.print("{");
              client.print("\"distancia\":");
              client.print(distance, 2);
              client.print(",");

              client.print("\"modo\":\"");
              if (manualControl)
                client.print("Manual");
              else client.print("Automatico");
              client.print("\",");

              client.print("\"direccion\":\"");
              switch (currentMotorSpinDirection) {
                case Left:
                  client.print("Izquierda");
                  break;
                case Right:
                  client.print("Derecha");
                  break;
                case Stop:
                  client.print("Detenido");
                  break;
              }
              client.println("\"}");
            } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.print("<h1>Opciones:</h1>");
              client.print("<h2>/L</h2>");
              client.print("<p>Cambiar a modo manual y rotar motor hacia la izquierda</p>");
              client.print("<h2>/R</h2>");
              client.print("<p>Cambiar a modo manual y rotar motor hacia la derecha</p>");
              client.print("<h2>/S</h2>");
              client.print("<p>Cambiar a modo manual y detiente la rotacion del motor</p>");
              client.print("<h2>/MANUAL</h2>");
              client.print("<p>Lo mismo que /S</p>");
              client.print("<h2>/AUTO</h2>");
              client.print("<p>El motor se mueve con respecto a las distancia indicadas</p>");
              client.print("<h2>/DATA</h2>");
              client.print("<p>Obtiene informacion sobre el sistema</p>");
              client.println();
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        if (currentLine.endsWith("GET /L")) {
          manualControl = true;
          spinMotor(Left);
        }
        if (currentLine.endsWith("GET /R")) {
          manualControl = true;
          spinMotor(Right);
        }
        if (currentLine.endsWith("GET /S") || currentLine.endsWith("GET /MANUAL")) {
          manualControl = true;
          spinMotor(Stop);
        }
        if (currentLine.endsWith("GET /AUTO")) {
          manualControl = false;
        }
        if (currentLine.endsWith("GET /DATA")) {
          sendData = true;
        }
      }
    }
    delay(1);

    client.stop();
    Serial.println("client disconnected");
  }

  float distance = getUltrasonicSensorDistance();
  //Serial.println(distance);
  if (!manualControl) {
    if (distance >= targetDistances[0].min && distance <= targetDistances[0].max) {
      //Serial.println("Entre "+ String(targetDistances[0].min) + " y " + String(targetDistances[0].max));
      spinMotor(Right);
    } else if (distance > targetDistances[1].min && distance <= targetDistances[1].max) {
      //Serial.println("Entre "+ String(targetDistances[1].min) + " y " + String(targetDistances[1].max));
      spinMotor(Left);
    } else {
      //Serial.println("Fuera de rango");
      spinMotor(Stop);
    };
  }
}

float getUltrasonicSensorDistance() {
  float timing = 0.0;
  float distance = 0.0;

  digitalWrite(ultrasonicTrigPin, LOW);
  delay(2);

  digitalWrite(ultrasonicTrigPin, HIGH);
  delay(10);
  digitalWrite(ultrasonicTrigPin, LOW);

  timing = pulseIn(ultrasonicEchoPin, HIGH);
  distance = (timing * 0.0343) / 2;

  return distance;
}

void spinMotor(enum spinMotorDirection direction) {
  switch (direction) {
    case Left:
      digitalWrite(motorPin3, HIGH);
      digitalWrite(motorPin2, LOW);
      currentMotorSpinDirection = Left;
      break;
    case Right:
      digitalWrite(motorPin3, LOW);
      digitalWrite(motorPin2, HIGH);
      currentMotorSpinDirection = Right;
      break;
    case Stop:
      digitalWrite(motorPin3, LOW);
      digitalWrite(motorPin2, LOW);
      currentMotorSpinDirection = Stop;
      break;
  };
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
