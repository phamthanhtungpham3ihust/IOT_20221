# DỰ ÁN: NHÀ THÔNG MINH 
Với dự án này, nhóm sinh viên thực hiện triển khai một hệ thống nhà thông minh nhỏ với phạm vi trong một phòng làm việc. Hệ thống có những chức năng cơ bản nhất của một căn nhà thông minh bao gồm: giám sát thông số nhiệt độ, độ ẩm, điều khiển tắt/mở các thiết bị điện trong phòng.
# Yêu cầu triển khai
Để có thể triển khai được dự án, những phần mềm và thiết bị phần cứng sau được yêu cầu
## Phần mềm:
- Visual Studio Code 
- IoT Development Framework chính thức của Espressif dành cho ESP32 (ESP - IDF) 
(Hướng dẫn: https://www.youtube.com/watch?v=OvMsayjTvjE&list=PL0LsX_xUaMBOf8jiiByV5mDIzsPB5P19j)
- Local MQTT Broker Mosquitto trên hệ điều hành Windows (Hướng dẫn: https://www.youtube.com/watch?v=xLLFrLhegcw&t=338s)
- Tạo một tài khoản trên platform Ubidots (https://ubidots.com/signup) (Chú ý: Đăng ký tài khoản STEM dành cho giáo dục)
- Phần mềm Esptouch trên điện thoại (có thể tìm trên App Store hoặc CH Play)
## Phần cứng:
- 1 cảm biến nhiệt độ, độ ẩm DHT11
- 2 module relay 5V 1 kênh (hoặc 1 module relay 5v 2 kênh)
- 4 module ESP32 

# Tổ chức chương trình
## Dự án bao gồm 4 folder chương trình
1. gateway: folder chương trình cho module esp32 cho gateway
   - components: chứa các thư viện được sử dụng trong chương chính
   - html: chứa file .html giao diện web điều khiển local của hệ thống, được nhúng vào flash của esp32
   - main: chứa file chương trình chính của gateway
2. node_sensor: folder chương trình cho module esp32 kết nối với cảm biến DHT11
   - components: chứa các thư viện được sử dụng trong chương chính
   - main: chứa file chương trình chính của hệ thống giám sát nhiệt độ, độ ẩm
3. node_relay_1: folder chương trình cho module esp32 kết nối với module relay 5V 1 kênh (hoặc module relay 5V 2 kênh) điều khiển thiết bị điện thứ 1
   - components: chứa các thư viện được sử dụng trong chương chính
   - main: chứa file chương trình chính của hệ thống điều khiển tắt/mở thiết bị điện thứ 1
4. node_relay_2: folder chương trình cho module esp32 kết nối với module relay 5V 1 kênh (hoặc module relay 5V 2 kênh) điều khiển thiết bị điện thứ 2
   - components: chứa các thư viện được sử dụng trong chương chính
   - main: chứa file chương trình chính của hệ thống điều khiển tắt/mở thiết bị điện thứ 2


# Cách cài đặt và sử dụng
## Bước 1: Tải các folder code của dự án về máy tính cá nhân
- Cách 1: Truy cập https://github.com/phamthanhtungpham3ihust/IOT_20221, chọn Code/Download ZIP, sau đó giải nén file zip vừa tải
- Cách 2: Nếu máy tính đã cài đặt git, sử dụng các lệnh sau:
  - Mở cửa sổ lệnh command prompt
  - Đi đến đường dẫn lưu dự án, ví dụ lưu dự án tại Folder "Project" trong ổ C
     ```
      C:\>cd Project
     ```
  - Clone dự án về đường dẫn đã chọn
    ```
    git init
    git clone https://github.com/phamthanhtungpham3ihust/IOT_20221.git
    ```
## Bước 2: Đăng nhập vào tài khoản Ubidots để sử dụng giao diện người dùng
- Username: tungg810
- Password: tung8102001

## Bước 3: Kết nối máy tính cá nhân với một mạng Wi-Fi
## Bước 4: Lấy địa chỉ IPv4 của máy tính cá nhân bằng lệnh
    ```
    C:\>ipconfig
    ```
## Bước 5: Nạp chương trình cho gateway
- Mở folder gateway bằng VSCode
- Vào components/app_mqtt, mở file app_mqtt.h, chỉnh dòng code
  ```
  #define MQTT_BROKER     "mqtt://192.168.1.107"
  ```
  thành
  ```
  #define MQTT_BROKER     "mqtt://your_ip_address"
  ```
  với your_ip_address là địa chỉ IPv4 của máy tính cá nhân đã lấy ở Bước 4
- Mở cửa sổ lênh ESP-IDF CMD, đi đến đường dẫn của folder gateway, ví dụ
  ```
  C:\>cd Project/IOT_20221/gatway
  ```
- Xóa flash của module ESP32 bằng lệnh
  ```
  idf.py -p PORT erase_flash
  ```
  với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager)
- Buld và flash chương trình xuống module ESP32 đóng vai trò là gateway bằng lệnh
  ```
  idf.py -p PORT flash monitor
  ```
  với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager), để thoát cửa sổ nhấn tổ hợp Ctrl + C
- Sử dụng Esptouch để kết nối Wi-Fi cho module ESP32 thông qua SmartConfig (chú ý: phải sử dụng cùng một Wi-Fi với máy tính cá nhân đang cài đặt broker Mosquitto)
  - Ví dụ terminal kết nối Wi-Fi thành công
    ```
    I (78114) esp_netif_handlers: sta ip: 192.168.0.115, mask: 255.255.255.0, gw: 192.168.0.1
    I (78114) smartconfig_example: got ip:192.168.0.115
    I (81194) smartconfig_example: smartconfig over
    I (81194) app_mqtt: [APP] Free memory: 235068 bytes
    I (81194) app_http_server: Starting server on port: '80'
    I (81194) app_mqtt: Other event id:7
    I (81214) app_http_server: Registering URI handlers
    ```
   - Địa chỉ ip của module ESP32 là 192.168.0.115
- Vào trình duyệt, truy cập vào giao diện web diều khiển local ở địa chỉ http://you_esp32_ip_address/index.html
  với your_esp32_ip_address là địa chỉ ip của module ESP32 
  
## Bước 6: Nạp chương trình cho module ESP32 kết nối với cảm biến DHT11
- Mở folder node_sensor bằng VSCode
- Vào components/app_mqtt, mở file app_mqtt.h, sửa dòng code
  ```
  #define MQTT_BROKER     "mqtt://192.168.1.107"
  ```
  thành
  ```
  #define MQTT_BROKER     "mqtt://your_ip_address"
  ```
  với your_ip_address là địa chỉ IPv4 của máy tính cá nhân đã lấy ở Bước 4
- Mở cửa sổ lênh ESP-IDF CMD, đi đến đường dẫn của folder node_sensor, ví dụ
  ```
  C:\>cd Project/IOT_20221/node_sensor
  ```
- Xóa flash của module ESP32 bằng lệnh
  ```
  idf.py -p PORT erase_flash
  ```
  với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager)
- Buld và flash chương trình xuống module ESP32 kết nối với cảm biến DHT11 bằng lệnh
  ```
  idf.py -p PORT flash monitor
  ```
  với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager), để thoát cửa sổ nhấn tổ hợp Ctrl + C
- Sử dụng Esptouch để kết nối Wi-Fi cho module ESP32 thông qua SmartConfig (chú ý: phải sử dụng cùng một Wi-Fi với máy tính cá nhân đang cài đặt broker Mosquitto)

## Bước 7: Nạp chương trình cho module ESP32 kết nối với module relay điều khiển tắt/mở thiết bị điện thứ 1
- Mở folder node_relay_1 bằng VSCode
- Vào components/app_mqtt, mở file app_mqtt.h, sửa dòng code
  ```
  #define MQTT_BROKER     "mqtt://192.168.1.107"
  ```
  thành
  ```
  #define MQTT_BROKER     "mqtt://your_ip_address"
  ```
  với your_ip_address là địa chỉ IPv4 của máy tính cá nhân đã lấy ở Bước 4
- Mở cửa sổ lênh ESP-IDF CMD, đi đến đường dẫn của folder node_relay_1, ví dụ
  ```
  C:\>cd Project/IOT_20221/node_relay_1
  ```
- Xóa flash của module ESP32 bằng lệnh
  ```
  idf.py -p PORT erase_flash
  ```
  với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager)
- Buld và flash chương trình xuống module ESP32 kết nối với module relay điều khiển thiết bị điện thứ 1 bằng lệnh
  ```
  idf.py -p PORT flash monitor
  ```
  với PORT là cổng COM tương ứng của thiết bị (xem trong Devices Manager), để thoát cửa sổ nhấn tổ hợp Ctrl + C
- Sử dụng Esptouch để kết nối Wi-Fi cho module ESP32 thông qua SmartConfig (chú ý: phải sử dụng cùng một Wi-Fi với máy tính cá nhân đang cài đặt broker Mosquitto)

## Bước 8: Lặp lại tương tụ bước 7, nạp chương trình cho module ESP32 kết nối với module relay điều khiển tắt/mở thiết bị điện thứ 2
## Sau khi thực hiện thành công 8 bước trên, hệ thống đã có thể hoạt động với các chức năng: giám sát thông số nhiệt độ, độ ẩm, điều khiển tắt/mở thiết bị điện trong phòng
