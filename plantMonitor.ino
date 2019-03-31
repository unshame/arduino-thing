#include <Arduino.h>

// DHT sensor library - Version: 1.3.4
#include <DHT.h>
#include <DHT_U.h>

// LiquidCrystal - Version: 1.0.7
#include <LiquidCrystal.h>

#include "plantMonitor.h"
#include "plantMonitorConfig.h"

LiquidCrystal lcd(PIN_LCD_1, PIN_LCD_2, PIN_LCD_3, PIN_LCD_4, PIN_LCD_5, PIN_LCD_6);
DHT dht(PIN_D_DHT, DHT_TYPE);

Param soilHumidity {
  "Soil Humidity", "SH",
  SH_MIN, SH_MAX,
  "WATER THE PLANTS", "TOO MUCH WATER",
  true, 0,
  getSoilHumidity
};

Param ambientBrightness {
  "Ambient Brightness", "AB",
  -1, AB_MAX,
  "", "LIGHT TOO BRIGHT",
  false, 0,
  getAmbientBrightness
};

Param airHumidity {
  "Air Humidity", "AH",
  AH_MIN, AH_MAX,
  "AIR IS TOO DRY", "AIR IS TOO MOIST",
  false, 1,
  getAirHumidity
};

Param airTemperature {
  "Air Temperature", "AT",
  AT_MIN, AT_MAX,
  "AIR IS TOO COLD", "AIR IS TOO HOT",
  false, 1,
  getAirTemperature
};

Param params[] = {
  soilHumidity,
  ambientBrightness,
  airHumidity,
  airTemperature
};

const int numParams = sizeof(params) / sizeof(Param);

bool isLightOn = false;


/* SETUP */

void setup() {
  Serial.begin(PORT);  
  initOutput();  
  initRandom();
  initDisplay();
  delay(INIT_DELAY);
}


/* INIT */

void initOutput() {
  pinMode(PIN_D_BUZZER, OUTPUT);
  pinMode(PIN_D_LIGHT, OUTPUT);
}

void initRandom() {
  randomSeed(analogRead(0));
}

void initDisplay() {
  lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  lcd.print(DISPLAY_GREETING);
}


/* LOOP */

void loop() {

  updateAllValues();
  logAllValues();

  bool isLightTooLow = ambientBrightness.cachedValue < AB_MIN;
  
  if (isLightTooLow) {
    turnLightOn();
  }
  else {
    turnLightOff();
  }

  if (isAllParamsOk()) {   
    displayAllValues();
    delay(UPDATE_INTERVAL);
  }
  else {
    playAlert();    
  }  
  
}


/* LIGHT */

void turnLightOn() {
  
  if (!isLightOn) {
    digitalWrite(PIN_D_LIGHT, HIGH);
    isLightOn = true;
  } 
  
}

void turnLightOff() {
  
  if (isLightOn) {
    digitalWrite(PIN_D_LIGHT, LOW);
    isLightOn = false;
  }
  
}


/* SOUND */

void playAlert() {
  int rndStart = random(10, 100);
  int rndEnd = random(rndStart + 50, rndStart + 100);
  int delayDuration = random(5, 20);
  int numCycles = 3;
  
  for (int j = 0; j < numCycles; j++) {
    for (int i = rndStart; i <= rndEnd; i++) {
      tone(PIN_D_BUZZER, i * 10);
      delay(delayDuration);
    }

    for (int i = rndEnd; i >= rndStart; i--) {
      tone(PIN_D_BUZZER, i * 10);
      delay(delayDuration);
    }   
  }
  
  noTone(PIN_D_BUZZER);
}


/* CONSOLE */

void logAllValues() {
  
  for (int i = 0; i < numParams; i++) {
    logValue( params[i].name, params[i].cachedValue, params[i].decimalPlaces );
  }
  
  Serial.println();
}

void logValue(String str, float value, int decimalPlaces) {
  Serial.println(String(str + ": ") + String(value, decimalPlaces));
}


/* GET VALUES */

void updateAllValues() {
  
	for (int i = 0; i < numParams; i++) {
		params[i].cachedValue = params[i].readValue();
	}
	
}

float getSoilHumidity() {
  return (float)analogRead(PIN_A_SOILHUMIDITY);
}

float getAmbientBrightness() {
  return (float)analogRead(PIN_A_PHOTOSENSOR);
}

float getAirHumidity() {
  return dht.readHumidity();
}

float getAirTemperature() {
  return dht.readTemperature();
}


/* CHECK VALUES */

bool isAllParamsOk() {
  
  for (int i = 0; i < numParams; i++) {
    
		if ( !isParamOk(&params[i]) ) {
		  return false;
		}
		
	}
	
	return true;
}

bool isParamOk(Param *param) {

	if ( param->cachedValue < param->minValue ) {
		displayWarning( param->invertMinMax ? param->messageExceeding : param->messageLacking );
		return false;
	}
	else if ( param->cachedValue > param->maxValue ) {
		displayWarning( param->invertMinMax ? param->messageLacking : param->messageExceeding );
		return false;
	}

	return true;
}

/* DISPLAY */

void resetDisplay() {  
  lcd.clear();
  lcd.setCursor(0, 0);
}

void displayAllValues() {
  resetDisplay();
  
  for (int i = 0; i < numParams; i++) {
		displayValue( params[i].shortName, params[i].cachedValue, params[i].decimalPlaces );
		
		if (i % 2 != 0) {
		  displayLineEnd();
		}
		
	}
}

void displayWarning(String warning) {
  resetDisplay();
  
  lcd.print("WARNING!");
  displayLineEnd();
  lcd.print(warning);
}

void displayLineEnd() {
  lcd.setCursor(0, 1);
}

void displayValue(String abr, float value, int decimalPlaces) {  
  lcd.print(abr + ":" + String(value, decimalPlaces) + " ");
}
