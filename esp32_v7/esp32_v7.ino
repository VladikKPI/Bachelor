// Cистема керування розумними жалюзі  (Smart blinds control system)
// виконано на ESP32 DEVKITv1
//====================================================================================================

#include <WiFi.h>                         // використовуємо бібліотеку Wi-Fi           
#include <ESP32Servo.h>                   // використовуємо бібліотеку серводвигуна
#include <LiquidCrystal_I2C.h>            // використовуємо бібліотеку LCD з модулем I2C

// Для підключення до мережі WIFI ЗАМІНИТИ на свій ідентифікатор (ssid) та пароль (password) !!!!!!!!
const char* ssid = "public";              // test1  ssid = "public"   password = "tivortin"
const char* password = "tivortin";        // test2  ssid = "jan2.5"   password = "1qazxcvb"

WiFiServer server(80);                    // виставляємо номер порта для сервера
String header;                            // створюємо змінну для зберігання заголовка HTTP-запиту

#define pin_Level_Light       34          // Пін аналогового входу сенсора освітлення 
int Level_Light = 0;

#define pin_Sens_Open         32          // Кнопка Відкрити
#define pin_Sens_Close        33          // Кнопка Закрити
#define pin_but_Open          16          // Кнопка Відкрити
#define pin_but_Close         17          // Кнопка Закрити
#define pin_but_Open_servo    18          // Кнопка Відкрити на серво
#define pin_but_Close_servo   19          // Кнопка Закрити на серво

#define pin_M1_Open           26          // виходи GPIO для керування мотором
#define pin_M1_Close          27          

String M1_Open_State = "off";             // допоміжні змінні для зберігання поточного стану виходів
String M1_Close_State = "off";

Servo myservo;                            // створити сервооб'єкт 1 для керування сервоприводом
#define pin_Servo             13          // GPIO сервопривода

String valueString = String(5);           // значення HTTP GET value
int pos1 = 0;
int pos2 = 0;

LiquidCrystal_I2C lcd(0x27,16,2);         // Налаштування LCD (адреса, кількість символів та рядків)  
                                          // використовуємо виводи GPIO 21 -SDA, GPIO 22 -SCL  

   byte simvol[8]   = {B11100,B10100,B11100,B00000,B00000,B00000,B00000,B00000,};         // Символ градусу
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


bool  M1_Open_web;                          //    
bool  M1_Close_web; 
bool  M1_Open_but; 
bool  M1_Close_but; 


//----------------------------------------------------------------------------------------------------
void setup() {
  
  Serial.begin(115200);                     // запускаємо послідовний зв’язок зі швидкістю  115200 бод із ціллю налагодження 

  pinMode(pin_Sens_Open, INPUT);
  pinMode(pin_Sens_Close, INPUT);
  pinMode(pin_but_Open, INPUT);
  pinMode(pin_but_Close, INPUT);
  pinMode(pin_but_Open_servo, INPUT);
  pinMode(pin_but_Close_servo, INPUT);
  digitalWrite(pin_Sens_Open, HIGH);         //внутрішній підтягуючий резистор на землю
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
  
  Serial.print("Connecting to ");           // Підключаємося до Wi-Fi
  Serial.println(ssid);
  WiFi.begin(ssid, password);        
  while (WiFi.status() != WL_CONNECTED) {     // чекаємо успішного з’єднання
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");                       // виводимо локальну IP-адресу ESP у Serial Monitor
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();                           // запускаємо сервер


  lcd.init();                               // ініциализуємо LCD
  lcd.clear();                              // Очищуємо екран  
  lcd.backlight();                          // Вмикаємо підсвітку дисплея

  lcd.createChar(1, bukva_ZH);              // генеруємо недостаючі літери
  lcd.createChar(2, bukva_L);
  lcd.createChar(3, bukva_IO);
  lcd.createChar(4, bukva_Z);

  lcd.setCursor(2, 0);                      // виставити курсор на 3 комірку 1 рядку
  lcd.print("ABT. \1A\2\3\4I");             // АВТ. ЖАЛЮЗІ
  lcd.setCursor(0, 1);                      // виставити курсор на 1 комірку 2 рядку
  lcd.print(WiFi.localIP());                // виводимо IP мережі
}

//-------------------------------------------------------------------------------------------------
void loop() {
  
  WiFiClient client = server.available();    // прослуховування вхідних клієнтів

 // Level_Light = analogRead(pin_Level_Light);   // зчитуємо значення    
 // lcd.clear();                            // Очищуємо екран  
 // lcd.setCursor(0, 0); 
 // lcd.print(Level_Light); 
 //delay(1000);

  if (client) {                              // якщо підключається новий клієнт,
    Serial.println("New Client.");           // виводимо повідомлення в serial port
    String currentLine = "";                 // створити рядок для зберігання вхідних даних від клієнта
    while (client.connected()) {             // цикл, поки є з'єднання клієнта
      if (client.available()) {              // якщо від клієнта надходять дані,
        char c = client.read();              // читаємо байт, потім
        Serial.write(c);                     // виводимо його на serial port
        header += c;
        if (c == '\n') {                     // якщо байт є переведення рядка
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
                 M1_Open_web = HIGH;                              // digitalWrite(pin_M1_Open, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
                       Serial.println("M1_Open off");
                       M1_Open_State = "off";
                       M1_Open_web = LOW;                        //digitalWrite(pin_M1_Open, LOW);

            } else if (header.indexOf("GET /27/on") >= 0) {
                      Serial.println("M1_Close on");
                      M1_Close_State = "on";
                      M1_Close_web = HIGH;                       //digitalWrite(pin_M1_Close, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
                       Serial.println("M1_Close off");
                       M1_Close_State = "off";
                       M1_Close_web = LOW;                      //digitalWrite(pin_M1_Close, LOW);
            }
           
           
            // Формуємо веб-сторінку на сервері
            client.println("<!DOCTYPE html><html>");             // вказуємо, що ми надсилаємо HTML
            // рядок робить веб-сторінку адаптивною в будь-якому веб-переглядачі
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
            client.println("<p>Open - State M1" + M1_Open_State + "</p>");               //GPIO 26 - State - напис над 1 кнопкою
            // Якщо поточний стан GPIO вимкнено, ми показуємо кнопку ON, якщо ні, ми відображаємо кнопку OFF
            if (M1_Open_State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            // Аналогічно для другої кнопки
            client.println("<p>Close - State M1 " + M1_Close_State + "</p>");         //GPIO 27 - State - напис над 2 кнопкою
            if (M1_Close_State == "off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            

            // Виводимо слайсер
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            client.println("<p>Position: <span id=\"servoPos\"></span></p>");    // добавил
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\"" + valueString + "\"/>");

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
              valueString = header.substring(pos1 + 1, pos2);

              //Rotate the servo
              myservo.write(valueString.toInt());
              Serial.println(valueString);
            }
         
         
         
            client.println();                 // HTTP-відповідь завершується порожнім рядком
            break;
          } else {                            //якщо отримали новий рядок, очищаємо currentLine
            currentLine = "";
          }
        } else if (c != '\r') {               // Якщо отримали щось ще окрім повернення рядка,
          currentLine += c;                   // додаємо його в кінець поточного рядка currentLine
        }
      }
    }
    header = "";                              // коли відповідь закінчується, ми очищаємо змінну заголовка 
    client.stop();                            // припиняємо з’єднання з клієнтом
    Serial.println("Client disconnected.");
    Serial.println("");
  }



 M1_Open_but = digitalRead(pin_but_Open);        // читаємо вхід 
 M1_Close_but = digitalRead(pin_but_Close);        // читаємо вхід 

if (M1_Open_web == HIGH || M1_Open_but == HIGH )           //    
       {
       digitalWrite(pin_M1_Open, HIGH);                                                                   // Загальна помилка
       }
else {                               
       digitalWrite(pin_M1_Open, LOW);
       } 

if (M1_Close_web == HIGH || M1_Close_but == HIGH )           //    
       {
       digitalWrite(pin_M1_Close, HIGH);                                                                   // Загальна помилка
       }
else {                               
       digitalWrite(pin_M1_Close, LOW);
       } 

}