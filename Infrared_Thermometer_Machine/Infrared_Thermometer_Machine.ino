//Khai báo thư viện cần thiết cho ESP32
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

//Information GG Firebase
#define FIREBASE_HOST "https://webofhuy-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "kEw0jTfdl4NXXCtHZetZ4nYmGmTt8oXd9fsP0T0p"
char ssid[] = "Mah";//Tên đăng nhập WiFi
char pass[] = "17092002";//Mật khẩu WiFi


WiFiClientSecure client;//Biến Client của WiFiClientSecure để gửi thông tin lên Google Sheet

/**********Google Sheets Definations***********/
String GOOGLE_SCRIPT_ID = "AKfycbzQzg9t3yVNDHahm4D1K9wcKhtLdEyNDbFlHICnXO4eRUGlT4OhauePn5OKnCkOBaNXgg";    

//Firebase Data
FirebaseData fbdo1;
FirebaseData fbdo2;
FirebaseData fbdo3;

//LCD OLED Parameter
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//Khai báo cho màn hình SSD1306 được kết nối với I2C (chân SDA, SCL)

//GPIO laser
#define laser 12

//Khai báo biến mlx là MLX90614 của Adafruit
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//Biến Temperature
double temp_amb;
double temp_obj;

//LED, cảm biến HC-SR501 và buzzer GPIO
const int led_pin = 14;
const int sensor_pin = 13;
const int buzzer_pin = 4;

//Khoảng thời gian của ngắt
const long interval = 2000;//Khoảng thời gian là 2000ms
unsigned long current_time = millis();//Thời gian tính bằng ms
unsigned long last_trigger = 0;
boolean timer_on = false;//Tắt timer

//Interrupt function
void IRAM_ATTR movement_detection() {
  Serial.println("Motion was detected");//In dòng: "Motion was detected"
  digitalWrite(led_pin, HIGH);//Bật led thể hiện phát hiện chuyển động
  timer_on = true;//Bật timer (Bắt đầu đếm ngược)
  last_trigger = millis();
}

void setup() {
  Serial.begin(115200);//Tốc độ baud của Serial Monitor là 115200

  //MLX90614
  mlx.begin();                                //Khởi tạo MLX90614 bắt đầu hoạt động
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //khởi tạo với I2C addr 0x3C (128x64)

  //Kết nối WiFi và Firebase
  WiFi.begin(ssid,pass);//Bắt đầu kết nối với WiFi với thông tin về tên đăng nhập và mật khẩu
  Serial.print("Connect Wi-Fi");//In ra dòng: "Connect Wi-Fi"
  while (WiFi.status() != WL_CONNECTED)//Nếu không kết nối được WiFi
  {
    Serial.print(".");//In ra dấu "."
    delay(300);//Độ trễ 0.3s
  }
  Serial.println();
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());//Nếu kết nối thành công in ra địa chỉ IP WiFi đang kết nối
  Serial.println();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);//Kết nối tới FireBase
  Firebase.reconnectWiFi(true);//Nếu mất kết nối tự động kết nối lại

  //Interrupt
  pinMode(sensor_pin, INPUT_PULLUP);//Cài đặt chân đầu vào và pull_up cho HC-SR501
  attachInterrupt(digitalPinToInterrupt(sensor_pin), movement_detection, RISING);//Tạo kết nối với hàm ngắt, chân nhận tín hiệu ngắt và xung cạnh lên
  pinMode(led_pin, OUTPUT);//Gán chân Led là OUTPUT
  digitalWrite(led_pin, LOW);//Chân led khởi đầu mức thấp
  pinMode(buzzer_pin, OUTPUT);
  digitalWrite(buzzer_pin, LOW);  

  Serial.println("Temperature Sensor MLX90614");//In ra dòng: "Temperature Sensor MLX90614"

  pinMode(laser, OUTPUT);  // Connect LASER
  
  //Cài đặt thông số Laser module
  pinMode(laser, OUTPUT);
  digitalWrite(laser, LOW);
  
  //Cài đặt thông số bắt đầu cho OLED
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" Thermometer");
  display.setCursor(25, 35);
  display.setTextSize(1);
  display.print("Initializing");
  display.display();
  delay(5000);
}

void loop() {
  //Cài đặt đơn vị thời gian là ms 
  current_time = millis();

  //Interrupt ON và Timer sẽ đếm xuống
  if (timer_on && (current_time - last_trigger <= interval)) {
    //Đọc nhiệt độ môi trường và vật thể
    //nếu muốn đọc nhiệt độ dưới dạng độ F thì dùng
    //mlx.readAmbientTempF() , mlx.readObjectTempF() )
    temp_amb = mlx.readAmbientTempC();
    temp_obj = mlx.readObjectTempC()+1.65;
    
    //Bật laser module
    digitalWrite(laser, HIGH);

    //Gửi dữ liệu ra Serial Monitor (Arduino IDE)
    Serial.print("Room Temp = ");//In ra dòng: "Room Temp = "
    Serial.println(temp_amb);//Nhiệt độ phòng
    Serial.print("Object temp = ");//In ra dòng: "Object temp = "
    Serial.println(temp_obj);//Nhiệt độ vật thể
    
    //Hiển thị nhiệt độ vật thể được đo ra màn hình
    display.clearDisplay();//xóa toàn bộ chữ có trên màn hình
    display.setCursor(25, 10);//Vị trí con trỏ
    display.setTextSize(1);//Độ lớn ký tự
    display.setTextColor(WHITE);//Màu ký tự
    display.println(" Temperature");//Hiện ra dòng: " Temperature"
    display.setCursor(25, 30);//Vị trí con trỏ
    display.setTextSize(2);//Độ lớn ký tự
    display.print(temp_obj);//Độ lớn nhiệt độ vật thể
    display.print((char)247);//Ký tự độ
    display.print("C");//Ký tự C
    display.display();

    //Gửi dữ liệu về nhiệt độ Object và Ambident lên Firebase
    Firebase.setInt(fbdo1, "/HTN/Object", temp_obj);
    Firebase.setInt(fbdo2, "/HTN/Ambident", temp_amb);

    //Nếu nhiệt độ người được đo cập nhật trạng thái bình thường ngược lại là Alert
    if(temp_obj <= 37.5 || temp_obj >= 34.5){
      Firebase.setString(fbdo3, "/HTN/Status", "NORMAL!!");
    } else {
      Firebase.setString(fbdo3, "/HTN/Status", "ALERT!!!");
    }

    //Điều kiện về nhiệt độ của con người để Buzzer báo động (Khối chấp hành)
    if (temp_obj >= 37.5) {
      digitalWrite(buzzer_pin, HIGH);
      Serial.print("ALERT!!!!\n ");
    }

    //Điều kiện phân biệt về nhiệt độ của con người gửi nhiệt độ lên GG Sheet
    if (temp_obj >= 35.75 && temp_obj <= 41) {
      sendData(temp_obj,temp_amb);
    }
    delay(2000);
  }

  //Interrupt OFF khi Timer lớn hơn interval 
  //Tắt hết các hệt động khi Interrupt ngắt
  if (timer_on && (current_time - last_trigger > interval)) {
    Serial.println("Motion has stopped");
    digitalWrite(led_pin, LOW);//LED tắt
    digitalWrite(buzzer_pin, LOW);//Buzzer tắt
    digitalWrite(laser, LOW);//laser tắt
    timer_on = false;//Tắt timer
    display.clearDisplay();//xóa màn hình oled
  }
  
}

//Tạo hàm sendData để gửi dữ liệu lên Google Sheet
void sendData(float temp_obj, float temp_amb) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?value1="+temp_obj+"&value2="+temp_amb;//Đường dẫn tự động gửi dữ liệu lên Sheet
   Serial.print(url);//In ra đường dẫn
    Serial.print("Making a request");//In ra dòng "Making a request"
    http.begin(url.c_str()); //Chỉ định URL và certificate
    int httpCode = http.GET();//Lấy httpCode
    http.end();//Kết thúc kết nối http 
    Serial.println(": done\n "+httpCode);//In ra dòng: "done" + httpCode
}