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
  const int16_t PADDING_RIGHT = 30;  // Dành chỗ bên phải 20px
  
  // Kiểm tra xem có vượt quá vùng hiển thị không
  bool isWithinBounds(int16_t x, int16_t w) {
    return (x + OFFSET_X + w <= tft.width() - PADDING_RIGHT);
  }
  
public:  void begin() {
    // TFT_eSPI handles initialization
  }
  
  void clearDisplay() {
    // Strategy mới: Clear toàn bộ eye area một cách thông minh
    // Xóa vùng rộng hơn để đảm bảo không còn trailing artifacts
    
    static unsigned long lastClear = 0;
    unsigned long now = millis();
    
    // Throttle clearing để avoid excessive clearing
    if (now - lastClear < 33) return; // Max 30 FPS clearing
    
    int16_t eyeAreaX = OFFSET_X - 20;  // 20px buffer bên trái  
    int16_t eyeAreaY = 40;             // Start từ y=40
    int16_t eyeAreaW = width() + 40;   // 310px (270 + 40 buffer)
    int16_t eyeAreaH = 160;            // 160px height để cover full movement range
    
    // Bounds checking nghiêm ngặt
    if (eyeAreaX < 0) {
      eyeAreaW += eyeAreaX; // Adjust width
      eyeAreaX = 0;
    }
    if (eyeAreaX + eyeAreaW > 320) {
      eyeAreaW = 320 - eyeAreaX;
    }
    if (eyeAreaY + eyeAreaH > 240) {
      eyeAreaH = 240 - eyeAreaY;
    }
    
    // Double-check bounds
    if (eyeAreaW > 0 && eyeAreaH > 0) {
      extern uint16_t backgroundColor;
      tft.fillRect(eyeAreaX, eyeAreaY, eyeAreaW, eyeAreaH, backgroundColor);
    }
    
    lastClear = now;
  }
  
  void display() {
    // TFT_eSPI updates immediately, no buffering needed
  }
    void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (isWithinBounds(x, 1)) {
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.drawPixel(x + OFFSET_X, y, tftColor);
    }
  }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.fillRect(x + OFFSET_X, y, w, h, tftColor);
    }
  }
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.drawRect(x + OFFSET_X, y, w, h, tftColor);
    }
  }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (isWithinBounds(x - r, 2 * r)) {
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.fillCircle(x + OFFSET_X, y, r, tftColor);
    }
  }
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (isWithinBounds(x - r, 2 * r)) {
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.drawCircle(x + OFFSET_X, y, r, tftColor);
    }
  }
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // Tìm x tối đa của tam giác
    int16_t maxX = max(max(x0, x1), x2);
    if (maxX + OFFSET_X <= tft.width() - PADDING_RIGHT) {
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.fillTriangle(x0 + OFFSET_X, y0, x1 + OFFSET_X, y1, x2 + OFFSET_X, y2, tftColor);
    }
  }
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);
      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.fillRoundRect(x + OFFSET_X, y, w, h, r, tftColor);
    }
  }
  
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Giới hạn width nếu vượt quá padding phải
    int16_t maxWidth = tft.width() - PADDING_RIGHT - (x + OFFSET_X);
    if (maxWidth > 0) {
      w = min(w, maxWidth);      uint16_t tftColor = (color == 1) ? TFT_BLUE : TFT_BLACK;
      tft.drawRoundRect(x + OFFSET_X, y, w, h, r, tftColor);    }
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
  const int16_t OFFSET_X = 10;     // Same as DisplayAdapter
  int16_t clearX = OFFSET_X - 15;  // Extra buffer for movement
  int16_t clearY = 45;             // Start earlier  
  int16_t clearW = 300;            // Wider area
  int16_t clearH = 150;            // Taller area
  
  // Bounds checking
  if (clearX < 0) clearX = 0;
  if (clearX + clearW > 320) clearW = 320 - clearX;
  if (clearY + clearH > 240) clearH = 240 - clearY;
  
  tft.fillRect(clearX, clearY, clearW, clearH, backgroundColor);
  forceEyeClear = false;
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
    handleCommand(command);
    // Force clear after command to ensure clean slate
    forceEyeClear = true;
  }
  
  // Force clear eye area if needed (to prevent trailing)
  if (forceEyeClear) {
    forceClearEyeArea();
  }
  
  // Track if we need to force clear on next frame (eye movement detection)
  unsigned long currentTime = millis();
  if (currentTime - lastEyeUpdate > 100) { // Check every 100ms for movement
    forceEyeClear = true;
    lastEyeUpdate = currentTime;
  }
  
  // Update RoboEyes (this handles all animations and drawing)
  eyes.update();
  
  // Update background color based on mood - AFTER eyes update để tránh conflict
  updateBackground();
  
  delay(67); // Optimized delay for 15 FPS (1000/15 = 66.67ms per frame)
}

void handleCommand(String command) {
  command.toLowerCase();
  Serial.println("Received: " + command);
  // Mood commands
  if (command == ":happy") {
    eyes.setMood(HAPPY);
    backgroundColor = TFT_YELLOW;
    currentMood = "happy";
    Serial.println("Mood: Happy 😊");
    
  } else if (command == ":angry") {
    eyes.setMood(ANGRY);
    backgroundColor = TFT_RED;
    currentMood = "angry";
    Serial.println("Mood: Angry 😠");
    
  } else if (command == ":tired") {
    eyes.setMood(TIRED);
    backgroundColor = TFT_NAVY;
    currentMood = "tired";
    Serial.println("Mood: Tired 😴");
    
  } else if (command == ":normal") {
    eyes.setMood(DEFAULT);
    backgroundColor = TFT_BLACK;
    currentMood = "normal";
    Serial.println("Mood: Normal 😐");
      // Animation commands
  } else if (command == ":confused") {
    eyes.anim_confused();
    forceEyeClear = true; // Force clear for animation
    Serial.println("Animation: Confused 😵");
    
  } else if (command == ":laugh") {
    eyes.anim_laugh();
    forceEyeClear = true; // Force clear for animation
    Serial.println("Animation: Laugh 😂");
    
  } else if (command == ":blink") {
    eyes.blink();
    forceEyeClear = true; // Force clear for animation
    Serial.println("Animation: Blink 😉");
    
  } else if (command == ":wink") {
    eyes.blink(true, false); // Only left eye
    forceEyeClear = true; // Force clear for animation
    Serial.println("Animation: Wink 😉");
      // Look direction commands
  } else if (command.startsWith(":look:")) {
    String direction = command.substring(6);
    direction.toUpperCase();    
    forceEyeClear = true; // Force clear for eye movement
    
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
      Serial.println("Looking: Center 👀");
    } else {
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
  // Smooth background color transitions - CHỈ thay đổi khi cần thiết
  static uint16_t currentBgColor = TFT_BLACK;
  static unsigned long lastBgUpdate = 0;
  static unsigned long lastMoodEffect = 0;
  
  // Chỉ update background khi mood thay đổi
  if (millis() - lastBgUpdate > 150 && currentBgColor != backgroundColor) { 
    currentBgColor = backgroundColor;
    // Fill background NGOÀI vùng mắt để không gây xung đột
    
    // Top area (above eyes)
    tft.fillRect(0, 0, 320, 60, currentBgColor);
    // Bottom area (below eyes) 
    tft.fillRect(0, 180, 320, 60, currentBgColor);
    // Left area 
    tft.fillRect(0, 60, 30, 120, currentBgColor);
    // Right area
    tft.fillRect(300, 60, 20, 120, currentBgColor);
    
    lastBgUpdate = millis();
  }
  
  // Giảm frequency của mood effects để tránh chớp
  if (millis() - lastMoodEffect > 300) { // Chỉ check mỗi 300ms
    // Happy sparkle effect - NGOÀI vùng mắt
    if (currentMood == "happy" && millis() % 7000 < 50) { // 7 giây 1 lần
      for (int i = 0; i < 2; i++) {
        int x, y;
        // Chọn vùng ngoài mắt
        if (random(2) == 0) {
          x = random(10, 300);
          y = random(10, 50); // Top area
        } else {
          x = random(10, 300); 
          y = random(190, 230); // Bottom area
        }
        tft.fillCircle(x, y, 1, TFT_WHITE);
      }
    }
    
    // Angry flicker - NGOÀI vùng mắt
    if (currentMood == "angry" && millis() % 4000 < 30) { // 4 giây 1 lần
      for (int i = 0; i < 3; i++) {
        int x = random(0, 320);
        int y;
        if (random(2) == 0) {
          y = random(0, 60); // Top area
        } else {
          y = random(180, 240); // Bottom area
        }
        tft.drawPixel(x, y, TFT_RED);
      }
    }
    lastMoodEffect = millis();
  }
}
