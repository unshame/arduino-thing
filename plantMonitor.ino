/* 
 * Plant Monitor
 * Модуль предназначен для мониторинга параметров воздуха, почвы и освещения.
 * Облегчает уход за домашними растениями, оповещая пользователя о неприемлемых для растений условиях существования.
 *
 * Используемые датчики: 
 *  - Датчик температуры и влажности воздуха DHT22
 *  - Датчик влажности почвы
 *  - Датчик яркости освещения (фоторезистор) 
 *  
 * Используемые переферийные устройства:
 *  - LCD дисплей, способный выводить две строки текста
 *  - Светодиод
 *  - Пьезоизлучатель (спикер)
 *
 * Файл реализации.
 *
 * Авторы: Шибаев О.Е и Ефремов Д.Е.
 * Дата последнего изменения указана в репозитории проекта: https://github.com/unshame/arduino-thing/blob/master/plantMonitor.ino   
 * Информация о лицензировании находится в файле LICENSE.txt 
 */

/* Библиотеки */

// Основная библиотека Arduino
#include <Arduino.h>

// Библиотека датчика воздуха
// DHT sensor library - Version: 1.3.4
#include <DHT.h>
#include <DHT_U.h>

// Библиотека LCD дисплея
// LiquidCrystal - Version: 1.0.7
#include <LiquidCrystal.h>

// Прототипы функций и типы данных
#include "plantMonitor.h"

// Конфигурация программы (константы)
#include "plantMonitorConfig.h"

/* Интерфейсы */

// Интерфейс LCD экрана
// void ::begin() - инициализирует дисплей
// void ::setCursor(int x, int y) - устанавливает  курсор в указанную позицию
// void ::print(String) - выводит строку в текущей позиции курсора
// void ::clear() - очищает экран дисплея
LiquidCrystal lcd(PIN_LCD_1, PIN_LCD_2, PIN_LCD_3, PIN_LCD_4, PIN_LCD_5, PIN_LCD_6);

// Интерфейс датчика воздуха
// float ::readHumidity() - считывает слажность воздуха
// float ::readTemperature() - считывает температуру воздуха
DHT dht(PIN_D_DHT, DHT_TYPE);

/* Параметры */

// Влажность почвы
Param soilHumidity{
    "Soil Humidity", "SH",
    SH_MIN, SH_MAX,
    "WATER THE PLANTS", "TOO MUCH WATER",

    // Здесь чем меньше значение, тем больше влажность,
    // поэтому сообщения об исбытке/недостатке влажности поменяны местами
    true,

    0,
    getSoilHumidity};

// Яркость освещения
// Недостаток освещения проверяется отдельно, поэтому здесь опущено минимальное значение
Param ambientBrightness{
    "Ambient Brightness", "AB",
    -1, AB_MAX,
    "", "LIGHT TOO BRIGHT",
    false, 0,
    getAmbientBrightness};

// Влажность воздуха
Param airHumidity{
    "Air Humidity", "AH",
    AH_MIN, AH_MAX,
    "AIR IS TOO DRY", "AIR IS TOO MOIST",
    false, 1,
    getAirHumidity};

// Температура воздуха
Param airTemperature{
    "Air Temperature", "AT",
    AT_MIN, AT_MAX,
    "AIR IS TOO COLD", "AIR IS TOO HOT",
    false, 1,
    getAirTemperature};

// Массив указателей на структуры данных параметров
// Нужен для итерации по всем параметрам
Param *params[] = {
    &soilHumidity,
    &ambientBrightness,
    &airHumidity,
    &airTemperature};

// Кол-во параметров
const int numParams = sizeof(params) / sizeof(Param *);

// Включено ли дополнительное освещение (диод)
bool isLightOn = false;

/* INIT */

// Инициализирует программу, выполняется один раз при подаче напряжения или сбросы платы Arduino
void setup()
{

  // Устанавливаем скорость потока данных в битах в секунду
  Serial.begin(DATA_RATE);

  // Инициализируем пины, генератор случайных чисел и LCD дисплей
  initOutput();
  initRandom();
  initDisplay();

  // Задержка для вывода приветствия
  delay(INIT_DELAY);
}

// Устанавливает пины спикера и диода на режим выхода
void initOutput()
{
  pinMode(PIN_D_BUZZER, OUTPUT);
  pinMode(PIN_D_LIGHT, OUTPUT);
}

// Инициализирует генератор случайных чисел информацией из неподключенного аналогового пина
// random используется для вывода звуков случайной частоты функцией alert()
void initRandom()
{
  randomSeed(analogRead(PIN_A_UNUSED));
}

// Инициализирует размеры дисплея и выводит приветствие на экран
void initDisplay()
{
  lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  lcd.print(DISPLAY_GREETING);
}

/* LOOP */

// Считывает, анализирует и выводит текущие значения подключенных датчиков
// Выполняется после функции setup() в бесконечном цикле
void loop()
{

  // Считываем значения всех датчиков и выводим их в консоль
  updateAllValues();
  logAllValues();

  // Отдельно проверяем, что яркость освещения не меньше минимального значения,..
  bool isLightTooLow = ambientBrightness.cachedValue < AB_MIN;

  // ...и соотвественно включаем/выключаем дополнительное освещение (диод)
  if (isLightTooLow)
  {
    turnLightOn();
  }
  else
  {
    turnLightOff();
  }

  // Проверяем, что значения всех параметров попадают в приемлемые границы
  // Для первого неприемлемого значения будет выведено соответствующее сообщение
  if (isAllParamsOk())
  {

    // Если все параметры успешно прошли проверку, выводим их
    displayAllValues();

    // Задерживаем повторное считывание параметров
    delay(UPDATE_INTERVAL);
  }
  else
  {

    // Если один из параметров провалил проверку, оповещаем пользователя звуком
    playAlert();

    // Здесь нет смысла задерживать следующую проверку,
    // т.к. проигрывание звука играет роль задержки
  }
}

/* LIGHT */

// Включает дополнительное освещение (диод), если оно уже не включено
void turnLightOn()
{

  if (!isLightOn)
  {
    digitalWrite(PIN_D_LIGHT, HIGH); // Подаем максимальный уровень сигнала на пин диода
    isLightOn = true;
  }
}

// Выключает дополнительное освещение (диод), если оно включено
void turnLightOff()
{

  if (isLightOn)
  {
    digitalWrite(PIN_D_LIGHT, LOW); // Подаем минимальный уровень сигнала на пин диода
    isLightOn = false;
  }
}

/* SOUND */

// Оповещает пользователя звуком, имитирующим сирену
void playAlert()
{

  // Случайные частоты звука в границах разумного
  // Звук будет проигрываться в пределах этих частот
  int rndStart = random(10, 100);
  int rndEnd = random(rndStart + 50, rndStart + 100);

  // Мы будем проигрывать очень короткие звуки, чтобы симулировать плавное изменение высоты звука
  int delayDuration = random(5, 20);

  // Количество подъемов и спусков высоты звука
  int numCycles = 3;

  // Запускаем сирену, подавая плавно изменяющиеся значения напряжения на пин спикера
  for (int j = 0; j < numCycles; j++)
  {

    for (int i = rndStart; i <= rndEnd; i++)
    {
      tone(PIN_D_BUZZER, i * 10);
      delay(delayDuration);
    }

    for (int i = rndEnd; i >= rndStart; i--)
    {
      tone(PIN_D_BUZZER, i * 10);
      delay(delayDuration);
    }
  }

  // Выключаем сирену, снимая напряжение с пина спикера
  noTone(PIN_D_BUZZER);
}

/* CONSOLE */

// Выводит значение всех параметров в консоль
void logAllValues()
{

  for (int i = 0; i < numParams; i++)
  {
    logValue(
        params[i]->name,
        params[i]->cachedValue,
        params[i]->decimalPlaces);
  }

  Serial.println();
}

// Выводит значение параметра в консоль в формате "name: value"
// decimalPlaces указывает число знаков значения параметра после точки, которые нужно вывести
void logValue(String name, float value, int decimalPlaces)
{
  Serial.println(String(name + ": ") + String(value, decimalPlaces));
}

/* GET VALUES */

// Считывает значения всех датчиков и кеширует их в структурах параметров
void updateAllValues()
{

  for (int i = 0; i < numParams; i++)
  {
    params[i]->cachedValue = params[i]->readValue();
  }
}

// Считывает и возвращает влажность почвы от 0 до 1023
// 0 - влажная
// 1023 - сухая
float getSoilHumidity()
{
  return (float)analogRead(PIN_A_SOILHUMIDITY);
}

// Считывает и возвращает яркость освещения от 0 до 1023
// 0 - темно
// 1023 - светло
float getAmbientBrightness()
{
  return (float)analogRead(PIN_A_PHOTOSENSOR);
}

// Считывает и возвращает влажность воздуха от 0 до 100%
float getAirHumidity()
{
  return dht.readHumidity();
}

// Считывает и возвращает температуру воздуха от -40 до +150 градусов цельсия
float getAirTemperature()
{
  return dht.readTemperature();
}

/* CHECK VALUES */

// Проверяет и возвращает, попадают ли все параметры в приемлемые рамки
// Если нет, то на экран будет выведено предупреждение для первого параметра, вышедшего за рамки дозволенного
bool isAllParamsOk()
{

  for (int i = 0; i < numParams; i++)
  {

    if (!isParamOk(params[i]))
    {
      return false;
    }
  }

  return true;
}

// Проверяет и возвращает, попадает ли параметр в приемлемые рамки
// Если нет, то выводит на экран выведено предупреждение для первого параметра, вышедшего за рамки дозволенного
// Принимает ссылку на структуру с данными параметра
bool isParamOk(Param *param)
{

  if (param->cachedValue < param->minValue)
  {

    // Значение параметра ниже дозволенного
    displayWarning(
        param->invertMinMax
            ? param->messageExceeding
            : param->messageLacking);

    return false;
  }
  else if (param->cachedValue > param->maxValue)
  {

    // Значение параметра выше дозволенного
    displayWarning(
        param->invertMinMax
            ? param->messageLacking
            : param->messageExceeding);

    return false;
  }

  // С параметром все ок
  return true;
}

/* DISPLAY */

// Очищает дисплей и устанавливает курсор в начальную позицию
void resetDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
}

// Выводит значения всех параметров с их сокращенным названием по два на строке экрана
void displayAllValues()
{
  resetDisplay();

  for (int i = 0; i < numParams; i++)
  {

    displayValue(
        params[i]->shortName,
        params[i]->cachedValue,
        params[i]->decimalPlaces);

    // Переходим на следующую строку экрана
    if (i % 2 != 0)
    {
      displayLineEnd();
    }
  }
}

// Выводит переданное предупреждение на экран
void displayWarning(String warning)
{
  resetDisplay();

  lcd.print("WARNING!");
  displayLineEnd();
  lcd.print(warning);
}

// Переводит курсор на следующую строку экрана
// Так как поддерживается только двухстрочных экран, перевод всегда происходит на вторую строку
void displayLineEnd()
{
  lcd.setCursor(0, 1);
}

// Выводит значение параметра на экран в формате "shortName:value "
// decimalPlaces указывает число знаков значения параметра после точки, которые нужно вывести
void displayValue(String shortName, float value, int decimalPlaces)
{
  lcd.print(shortName + ":" + String(value, decimalPlaces) + " ");
}
