// Librariile
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Dimensiune la oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//definim servomotoarele, oled si variabilele
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo servo1;
Servo servo2;

int parc = 3;
int servo1State = 0, servo2State = 0;

// variabilele la fiecare senzor de ultrasunet pentru reducerea codului
const int trigPins[] = {4, 7, 9, 11, 12};
const int echoPins[] = {5, 6, 8, 10, 13};

long durations[5];
int distances[5];

// actualizeaza display la oled numai cand este apelata functia
void updateDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 5);           // x=0, y=10
  display.print("Parcare liber: " + String(parc));

  drawParkingSlots();
  display.display();
}

// locuri de parcari care deseneaza la display, inclusa in functia updateDisplay
void drawParkingSlots() {
  int x[] = {10, 50, 90}; // Slot positions

  for (int i = 0; i < 3; i++) {
    int d = distances[i + 2]; // S3, S4, S5 sunt locuri de parcari

    if (d <= 5) {
      display.fillRect(x[i], 25, 30, 30, SSD1306_WHITE); // locuri ocupate
    } else {
      display.drawRect(x[i], 25, 30, 30, SSD1306_WHITE); // locuri libere
    }
  }
  display.display();
}

// servomotor care ridica bariera 
void openServo(Servo &servo, int &servoState) {
  for (int pos = 0; pos <= 90; pos++) {
    servo.write(pos);
    delay(15);
  }
  servoState = 1;
}

// servomotor coboara bariera
void closeServo(Servo &servo, int &servoState) {
  for (int pos = 90; pos >= 0; pos--) {
    servo.write(pos);
    delay(15);
  }
  servoState = 0;
}

//initializam pentru a fi definite, incluzand serial pentru a vedea partea de debug
void setup() {
  servo1.attach(2);
  servo2.attach(3);

  for (int i = 0; i < 5; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found"));
    for (;;); // Stop if OLED is not found
  }
  updateDisplay();
  closeServo(servo1, servo1State);
}

void loop() {

  //preia fiecare ultrasunet numai daca una din ele detecteaza o distanta
  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPins[i], LOW);
    delayMicroseconds(2);
    digitalWrite(trigPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[i], LOW);

    durations[i] = pulseIn(echoPins[i], HIGH);
    distances[i] = durations[i] * 0.034 / 2;
  }

//daca ultrasunetul de intrare se afla intre 0cm-5cm, atunci ridica bariera si actualizeaza display-ul
  if (distances[0] >= 0 && distances[0] <= 5 && servo1State == 0) {
    if (parc == 0) {
      updateDisplay();
    } else {
      openServo(servo1, servo1State);
    }
  }

//daca ultrasunetul de iesire se afla intre 0cm-5cm, atunci ridica bariera si actualizeaza display-ul
  if (distances[1] >= 0 && distances[1] <= 5 && servo2State == 0) {
    if (parc > 0) {
      updateDisplay();
    } else {
      openServo(servo2, servo2State);
    }
  }

//daca ultrasunetul de intrare a intrat in stare de deschidere, atunci ocupa un loc de parcare
  if (distances[0] > 5 && servo1State == 1) {
    closeServo(servo1, servo1State);
    parc--;
    updateDisplay();
  }

//daca ultrasunetul de iesire a intrat in stare de deschidere, atunci elibereaza un loc de parcare
  else if (distances[1] > 5 && servo2State == 1) {
    closeServo(servo2, servo2State);
    parc++;
    updateDisplay();
  }

// daca parcarea e mai mare decat 0, va actualiza starea locurilor de parcare
  if (parc > 0) {
    updateDisplay();
  }

// daca una din locuri de parcare a detectat o distanta, o va considera ocupata
  if (distances[2] <= 5 || distances[3] <= 5 || distances[4] <= 5){
    updateDisplay();
  } 

// parte de debug pentru ultrasunete
  Serial.println("--- Sensor Distances ---");
  for (int i = 0; i < 5; i++) {
    Serial.print("S");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.println(" cm");
  }
  Serial.println("========================\n");
  delay(500);
}