#include <LiquidCrystal_I2C.h>

// Cистема керування жалюзі  (Blinds control system)
// виконано на ESP32 DEVKITv1
//====================================================================================================

#include <WiFi.h>                         // використовуємо бібліотеку Wi-Fi           
#include <ESP32Servo.h>                   // використовуємо бібліотеку серводвигуна
#include <LiquidCrystal_I2C.h>            // використовуємо бібліотеку LCD з модулем I2C

// Для підключення до мережі WIFI ЗАМІНИТИ на свій ідентифікатор (ssid) та пароль (password) !!!!!!!!
const char* ssid = "jan2.5";              // test1  ssid = "public"   password = "tivortin"
const char* password = "1qazxcvb";        // test2  ssid = "jan2.5"   password = "1qazxcvb"
                                          // test3  ssid = "Redmi Note 8T" password = "kremen03"

WiFiServer server(80);                    // виставляємо номер порта для сервера
String header;                            // створюємо змінну для зберігання заголовка HTTP-запиту

// піни мікроконтролера -------------------------------------------------------------------------------
#define pin_Level_Light       34          // Пін аналогового входу сенсора освітлення 
#define pin_Sens_Open         32          // Кнопка Відкрити
#define pin_Sens_Close        33          // Кнопка Закрити
#define pin_but_Open          16          // Кнопка Відкрити
#define pin_but_Close         17          // Кнопка Закрити
#define pin_but_Open_servo    18          // Кнопка Відкрити на серво
#define pin_but_Close_servo   19          // Кнопка Закрити на серво

#define pin_M1_Open           26          // виходи GPIO для керування мотором
#define pin_M1_Close          27          

Servo myservo;                            // створюємо об'єкт 1 для керування сервоприводом
#define pin_Servo             13          // GPIO сервопривода

String valueString = String(5);           // значення HTTP GET value
int pos1 = 0;
int pos2 = 0;

LiquidCrystal_I2C lcd(0x27,16,2);         // Налаштування LCD (адреса, кількість символів та строк)  
                                          // використовуємо виводи GPIO 21 -SDA, GPIO 22 -SCL  

   byte simvol[8]   = {B11100,B10100,B11100,B00000,B00000,B00000,B00000,B00000,};         // Символ градуса
   byte bukva_B[8]  = {B11111, B10000, B10000, B11110, B10001, B10001, B11110};           // Б
   byte bukva_G[8]  = {B11111, B10000, B10000, B10000, B10000, B10000, B10000};           // Г   
   byte bukva_D[8]  = {B00111, B01001, B01001, B01001, B01001, B01001, B11111, B10001};   // Д 
   byte bukva_ZH[8] = {B10101, B10101, B10101, B01110, B10101, B10101, B10101};           // Ж 
   byte bukva_Z[8]  = {B01110, B10001, B00001, B00010, B00001, B10001, B01110};           // З
   byte bukva_I[8]  = {B10001, B10011, B10101, B10101, B10101, B11001, B10001};           // И
   byte bukva_II[8] = {B00100, B10001, B10011, B10101, B10101, B11001, B10001};           // Й
   byte bukva_L[8]  = {B00111, B01001, B01001, B01001, B01001, B01001, B10001};           // Л
   byte bukva_P[8]  = {B11111, B10001, B10001, B10001, B10001, B10001, B10001};           // П
   byte bukva_Y[8]  = {B10001, B10001, B10001, B01010, B00100, B01000, B10000};           // У 
   byte bukva_IL[8] = {B10001, B10001, B10001, B10001, B10001, B10001, B11111, B00001};   // Ц
   byte bukva_CH[8] = {B10001, B10001, B10001, B01111, B00001, B00001, B00001};           // Ч
   byte bukva_F[8]  = {B11111, B10101, B10101, B10101, B10101, B11111, B00100};           // Ф  
   byte bukva_III[8]= {B10101, B10101, B10101, B10101, B10101, B10101, B11111};           // Ш
   byte bukva_IIL[8]= {B10101, B10101, B10101, B10101, B10101, B10101, B11111, B00001};   // Щ
   byte bukva_b[8]  = {B10000, B10000, B10000, B11110, B10001, B10001, B11110};           // Ь
   byte bukva_IO[8] = {B10010, B10101, B10101, B11101, B10101, B10101, B10010};           // Ю   
   byte bukva_IA[8] = {B01111, B10001, B10001, B01111, B00101, B01001, B10001};           // Я

// Змінні програми   --------------------------------------------------------------------------
bool  Pulse_20ms;                         // пульс 20ms
bool  Pulse_100ms;                        // пульс 100ms
bool  Pulse_2000ms;                       // пульс 2000ms
unsigned long Act_Time_ms;                // час, мс
unsigned long Time_takt_20ms;             // лічильник такту 20ms
unsigned long Time_takt_100ms;            // лічильник такту 100ms
unsigned long Time_takt_2000ms;           // лічильник такту 100ms

bool  Sens_Open;                          // змінні стану сенсорів та кнопок 
bool  Sens_Close;
bool  M1_Open_web;                          
bool  M1_Close_web; 
bool  M1_Open_but; 
bool  M1_Close_but; 
bool  servo_Open_web;                          
bool  servo_Close_web; 
bool  servo_Open_but; 
bool  servo_Close_but; 

bool  Err_sens;                           // змінні помилок системи
bool  Err_swich; 
bool  Err_but; 

bool  Mess1;                              // змінні повідомлень
bool  Mess2;
bool  Mess3;
bool  Mess4;
bool  Mess5;
bool  Mess6;

bool  Mode_Avto;                          // автоматичний режим роботи
bool  Avto_web;
bool  Avto;
bool  fl1;  

float h_F;                                // фільтр

int Level_Light = 0;                      // змінні стану опт сенсора
int Servo_Value = 0; 
int Servo_Value_web = 0; 
int Servo_Value_web_1 = 0; 

String M1_Open_State = "off";             // допоміжні змінні для зберігання поточного стану кнопок на WEB сторінці
String M1_Close_State = "off";
String Avto_State  = "off";

//=====================================================================================================================================
// Налаштування при вмиканні
//=====================================================================================================================================

void setup() {
  
  pinMode(pin_Sens_Open, INPUT);            // налаштовуємо GPIO плати як INPUT
  pinMode(pin_Sens_Close, INPUT);
  pinMode(pin_but_Open, INPUT);
  pinMode(pin_but_Close, INPUT);
  pinMode(pin_but_Open_servo, INPUT);
  pinMode(pin_but_Close_servo, INPUT);
  digitalWrite(pin_Sens_Open, HIGH);        //внутрішній підтягуючий резистор на землю
  digitalWrite(pin_Sens_Close, HIGH);            
  digitalWrite(pin_but_Open, HIGH);            
  digitalWrite(pin_but_Close, HIGH);             
  digitalWrite(pin_but_Open_servo, HIGH);             
  digitalWrite(pin_but_Close_servo, HIGH);             

  pinMode(pin_M1_Open, OUTPUT);             // налаштовуємо GPIO плати як OUTPUT
  pinMode(pin_M1_Close, OUTPUT);
  digitalWrite(pin_M1_Open, LOW);           // встановлюємо для GPIO значення LOW
  digitalWrite(pin_M1_Close, LOW);

  myservo.attach(pin_Servo);                // GPIO для керування сервоприводом
  

if (digitalRead(pin_Sens_Open) == HIGH || digitalRead(pin_Sens_Close) == HIGH )              
       {
       Err_sens = HIGH;                                                                   // Помилка стану концевих вимикачів
       }
if (digitalRead(pin_but_Open) == HIGH || digitalRead(pin_but_Close) == HIGH || 
        digitalRead(pin_but_Open_servo) == HIGH || digitalRead(pin_but_Close_servo) == HIGH )             
       {
       Err_but = HIGH;                                                                   // Помилка стану кнопок
       }

  Serial.begin(115200);                     // запускаємо послідовний зв’язок зі швидкістю  115200 біт/с для цілей налагодження 
  Serial.print("Connecting to ");           // Підключаємося до Wi-Fi
  Serial.println(ssid);
  WiFi.begin(ssid, password);        
  while (WiFi.status() != WL_CONNECTED) {   // чекаємо успішного з’єднання
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");                       // виводимо локальну IP-адресу ESP у Serial Monitor
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();                           // запускаємо сервер


  lcd.init();                               // ініціалізуємо LCD
  lcd.clear();                              // Очищуємо екран  
  lcd.backlight();                          // Вмикаємо підсвітку дисплея

  lcd.createChar(1, bukva_ZH);              // Генеруємо недостаючі літери
  lcd.createChar(2, bukva_L);
  lcd.createChar(3, bukva_IO);
  lcd.createChar(4, bukva_Z);
  lcd.createChar(5, simvol);

  lcd.setCursor(2, 0);                      // виставити курсор на 3 комірку 1 рядку
  lcd.print("SMART \1A\2\3\4I");            // АВТ. ЖАЛЮЗІ
  lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
  if (Err_sens == HIGH)                     // якщо зафіксована помилка стану сенсорів 
     {
      lcd.print("Err swich");               // повідомлення Помилка стану концевих вммикачів
     }
    else
      if (Err_but == HIGH )                 // якщо зафіксована помилка стану кнопок  
         {
          lcd.print("Err button");          // повідомлення Помилка стану кнопок
         }
        else 
           {
            lcd.print(WiFi.localIP());      // якщо все ОК виводимо IP мережі
           }
    delay(5000);                            // час відображення привітання
    Mode_Avto = HIGH;                       // вмикаємо автоматичний режим
}

//=====================================================================================================================================
// Основний цикл програми
//=====================================================================================================================================
void loop() {

  Act_Time_ms = millis();                        // значення часу кожний цикл  
   
   Pulse_20ms = LOW;                             // генератор імпульсів 20ms
   if (Act_Time_ms - Time_takt_20ms > 20) {      // перевірка часу
       Time_takt_20ms = Act_Time_ms;           
       Pulse_20ms = HIGH;                        // імпульс 20 мс 
       }  
   Pulse_100ms = LOW;                            // генератор імпульсів 100ms
   if (Act_Time_ms - Time_takt_100ms > 100) {    // перевірка часу
       Time_takt_100ms = Act_Time_ms;           
       Pulse_100ms = HIGH;                       // імпульс 100 мс 
       }
   Pulse_2000ms = LOW;                           // генератор імпульсів 2000ms
   if (Act_Time_ms - Time_takt_2000ms > 2000) {  // перевірка часу
       Time_takt_2000ms = Act_Time_ms;           
       Pulse_2000ms = HIGH;                      // імпульс 2000ms
       }

  WiFiClient client = server.available();        // прослуховування вхідних клієнтів

  if (client) {                                  // якщо підключається новий клієнт,
    Serial.println("New Client.");               // виводимо повідомлення в serial port
    String currentLine = "";                     // створити рядок для зберігання вхідних даних від клієнта
    while (client.connected()) {                 // цикл, поки є з'єднання клієнта
      if (client.available()) {                  // якщо від клієнта надходять дані,
        char c = client.read();                  // читаємо байт, потім
        Serial.write(c);                         // виводимо його на serial port
        header += c;
        if (c == '\n') {                         // якщо байт -> переведення рядка
          // якщо рядок порожній, ми отримали два символи переведення рядка
          // це кінець HTTP-запиту, формуємо відповідь сервера:
          if (currentLine.length() == 0) {
            // заголовки HTTP завжди починаються з коду відповіді (напр., HTTP / 1.1 200 OK)
            // и content-type, потім порожній рядок:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // перевіряємо, яка кнопка була натиснута на вашій веб-сторінці та вмикаємо або вимикаємо GPIO
            if (header.indexOf("GET /26/on") >= 0) {
                 Serial.println("M1_Open on");
                 M1_Open_State = "on";
                 M1_Open_web = HIGH;                             // M1_Open_web;
            } else if (header.indexOf("GET /26/off") >= 0) {
                       Serial.println("M1_Open off");
                       M1_Open_State = "off";
                       M1_Open_web = LOW;                        // M1_Open_web;

            } else if (header.indexOf("GET /27/on") >= 0) {
                      Serial.println("M1_Close on");
                      M1_Close_State = "on";
                      M1_Close_web = HIGH;                       //M1_Close_web;
            } else if (header.indexOf("GET /27/off") >= 0) {
                       Serial.println("M1_Close off");
                       M1_Close_State = "off";
                       M1_Close_web = LOW;                       //M1_Close_web;

             } else if (header.indexOf("GET /Avto/on") >= 0) {
                      Serial.println("Avto on");
                      Avto_State = "on";
                      Avto_web = HIGH;                           //Avto_web;
            } else if (header.indexOf("GET /Avto/off") >= 0) {
                       Serial.println("Avto off");
                       Avto_State = "off";
                       Avto_web = LOW;                           //Avto_web;
            }
           
            // Формуємо веб-сторінку на сервері
            client.println("<!DOCTYPE html><html>");            
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS для стилізації кнопок і зовнішнього вигляду веб-сторінки. Ми вибираємо шрифт Helvetica, визначаємо вміст, який буде відображатися блоком і вирівнюватись по центру
            client.println("<style>html { font-family: Helvetica;  display: inline-block;  margin: 0px auto;  text-align: center;}");
            // стилізуємо наші кнопки кольором #4CAF50, без рамки, текст білого кольору та з таким відступом: 16px 40px
            client.println(".button { background-color: #4CAF50; border:  none; color: white; padding: 16px 40px;");
            // встановлюємо параметр text-decoration у значення none, визначаємо розмір шрифту, поля та курсор для вказівника
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            // визначаємо стиль для другої кнопки з усіма властивостями кнопки, які ми визначили раніше, але іншого кольору. Це буде стиль кнопки вимкнення
            client.println(".button2 {background-color:#555555;}</style></head>");
            // встановлюємо перший заголовок веб-сторінки
            client.println("<body><h1>Smart blinds control</h1>");                        //  Smart blinds control - заголовок сторінки
           
           
            // Виводимо поточний стан кнопок
            client.println("<p>Open Smart blinds  -  " + M1_Open_State + "</p>");               //GPIO 26 - State - напис над 1 кнопкою
            // Якщо поточний стан GPIO вимкнено, ми показуємо кнопку ON, якщо ні, ми відображаємо кнопку OFF
            if (M1_Open_State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button\">ON</button></a></p>");
            }
            // Аналогічно для другої кнопки
            client.println("<p>Close Smart blinds  -  " + M1_Close_State + "</p>");         //GPIO 27 - State - напис над 2 кнопкою
            if (M1_Close_State == "off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button\">ON</button></a></p>");
            }
          
          // Аналогічно для другої кнопки
            client.println("<p>Avto mode - " + Avto_State + "</p>");         //Напис над 3 кнопкою
            if (Avto_State == "off") {
              client.println("<p><a href=\"/Avto/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/Avto/off\"><button class=\"button\">ON</button></a></p>");
            }


            // Виводимо слайдер
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            client.println("<p>Position: <span id=\"servoPos\"></span></p>");   
            client.println("<input type=\"range\" min=\"0\" max=\"90\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\"" + valueString + "\"/>");

            client.println("<script>var slider = document.getElementById(\"servoSlider\");");
            client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");

            
            
            client.println("</body></html>");
            
          
           //GET /?value=180& HTTP/1.1
            if (header.indexOf("GET /?value=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);    // Значення кута повороту слайдера

              //Rotate the servo
           //   myservo.write(valueString.toInt());
              Serial.println(valueString);
            }
         
         
         
            client.println();                      // HTTP-відповідь завершується порожнім рядком
            break;
          } else {                                 //якщо отримали новий рядок, очищаємо currentLine
            currentLine = "";
          }
        } else if (c != '\r') {                    // Якщо отримали щось ще окрім повернення рядка,
          currentLine += c;                        // додаємо його в кінець поточного рядка currentLine
        }
      }
    }
    header = "";                                   // коли відповідь закінчується, ми очищаємо змінну заголовка 
    client.stop();                                 // припиняємо з’єднання з клієнтом
    Serial.println("Client disconnected.");
    Serial.println("");
  }


 Sens_Open        = digitalRead(pin_Sens_Open);     // визначаємо стан сигналів
 Sens_Close       = digitalRead(pin_Sens_Close);
 M1_Open_but      = digitalRead(pin_but_Open);      
 M1_Close_but     = digitalRead(pin_but_Close);      
 servo_Open_but   = digitalRead(pin_but_Open_servo);      
 servo_Close_but  = digitalRead(pin_but_Close_servo);   

if (M1_Open_web == HIGH || M1_Open_but == HIGH )    // якщо є команда на переміщення
       {
       Mess1 =  HIGH;                               // флаг активації нового повідомлення
       if (Sens_Open == LOW )                       // та сенсор межі дозволяє переміщення   
          {
          digitalWrite(pin_M1_Open, HIGH);          // включаемо переміщення
          lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
          lcd.print("Open RUN        ");            // виводимо повідомлення 16 символів
          }
       if (Sens_Open == HIGH && Sens_Close == LOW)                      // та сенсор межі HE дозволяє переміщення   
          {
          digitalWrite(pin_M1_Open, LOW);           // виключаемо переміщення
          lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
          lcd.print("Open max        ");            // виводимо повідомлення  16 символів
          } 
       if (Sens_Open == HIGH && Sens_Close == HIGH)  // якщо оба сенсори спрацювали - помилка  
          {
          digitalWrite(pin_M1_Open, LOW);           // виключаемо переміщення
          lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
          lcd.print("Err swich       ");            // виводимо повідомлення  16 символів
          }                                                                     
       }
else {  
       Mess1 = LOW;                                 // флаг активації нового повідомлення                             
       digitalWrite(pin_M1_Open, LOW);
       } 

if (M1_Close_web == HIGH || M1_Close_but == HIGH )  // якщо є команда на переміщення
       {
       Mess2 =  HIGH;                               // флаг активації нового повідомлення
       if (Sens_Close== LOW )                       // та сенсор межі дозволяє переміщення   
          {
          digitalWrite(pin_M1_Close, HIGH);         // включаемо переміщення
          lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
          lcd.print("Close RUN       ");            // виводимо повідомлення 16 символів
          }
       if (Sens_Close == HIGH && Sens_Open == LOW)                     // та сенсор межі HE дозволяє переміщення   
          {
          digitalWrite(pin_M1_Close, LOW);          // виключаемо переміщення
          lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
          lcd.print("Close max       ");            // виводимо повідомлення 16 символів
          } 
       if (Sens_Open == HIGH && Sens_Close == HIGH) // якщо обидва сенсори спрацювали - помилка  
          {
          digitalWrite(pin_M1_Close, LOW);          // виключаемо переміщення
          lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
          lcd.print("Err swich       ");            // виводимо повідомлення  16 символів
          }                                                                           // Загальна помилка
       }
else { 
       Mess2 =  LOW;                                // флаг активації нового повідомлення                              
       digitalWrite(pin_M1_Close, LOW);
       } 

 Servo_Value_web =valueString.toInt();
 if (fl1 == LOW )                                   //  
          {
            Servo_Value_web_1 = Servo_Value_web;
            fl1 = HIGH;
          }  
 if (Servo_Value_web  > Servo_Value_web_1 || Servo_Value_web < Servo_Value_web_1 )                                   //  
          {
            Mode_Avto = LOW;                        // вимикаємо автоматичний режим
            Servo_Value = Servo_Value_web;
            Servo_Value_web_1 = Servo_Value_web;
          }  


if (servo_Open_web == HIGH || servo_Open_but == HIGH )    // якщо є команда на поворот 
       {
       Mess3 =  HIGH;                                // флаг активації нового повідомлення 
       Mode_Avto = LOW;                              // вимикаємо автоматичний режим
       if (Pulse_20ms == HIGH )                      // з кожним тактом
          {
          if (Servo_Value < 90  )                    // перевіряємо, якщо
             {
              Servo_Value++ ;                        // +1 
              lcd.setCursor(0, 1);                   // виставити курсор на 1 комірку 2 рядку
              lcd.print("Servo Open   ");            // виводимо повідомлення  16 символів 
              lcd.print(Servo_Value);
              lcd.print("\5 ");    
             } 
          if (Servo_Value > 89  )                    // якщо вище   
             {
              lcd.setCursor(0, 1);                   // виставити курсор на 1 комірку 2 рядку
              lcd.print("Servo max Open  ");         // виводимо повідомлення  16 символів     
             } 
          }                                                          
       }
else {  
       Mess3 = LOW;                                  // флаг активації нового повідомлення                
       } 

if (servo_Close_web == HIGH || servo_Close_but == HIGH )    // якщо є команда на поворот 
       {
       Mess4 =  HIGH;                                // флаг активації нового повідомлення 
       Mode_Avto = LOW;                              // вимикаємо автоматичний режим
       if (Pulse_20ms == HIGH )                      // з кожним тактом
          {
          if (Servo_Value > 0  )                     // перевіряємо, якщо 
             {
              Servo_Value --;                        // -1 
              lcd.setCursor(0, 1);                   // виставити курсор на 1 комірку 2 рядку
              lcd.print("Servo Close  ");            // виводимо повідомлення  16 символів 
              lcd.print(Servo_Value);
              lcd.print("\5 ");   
             } 
          if (Servo_Value < 1  )                     // та сенсор межі HE дозволяє переміщення   
             {
              lcd.setCursor(0, 1);                   // виставити курсор на 1 комірку 2 рядку
              lcd.print("Servo max Close ");         // виводимо повідомлення  16 символів    
             } 
          }                                                          
       }
else {  
       Mess4 = LOW;                                  // флаг активації нового повідомлення                
       } 

if (Mode_Avto ==  HIGH )                             // якщо увімкнений автоматичний режим 
   {
     if (Pulse_100ms == HIGH )                       // з кожним тактом
          {
           Level_Light = analogRead(pin_Level_Light);       // значення опт сенсору  
            h_F = pr_FILTR_hx30 ((float)Level_Light);        // виклик підпрограми фільтра
           h_F = h_F/33.0;                                  // перерахунок для визначення кута серво
           Servo_Value = (int)h_F;
            }
    if (Servo_Value > 90)                            // значення не може бути більше 90
       { 
        Servo_Value = 90;
       }

    if (Mess1 == LOW && Mess2 == LOW)                // якщо є команда на переміщення
       {
        lcd.setCursor(0, 1);                         // виставити курсор на 1 комірку 2 рядку
        lcd.print("Mode Avto    ");                  // виводимо повідомлення  16 симв  
        lcd.print(Servo_Value);
        lcd.print("\5 ");
       }
   } 
if (Mode_Avto ==  LOW )                              // якщо увімкнений автоматичний режим 
   {
    if (Mess1 == LOW && Mess2 == LOW && Mess3 == LOW && Mess4 == LOW ) // якщо є команда на переміщення
       {
        lcd.setCursor(0, 1);                         // виставити курсор на 1 комірку 2 рядку
        lcd.print("Mode Manual  ");                  // виводимо повідомлення  16 символів
        lcd.print(Servo_Value);
        lcd.print("\5 ");
       }
   }
  myservo.write(Servo_Value);                        // задача на сервопривід  
/*
if (Avto_web ==  HIGH )                              // якщо увімкнений автоматичний режим 
   {
    Mode_Avto = HIGH;
   }
  else
   {
    Mode_Avto = LOW;
   }

   */
}
//=====================================================================================================================================
// Підпрограми
//=====================================================================================================================================
   float pr_FILTR_hx30 (float input)                 // перевірка фільтру
      {                                             
       static bool first_cycle = LOW; 
       static float valve_filtr = 0; 
       static float output = 0;

       if (first_cycle == LOW)                       // при 1 циклі
          {    
           valve_filtr = input * 29.0;               // встановлюємо значення фільтру
           first_cycle = HIGH;                       // встановлюємо 1 цикл   
          }   
       
       valve_filtr = valve_filtr + input;   
       output = valve_filtr / 30.0;
       valve_filtr = valve_filtr - output; 
    
       return output;                                // повертаємо значення
      } 
