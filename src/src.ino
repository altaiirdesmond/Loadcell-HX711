#include <HX711_ADC.h>

HX711_ADC loadcell(4, 5);

float calibrationFactor = 680; // Change this

const int RELAYPIN0 = 6;
const int RELAYPIN1 = 7;
const int REEDSWITCHPIN = 8;

boolean extracting = false;
boolean caseOpen = false;

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

	if (digitalRead(REEDSWITCHPIN) == HIGH) {
		digitalWrite(RELAYPIN0, HIGH);
		digitalWrite(RELAYPIN1, HIGH);

		caseOpen = true;
	}
	else {
		caseOpen = false;
	}
}

void loop() {
	// Case open
	if (digitalRead(REEDSWITCHPIN) == HIGH) {
		digitalWrite(RELAYPIN0, HIGH);
		digitalWrite(RELAYPIN1, HIGH);

		caseOpen = true;
	}
	else {
		caseOpen = false;
	}

	if (Serial.available() && !caseOpen) {
		char data[20];
		strcpy(data, Serial.readString().c_str());

		// If should start extraction
		if (strstr(data, "extract_") != nullptr) {
			extracting = true;

      // Turn machine on
			digitalWrite(RELAYPIN0, LOW);
      digitalWrite(RELAYPIN1, LOW);

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
          // Turn off ALL relays
					StopExtract("HARDRESET");
				}

				// Non-blocking
				char raw[10];
				int index = 0;
				while (Serial.available()) {
					raw[index++] = Serial.read();
				}

				if (strstr(raw, "relay off") != nullptr) {
					StopExtract("HARDRESET");
				}
				else if (strstr(raw, "stop detach") != nullptr) {
					StopExtract("SOFTRESET");
				}

				if (digitalRead(REEDSWITCHPIN) == HIGH) {
					StopExtract("HARDRESET");
				}
			}
		}
	}
}

void StopExtract(char *mode) {
	if (mode == "HARDRESET") {
		// Turn off machine
		digitalWrite(RELAYPIN0, HIGH);
		digitalWrite(RELAYPIN1, HIGH);
	}
	else {
		digitalWrite(RELAYPIN0, HIGH);
	}

	Serial.print("Resetting...");

	// Wait for loadcell to go back to 0
	while (loadcell.getData() >= 0) {
		loadcell.update();
	}

	Serial.println("OK");

	extracting = false;
}
