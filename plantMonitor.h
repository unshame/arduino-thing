struct Param {
    const String name;
    const String shortName;
    const int minValue;
    const int maxValue;
    const String messageExceeding;
    const String messageLacking;
    const bool invertMinMax;
    const int decimalPlaces;

    float (*readValue)();

    float cachedValue;
};

/* SETUP */
void setup();

/* INIT */
void initOutput();
void initRandom();
void initDisplay();

/* LOOP */
void loop();

/* LIGHT */
void turnLightOn();
void turnLightOff();

/* SOUND */
void playAlert();

/* CONSOLE */
void writeAllValues();
void writeValue(String str, float value, int decimalPlaces);

/* GET VALUES */
void updateAllValues();
float getSoilHumidity();
float getAmbientBrightness();
float getAirHumidity();
float getAirTemperature();

/* CHECK VALUES */
bool isAllParamsOk();
bool isParamOk(Param *param);

/* DISPLAY */
void resetDisplay();
void displayAllValues();
void displayWarning(String warning);
void displayLineEnd();
void displayValue(String abr, float value, int decimalPlaces);
