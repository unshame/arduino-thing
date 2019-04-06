/* 
 * Plant Monitor
 * Модуль предназначен для мониторинга параметров воздуха, почвы и освещения.
 * Облегчает уход за домашними растениями, оповещая пользователя о неприемлемых для растений условиях существования.
 * 
 * Заголовочный файл.
 *
 * Авторы: Шибаев О.Е и Ефремов Д.Е.
 * Дата последнего изменения указана в репозитории проекта: https://github.com/unshame/arduino-thing/blob/master/plantMonitor.h   
 * Информация о лицензировании находится в файле LICENSE.txt 
 */

// Структура, хранящая информацию о считываемых параметрах
struct Param {
    const String name;             // Полное название параметра для вывода в консоль
    const String shortName;        // Сокращенное название параметра для отображения на экране
    const int minValue;            // Минимальное приемлемое значение параметра
    const int maxValue;            // Максимальное приемлемое значение параметра
    const String messageLacking;   // Сообщение, выводимое на экран, когда значение параметра меньше минимального
    const String messageExceeding; // Сообщение, выводимое на экран, когда значение параметра больше максимального
    const bool invertMinMax;       // Меняет местами messageLacking и messageExceeding
    const int decimalPlaces;       // Сколько знаков после точки будет выводиться на экран для значения параметра

    // Функция для считывания значения параметра - возвращает значение в виде числа с плавающей точкой
    float (*readValue)();

    // Здесь будет кешироваться значение, возвращенное из readValue, чтобы не считывать значение несколько раз
    float cachedValue;
};

/* 
 * Далее идут Прототипы всех функций, использующихся в программе.
 * Комментарии смотреть в файле реализации.
 */

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
