//***********************************************************************************************
//  ESP32 Emotional Pet with FluxGarage RoboEyes + TFT_eSPI
//  
//  Hardware: ESP32 S3 N16R8 PSRAM + TFT 2.4" 320x240 ILI9341
//  
//  Tính năng:
//  - Tích hợp thư viện FluxGarage RoboEyes với TFT_eSPI
//  - Animation mượt mà với auto-blink và idle mode
//  - Nhiều trạng thái cảm xúc và hiệu ứng
//  - Performance cao hơn với TFT_eSPI
//  
//  Lệnh Serial:
//  :happy - Vui vẻ
//  :angry - Tức giận  
//  :tired - Mệt mỏi
//  :normal - Bình thường
//  :confused - Bối rối (animation)
//  :laugh - Cười (animation)
//  :blink - Chớp mắt
//  :wink - Nháy mắt
//  :look:N/NE/E/SE/S/SW/W/NW - Nhìn theo hướng
//  :idle:1/0 - Bật/tắt idle mode
//  :auto:1/0 - Bật/tắt auto blink
//  :cyclops:1/0 - Chế độ một mắt
//  :curious:1/0 - Chế độ tò mò
//
//***********************************************************************************************

#include <TFT_eSPI.h>
#include <SPI.h>

// TFT_eSPI instance
TFT_eSPI tft = TFT_eSPI(320, 240); // Landscape mode 320x240

// Display Adapter để FluxGarage RoboEyes hoạt động với TFT_eSPI
// Case che bên trái 40px, bên phải 20px -> dịch chuyển mắt sang phải
class DisplayAdapter {
private:  const int16_t OFFSET_X = 10;       // Giảm offset để mắt nằm giữa màn hình
  const int16_t PADDING_RIGHT = 40;  // Dành chỗ bên phải 20px
  
  // Kiểm tra xem có vượt quá vùng hiển thị không
  bool isWithinBounds(int16_t x, int16_t w) {
    return (x + OFFSET_X + w <= tft.width() - PADDING_RIGHT);
  }
  
public:  void begin() {
    // TFT_eSPI handles initialization
  }
  void clearDisplay() {
    // NUCLEAR OPTION - Xóa toàn bộ màn hình mỗi frame
    // Đây là cách duy nhất để đảm bảo 100% không có trailing
    extern uint16_t backgroundColor;
    tft.fillScreen(backgroundColor);
  }
  
  void display() {
    // TFT_eSPI updates immediately, no buffering needed
  }  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    extern uint16_t backgroundColor;
    // ALWAYS clear pixel trước khi vẽ để tránh nhiễu
    tft.drawPixel(x + OFFSET_X, y, backgroundColor);
    if (isWithinBounds(x, 1)) {
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.drawPixel(x + OFFSET_X, y, tftColor);
    }
  }void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.fillRect(x + OFFSET_X, y, w, h, tftColor);
    }
  }  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.drawRect(x + OFFSET_X, y, w, h, tftColor);
    }
  }  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (isWithinBounds(x - r, 2 * r)) {
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.fillCircle(x + OFFSET_X, y, r, tftColor);
    }
  }
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (isWithinBounds(x - r, 2 * r)) {
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.drawCircle(x + OFFSET_X, y, r, tftColor);
    }
  }  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // Tìm x tối đa của tam giác
    int16_t maxX = max(max(x0, x1), x2);
    if (maxX + OFFSET_X <= tft.width() - PADDING_RIGHT) {
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.fillTriangle(x0 + OFFSET_X, y0, x1 + OFFSET_X, y1, x2 + OFFSET_X, y2, tftColor);
    }
  }  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.fillRoundRect(x + OFFSET_X, y, w, h, r, tftColor);
    }
  }
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      extern uint16_t backgroundColor;
      uint16_t tftColor = (color == 1) ? TFT_CYAN : backgroundColor;
      tft.drawRoundRect(x + OFFSET_X, y, w, h, r, tftColor);
    }
  }
    // Trả về kích thước có tính padding để RoboEyes tính toán đúng
  int16_t width() { return tft.width() - OFFSET_X - PADDING_RIGHT; }  // 320 - 10 - 20 = 290px
  int16_t height() { return tft.height(); }  // 240px - kích thước đầy đủ
};

// Create display adapter instance (renamed to avoid conflict with RoboEyes)
DisplayAdapter displayAdapter;

// Create global display reference for FluxGarage RoboEyes library
DisplayAdapter& display = displayAdapter;

// Include FluxGarage RoboEyes library AFTER display is defined
#include <FluxGarage_RoboEyes.h>

// FluxGarage RoboEyes instance với TFT_eSPI adapter
roboEyes eyes;

// Variables for background effects
uint16_t backgroundColor = TFT_BLACK;
String currentMood = "normal";
unsigned long lastMoodChange = 0;
unsigned long moodDuration = 5000; // 5 seconds

// Variables for eye movement tracking
static unsigned long lastEyeUpdate = 0;
static bool forceEyeClear = false;

// Force clear eye area (used when eyes move to prevent trailing)
void forceClearEyeArea() {
  // NUCLEAR CLEAR - Xóa toàn bộ màn hình
  tft.fillScreen(backgroundColor);
}

void setup() {
  Serial.begin(115200);
    // Initialize TFT
  tft.init();
  tft.setRotation(2); // Landscape 320x240
  tft.fillScreen(TFT_BLACK);  // Initialize RoboEyes với kích thước đầy đủ màn hình
  eyes.begin(320, 240, 15);  // Sử dụng full screen, OFFSET sẽ center mắt// Configure RoboEyes default settings cho 15 FPS
  eyes.setAutoblinker(true, 6, 10); // Auto blink every 6-10 seconds (chậm hơn cho 15 FPS)
  eyes.setIdleMode(false);          // BẬT idle mode để có chuyển động tự nhiên
  eyes.setCuriosity(true);         // BẬT curiosity mode để tự động nhìn xung quanh
    // Set larger eyes for TFT display
  eyes.setWidth(50, 50);           // Bigger eyes
  eyes.setHeight(50, 50);
  eyes.setBorderradius(12, 12);    // More rounded
  eyes.setSpacebetween(30);        // More space between eyes
    // CỐ ĐỊNH vị trí mắt ở trung tâm ban đầu
  eyes.setPosition(DEFAULT);       // Set to center position
  
  Serial.println("=== ESP32 Emotional Pet with FluxGarage RoboEyes ===");
  Serial.println("TFT_eSPI + FluxGarage RoboEyes Integration");
  Serial.println("");
  Serial.println("Commands:");
  Serial.println(":happy - Happy mood");
  Serial.println(":angry - Angry mood");
  Serial.println(":tired - Tired mood");
  Serial.println(":normal - Normal mood");
  Serial.println("");
  Serial.println("Animations:");
  Serial.println(":confused - Confused animation");
  Serial.println(":laugh - Laugh animation");
  Serial.println(":blink - Manual blink");
  Serial.println(":wink - Wink (left eye)");
  Serial.println("");
  Serial.println("Look directions:");
  Serial.println(":look:N/NE/E/SE/S/SW/W/NW/CENTER");
  Serial.println("");
  Serial.println("Settings:");
  Serial.println(":idle:1/0 - Toggle idle mode");
  Serial.println(":auto:1/0 - Toggle auto blink");
  Serial.println(":cyclops:1/0 - Toggle cyclops mode");
  Serial.println(":curious:1/0 - Toggle curiosity mode");
  Serial.println("=========================================================");
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.println("DEBUG: Raw command = '" + command + "'");
    handleCommand(command);
  }
  
  // REDUCED clearing strategy - chỉ clear khi cần thiết
  static unsigned long lastClear = 0;
  if (millis() - lastClear > 50) { // Clear mỗi 50ms thay vì mỗi frame
    tft.fillScreen(backgroundColor);
    lastClear = millis();
  }
  
  // Update RoboEyes (this handles all animations and drawing)
  eyes.update();
  
  delay(33); // 30 FPS
}

void handleCommand(String command) {
  command.toLowerCase();
  Serial.println("Received: " + command);
  Serial.println("DEBUG: Command length = " + String(command.length()));
  
  // Mood commands
  if (command == ":happy") {
    Serial.println("DEBUG: Processing :happy command");
    eyes.setMood(HAPPY);
    backgroundColor = TFT_YELLOW;
    currentMood = "happy";
    Serial.println("Mood: Happy 😊");
    
  } else if (command == ":angry") {
    Serial.println("DEBUG: Processing :angry command");
    eyes.setMood(ANGRY);
    backgroundColor = TFT_RED;
    currentMood = "angry";
    Serial.println("Mood: Angry 😠");
    
  } else if (command == ":tired") {
    Serial.println("DEBUG: Processing :tired command");
    eyes.setMood(TIRED);
    backgroundColor = TFT_NAVY;
    currentMood = "tired";    Serial.println("Mood: Tired 😴");
    
  } else if (command == ":normal") {
    eyes.setMood(DEFAULT);
    backgroundColor = TFT_BLACK;
    currentMood = "normal";
    Serial.println("Mood: Normal 😐");
    
  // Animation commands
  } else if (command == ":confused") {
    tft.fillScreen(backgroundColor); // Nuclear clear BEFORE animation
    eyes.anim_confused();
    Serial.println("Animation: Confused 😵");
    
  } else if (command == ":laugh") {
    tft.fillScreen(backgroundColor); // Nuclear clear BEFORE animation
    eyes.anim_laugh();
    Serial.println("Animation: Laugh 😂");
    
  } else if (command == ":blink") {
    tft.fillScreen(backgroundColor); // Nuclear clear BEFORE animation
    eyes.blink();
    Serial.println("Animation: Blink 😉");
    
  } else if (command == ":wink") {
    tft.fillScreen(backgroundColor); // Nuclear clear BEFORE animation    eyes.blink(true, false); // Only left eye
    Serial.println("Animation: Wink 😉");
    
  // Look direction commands
  } else if (command.startsWith(":look:")) {String direction = command.substring(6);
    direction.toUpperCase();    
    tft.fillScreen(backgroundColor); // Nuclear clear BEFORE eye movement
    
    if (direction == "N") {
      eyes.setPosition(N);
      Serial.println("Looking: North ⬆️");
    } else if (direction == "NE") {
      eyes.setPosition(NE);
      Serial.println("Looking: North-East ↗️");
    } else if (direction == "E") {
      eyes.setPosition(E);
      Serial.println("Looking: East ➡️");
    } else if (direction == "SE") {
      eyes.setPosition(SE);
      Serial.println("Looking: South-East ↘️");
    } else if (direction == "S") {
      eyes.setPosition(S);
      Serial.println("Looking: South ⬇️");
    } else if (direction == "SW") {
      eyes.setPosition(SW);
      Serial.println("Looking: South-West ↙️");
    } else if (direction == "W") {
      eyes.setPosition(W);
      Serial.println("Looking: West ⬅️");
    } else if (direction == "NW") {
      eyes.setPosition(NW);
      Serial.println("Looking: North-West ↖️");
    } else if (direction == "CENTER") {
      eyes.setPosition(DEFAULT);
      Serial.println("Looking: Center 👀");    } else {
      Serial.println("Invalid direction. Use: N/NE/E/SE/S/SW/W/NW/CENTER");
    }
    
  // Settings commands
  } else if (command.startsWith(":idle:")) {
    int value = command.substring(6).toInt();
    eyes.setIdleMode(value == 1);
    Serial.println("Idle mode: " + String(value == 1 ? "ON (mắt sẽ di chuyển)" : "OFF (mắt cố định)"));
    
  } else if (command.startsWith(":auto:")) {
    int value = command.substring(6).toInt();
    eyes.setAutoblinker(value == 1);
    Serial.println("Auto blink: " + String(value == 1 ? "ON" : "OFF"));
    
  } else if (command.startsWith(":cyclops:")) {
    int value = command.substring(9).toInt();
    eyes.setCyclops(value == 1);
    Serial.println("Cyclops mode: " + String(value == 1 ? "ON" : "OFF"));
    
  } else if (command.startsWith(":curious:")) {
    int value = command.substring(9).toInt();
    eyes.setCuriosity(value == 1);
    Serial.println("Curiosity mode: " + String(value == 1 ? "ON (tự động nhìn xung quanh)" : "OFF (không tự động nhìn)"));
    
  } else {
    Serial.println("❌ Unknown command!");
    Serial.println("📋 Available commands:");
    Serial.println("   Moods: :happy, :angry, :tired, :normal");
    Serial.println("   Animations: :confused, :laugh, :blink, :wink");
    Serial.println("   Look: :look:N/NE/E/SE/S/SW/W/NW/CENTER");
    Serial.println("   Settings: :idle:1/0, :auto:1/0, :cyclops:1/0, :curious:1/0");
  }
  
  lastMoodChange = millis();
}

void updateBackground() {
  // Simplified background - CHỈ fill các góc để tránh conflict với eye area
  static uint16_t currentBgColor = TFT_BLACK;
  static unsigned long lastBgUpdate = 0;
  
  // Chỉ update background khi mood thay đổi
  if (millis() - lastBgUpdate > 200 && currentBgColor != backgroundColor) { 
    currentBgColor = backgroundColor;
    
    // CHỈ fill 4 góc màn hình để tránh conflict với eye area
    // Top corners
    tft.fillRect(0, 0, 320, 25, currentBgColor);
    // Bottom corners 
    tft.fillRect(0, 215, 320, 25, currentBgColor);
    // Left side
    tft.fillRect(0, 25, 15, 190, currentBgColor);
    // Right side
    tft.fillRect(305, 25, 15, 190, currentBgColor);
    
    lastBgUpdate = millis();
  }
  
  // LOẠI BỎ mood effects để tránh xung đột
}
