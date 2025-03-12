// include the LED Matrix library from the Uno R4 core:
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// make an instance of the library:
ArduinoLEDMatrix matrix;

//include the "animation.h" header file that stores the frames for the animation
#include "animation.h"

unsigned long previousMillisLED = 0;
unsigned long previousMillisScroll = 0;
const long ledInterval = 1000;  // intervallo di lampeggio LED (1 secondo)
const long scrollInterval = 50; // velocità di scorrimento testo

bool ledState = LOW;
int scrollPosition = 0;
const char text[] = "Apertura Cancello";

void setup() {
  Serial.begin(115200);

  //load frames from the animation.h file
  matrix.loadSequence(frames);
  // start the matrix
  matrix.begin();

  //define LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  // Gestione lampeggio LED
  if (currentMillis - previousMillisLED >= ledInterval) {
    previousMillisLED = currentMillis;
    ledState = !ledState; // Inverti stato LED
    digitalWrite(LED_BUILTIN, ledState);
  }

  // Gestione scrolling testo
  if (currentMillis - previousMillisScroll >= scrollInterval) {
    previousMillisScroll = currentMillis;
    
    matrix.beginDraw();
    matrix.clear(); // Pulisce la matrice prima di disegnare il nuovo frame
    matrix.textFont(Font_5x7);
    matrix.beginText(scrollPosition, 1, 0xFFFFFF);
    matrix.println(text);
    matrix.endText();
    matrix.endDraw();

    // Sposta la posizione del testo
    scrollPosition--;

    // Se il testo è completamente fuori dalla matrice, ricomincia da capo
    if (scrollPosition < -((int)strlen(text) * 6)) {
      scrollPosition = matrix.width();
    }
  }
}
