#include <HX711_ADC.h>

HX711_ADC loadcell(4, 5);

float calibrationFactor = 680; // Change this

const int RELAYPIN0 = 6;
const int RELAYPIN1 = 7;
const int REEDSWITCHPIN = 8;

boolean extracting = false;

void setup() { 
	Serial.begin(9600); 

	loadcell.begin();
	loadcell.start(2000);
	loadcell.setCalFactor(calibrationFactor);

	pinMode(RELAYPIN0, OUTPUT);
	pinMode(RELAYPIN1, OUTPUT);
	pinMode(REEDSWITCHPIN, OUTPUT);

	digitalWrite(RELAYPIN0, HIGH);
	digitalWrite(RELAYPIN1, HIGH);
}

void loop() {
	if (Serial.available()) {
		char data[20];
		strcpy(data, Serial.readString().c_str());

		// If should start extraction
		if (strstr(data, "extract_") != nullptr) {
			extracting = true;

			digitalWrite(RELAYPIN0, LOW);

			// Split
			char *pch = strtok(data, "_");

			char *weightInput;

			while (pch != nullptr) {
				pch = strtok(nullptr, "_");

				// Check for weight input
				if (isdigit(*pch)) {
					weightInput = pch;
				}
			}

			// Convert to integer
			int weightInputToInt = atoi(weightInput);

			while (extracting) {
				loadcell.update();
				float weight = loadcell.getData();
				Serial.print("extracting:");
				Serial.println(weight);

				// Target weight reached
				if (weight >= weightInputToInt) {
					// Turn off machine
					digitalWrite(RELAYPIN0, HIGH);

					Serial.print("Resetting...");

					// Wait for loadcell to go back to 0
					while (loadcell.getData() >= 0) {
						loadcell.update();
					}

					Serial.println("OK");

					extracting = false;
				}
			}
		}
	}
}