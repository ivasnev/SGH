#include <iarduino_DHT.h>
#include <Servo.h>  // Подключаем библиотеку для работы с датчиком DHT
iarduino_DHT sensor(A0);
Servo WaterServo;  // create servo object to control a servo
Servo WindowServo;
const uint8_t WLSensor = A1;  // Объявляем объект, указывая номер вывода, к которому подключён модуль
const uint8_t LWLSensor = A2;
float hum_air = 0;
float tem_air = 0;
float water_lvl = 0;
float hum_soil = 0;
float min_hum_air = 50;
float min_tem_air = 15;
float min_water_lvl = 50;
float min_hum_soil = 60;
float max_hum_air = 100;
float max_tem_air = 40;
float max_water_lvl = 100;
float max_hum_soil = 90;
bool is_window_open = false;
bool is_water_go = false;
bool max_tem_air_error = false;
bool min_tem_air_error = false;
bool max_hum_soil_error = false;
bool min_hum_soil_error = false;
bool max_hum_air_error = false;
bool min_hum_air_error = false;
bool min_water_lvl_error = false;
bool max_water_lvl_error = false;
// СКЕТЧ РАБОТАЕТ, КАК С МОДУЛЕМ DHT-11, ТАК И С МОДУЛЕМ DHT-22.
void setup() {
  Serial.begin(9600);  // Инициируем передачу данных в монитор последовательного порта, на скорости 9600 бод
  delay(1000);         // Приостанавливаем выполнение скетча на 1 секунду, для перехода датчика в активное состояние
}
void loop() {
  String json = "{";
  switch (sensor.read()) {  // Читаем показания датчика
    case DHT_OK:
      {
        hum_air = sensor.hum;
        tem_air = sensor.tem;
        json = (String)json + "\"hum_air\": " + hum_air + ", \"tem_air\": " + tem_air + ", ";
        break;
      }

    default:
      {
        hum_air = -10;
        tem_air = -100;
        json = (String)json + "\"hum_air\": " + hum_air + ", \"tem_air\": " + tem_air + ", ";
        break;
      }
  }
  hum_soil = analogRead(LWLSensor);
  water_lvl = analogRead(WLSensor);

  water_lvl =((water_lvl - 400) / 240) * 100;
  if (water_lvl < 0) water_lvl = 0; else if (water_lvl > 100) water_lvl = 100;
  hum_soil =(hum_soil / 800) * 100;
  if (hum_soil > 100) hum_soil = 100;

  json = (String)json + "\"hum_soil\": " + hum_soil + ", \"water_lvl\": " + water_lvl + ", ";
  json = (String)json + "\"min_hum_air\": " + min_hum_air + ", \"min_tem_air\": " + min_tem_air + ", \"min_hum_soil\": " + min_hum_soil + ", \"min_water_lvl\": " + min_water_lvl + ", ";
  json = (String)json + "\"max_hum_air\": " + max_hum_air + ", \"max_tem_air\": " + max_tem_air + ", \"max_hum_soil\": " + max_hum_soil + ", \"max_water_lvl\": " + max_water_lvl + ", ";

  if (tem_air > max_tem_air) {
    max_tem_air_error = true;
    if (!is_window_open) {
      is_window_open = true;
      WindowServo.write(90) // open
      delay(1000);
    }
  } else {
    if (tem_air < min_tem_air) {
      min_tem_air_error = true;
      if (is_window_open) {
        is_window_open = false;
        WindowServo.write(0) // close
        delay(1000);
      }
    } else {
      if (max_tem_air_error) max_tem_air_error = false;
      if (min_tem_air_error) min_tem_air_error = false;
    }
  }

  if (hum_soil > max_hum_soil) {
    max_hum_soil_error = true;
    if (is_water_go) {
      is_water_go = false;
      WaterServo.write(0) // close
      delay(1000);
    }
  } else {
    if (hum_soil < min_hum_soil) {
      min_hum_soil_error = true;
      if (is_water_go) {
        if (water_lvl < min_water_lvl) {
          min_water_lvl_error = true;
          is_water_go = false;
          WaterServo.write(0) //close
          delay(1000);
        }
      } else {
        if (water_lvl >= min_water_lvl) {
          min_water_lvl_error = false;
          is_water_go = true;
          WaterServo.write(90) // open
          delay(1000);
        } else {
          min_water_lvl_error = true;
        }
      }
    } else {
      if (max_hum_soil_error) max_hum_soil_error = false;
      if (min_hum_soil_error) min_hum_soil_error = false;
    }
  }

  if (hum_air > max_hum_air) {
    max_hum_air_error = true;
  } else {
    if (hum_air < min_hum_air) {
      min_hum_air_error = true;
    } else {
      if (max_hum_air_error) max_hum_air_error = false;
      if (min_hum_air_error) min_hum_air_error = false;
    }
  }

  if (water_lvl > max_water_lvl) {
    max_water_lvl_error = true;
  } else {
    if (water_lvl < min_water_lvl) {
      min_water_lvl_error = true;
    } else {
      if (max_water_lvl_error) max_water_lvl_error = false;
      if (min_water_lvl_error) min_water_lvl_error = false;
    }
  }

  String json2 = (String)"\"min_hum_soil_error\": " + min_hum_soil_error + ", \"max_hum_soil_error\": " + max_hum_soil_error
         + ", \"max_tem_air_error\": " + max_tem_air_error + ", \"min_tem_air_error\": " + min_tem_air_error
         + ", \"max_hum_air_error\": " + max_hum_air_error + ", \"min_hum_air_error\": " + min_hum_air_error
         + ", \"min_water_lvl_error\": " + min_water_lvl_error + ", \"max_water_lvl_error\": " + max_water_lvl_error + "}";

  Serial.print(json);
  Serial.print(json2);
  Serial.println();
  
  delay(1000);  // Приостанавливаем выполнение скетча на 1 секунду, между выводами показаний
}
