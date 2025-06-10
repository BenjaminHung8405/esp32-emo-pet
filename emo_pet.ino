//***********************************************************************************************
//  ESP32 Emotional Pet - Sử dụng TFT_eSPI và FluxGarage RoboEyes
//  
//  Hardware: ESP32 S3 N16R8 PSRAM + TFT 2.4" 320x240 ILI9341
//  
//  Thư viện cần thiết:
//  - TFT_eSPI (cấu hình trong User_Setup.h)
//  - FluxGarage_RoboEyes: https://github.com/FluxGarage/RoboEyes
//  
//  Tính năng:
//  - Nhận lệnh từ Serial (laptop) để thay đổi biểu cảm
//  - 8 trạng thái cảm xúc với animation đặc biệt
//  - Sử dụng RoboEyes library cho animation mượt mà
//  - Background và hiệu ứng thay đổi theo cảm xúc
//  
//  Lệnh Serial:
//  :happy - Vui vẻ (mắt cười + nền vàng)
//  :angry - Tức giận (mắt nhỏ + nền đỏ + lông mày)
//  :sleep - Ngủ (mắt nhắm + nền tối)
//  :sad - Buồn (mắt to + nền xanh + nước mắt)
//  :love - Yêu thương (mắt trái tim + nền hồng + trái tim bay)
//  :surprise - Ngạc nhiên (mắt rất to + nền trắng)
//  :normal - Bình thường (mặc định)
//  :wink - Nháy mắt (chớp mắt liên tục)
//  :confused - Bối rối (mắt lắc qua lại)
//  :tired - Mệt mỏi (mắt nửa nhắm + yawn animation)
//
//***********************************************************************************************

#include <TFT_eSPI.h>
#include <SPI.h>

// Khởi tạo display trước khi include RoboEyes
TFT_eSPI tft = TFT_eSPI(320, 240); // Kích thước TFT 2.4" ILI9341

// Tạo adapter để RoboEyes hoạt động với TFT_eSPI  
class DisplayAdapter {
private:
  const int16_t OFFSET_X = 10;  // Padding bên trái 10px
  
public:
  void clearDisplay() {
    tft.fillScreen(TFT_BLACK);
  }
  
  void display() {
    // TFT_eSPI không cần buffer flush
  }
  
  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    tft.drawPixel(x + OFFSET_X, y, color);
  }
  
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.fillRect(x + OFFSET_X, y, w, h, color);
  }
  
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.drawRect(x + OFFSET_X, y, w, h, color);
  }
  
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    tft.fillCircle(x + OFFSET_X, y, r, color);
  }
  
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    tft.drawCircle(x + OFFSET_X, y, r, color);
  }
  
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    tft.fillTriangle(x0 + OFFSET_X, y0, x1 + OFFSET_X, y1, x2 + OFFSET_X, y2, color);
  }
  
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    tft.fillRoundRect(x + OFFSET_X, y, w, h, r, color);
  }
  
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    tft.drawRoundRect(x + OFFSET_X, y, w, h, r, color);
  }
  
  int16_t width() { return tft.width() - OFFSET_X; }  // Trừ đi padding để RoboEyes tính toán đúng
  int16_t height() { return tft.height(); }
};

// Tạo global display object cho RoboEyes
DisplayAdapter display;

#include <FluxGarage_RoboEyes.h>

// Khởi tạo RoboEyes sau khi display adapter sẵn sàng
roboEyes roboEyes;

// Enum cho các trạng thái cảm xúc - tránh xung đột với FluxGarage RoboEyes #define
enum EmotionState {
  EMO_NORMAL,
  EMO_HAPPY,
  EMO_ANGRY,
  EMO_SLEEP,
  EMO_SAD,
  EMO_LOVE,
  EMO_SURPRISE,
  EMO_WINK,
  EMO_CONFUSED,
  EMO_TIRED
};

// Class quản lý cảm xúc và hiệu ứng
class EmotionalPet {
private:
  EmotionState currentEmotion;
  EmotionState previousEmotion;
  
  // Timing cho các hiệu ứng
  unsigned long lastEffectUpdate;
  unsigned long lastBackgroundUpdate;
  unsigned long emotionStartTime;
  int effectFrame;
  
  // Background colors
  uint16_t backgroundColor;
  uint16_t primaryColor;
  uint16_t secondaryColor;
  
  // Animation states
  bool isPlayingAnimation;
  int animationStep;
  unsigned long lastAnimationUpdate;

public:  EmotionalPet() {
    currentEmotion = EMO_NORMAL;
    previousEmotion = EMO_NORMAL;
    lastEffectUpdate = 0;
    lastBackgroundUpdate = 0;
    emotionStartTime = 0;
    effectFrame = 0;
    isPlayingAnimation = false;
    animationStep = 0;
    lastAnimationUpdate = 0;
  }
  
  void begin() {    // Khởi tạo TFT
    tft.init();
    tft.setRotation(2); // Landscape
    tft.fillScreen(TFT_CYAN); // Sáng hơn thay vì NAVY
      // Khởi tạo RoboEyes
    roboEyes.begin(320, 240, 60); // 60 FPS max
    
    // Đặt màu mắt sáng hơn
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt màu trắng thay vì xanh tối
    
    // Cấu hình mặc định
    setNormalMode();
    
    Serial.println("ESP32 Emotional Pet initialized!");
  }
  
  void setEmotion(EmotionState newEmotion) {
    if (currentEmotion != newEmotion) {
      previousEmotion = currentEmotion;
      currentEmotion = newEmotion;
      emotionStartTime = millis();
      effectFrame = 0;
      isPlayingAnimation = true;
      animationStep = 0;
      
      // Áp dụng cấu hình cho cảm xúc mới
      applyEmotionSettings();
      
      // Chạy animation chuyển đổi
      playTransitionAnimation();
    }
  }
  
  void update() {
    // Update RoboEyes
    roboEyes.update();
    
    // Update hiệu ứng background
    updateBackgroundEffects();
    
    // Update animation đặc biệt
    updateSpecialAnimations();
    
    // Vẽ các hiệu ứng overlay
    drawEmotionOverlay();
  }

private:
  void applyEmotionSettings() {
    // Reset về cấu hình mặc định trước
    roboEyes.setAutoblinker(OFF);
    roboEyes.setIdleMode(OFF);
    roboEyes.setHFlicker(OFF);
    roboEyes.setVFlicker(OFF);
    roboEyes.setCyclops(OFF);
      switch(currentEmotion) {
      case EMO_NORMAL:
        setNormalMode();
        break;
        
      case EMO_HAPPY:
        setHappyMode();
        break;
        
      case EMO_ANGRY:
        setAngryMode();
        break;
        
      case EMO_SLEEP:
        setSleepMode();
        break;
        
      case EMO_SAD:
        setSadMode();
        break;
        
      case EMO_LOVE:
        setLoveMode();
        break;
        
      case EMO_SURPRISE:
        setSurpriseMode();
        break;
        
      case EMO_WINK:
        setWinkMode();
        break;
        
      case EMO_CONFUSED:
        setConfusedMode();
        break;
        
      case EMO_TIRED:
        setTiredMode();
        break;
    }
  }
  void setNormalMode() {
    backgroundColor = TFT_CYAN;      // Sáng hơn thay vì NAVY
    primaryColor = TFT_BLACK;
    secondaryColor = TFT_CYAN;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(50, 50);
    roboEyes.setHeight(50, 50);
    roboEyes.setBorderradius(12, 12);
    roboEyes.setSpacebetween(30);
    roboEyes.setMood(DEFAULT);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 3, 2);
    roboEyes.setIdleMode(OFF, 2, 2);
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt trắng sáng
  }
    void setHappyMode() {
    backgroundColor = TFT_YELLOW;
    primaryColor = TFT_BLACK;
    secondaryColor = TFT_ORANGE;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(60, 60);
    roboEyes.setHeight(35, 35); // Mắt cười
    roboEyes.setBorderradius(20, 20);
    roboEyes.setSpacebetween(25);
    roboEyes.setMood(HAPPY);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 1, 1); // Blink nhanh hơn
    roboEyes.setIdleMode(ON, 1, 1);
    roboEyes.setEyeColor(TFT_BLACK);  // Mắt đen trên nền vàng
  }
    void setAngryMode() {
    backgroundColor = TFT_RED;
    primaryColor = TFT_WHITE;
    secondaryColor = TFT_MAROON;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(45, 45);
    roboEyes.setHeight(30, 30); // Mắt nhỏ giận dữ
    roboEyes.setBorderradius(5, 5);
    roboEyes.setSpacebetween(40);
    roboEyes.setMood(ANGRY);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 1, 0); // Blink nhanh đều
    roboEyes.setHFlicker(ON, 3); // Rung ngang
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt trắng trên nền đỏ
  }
  void setSleepMode() {
    backgroundColor = TFT_DARKGREY;  // Sáng hơn thay vì BLACK
    primaryColor = TFT_WHITE;        // Sáng hơn
    secondaryColor = TFT_LIGHTGREY;  // Sáng hơn
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(55, 55);
    roboEyes.setHeight(8, 8); // Mắt nhắm
    roboEyes.setBorderradius(25, 25);
    roboEyes.setSpacebetween(30);
    roboEyes.setMood(TIRED);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(OFF); // Không blink khi ngủ
    roboEyes.setIdleMode(OFF);
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt trắng sáng
  }
  void setSadMode() {
    backgroundColor = TFT_CYAN;       // Giữ cyan sáng
    primaryColor = TFT_BLUE;
    secondaryColor = TFT_NAVY;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(45, 45);
    roboEyes.setHeight(70, 70); // Mắt to buồn
    roboEyes.setBorderradius(15, 15);
    roboEyes.setSpacebetween(35);
    roboEyes.setMood(TIRED); // Dùng TIRED cho vẻ buồn
    roboEyes.setPosition(S); // Nhìn xuống
    roboEyes.setAutoblinker(ON, 4, 2); // Blink chậm
    roboEyes.setVFlicker(ON, 2); // Rung nhẹ
    roboEyes.setEyeColor(TFT_BLUE);  // Mắt xanh dương trên nền cyan
  }
    void setLoveMode() {
    backgroundColor = TFT_MAGENTA;
    primaryColor = TFT_WHITE;
    secondaryColor = TFT_PINK;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(55, 55);
    roboEyes.setHeight(55, 55);
    roboEyes.setBorderradius(25, 25); // Tròn như trái tim
    roboEyes.setSpacebetween(25);
    roboEyes.setMood(HAPPY);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 2, 1);
    roboEyes.setIdleMode(ON, 3, 2);
    roboEyes.setEyeColor(TFT_PINK);  // Mắt hồng cho love mode
  }
    void setSurpriseMode() {
    backgroundColor = TFT_WHITE;
    primaryColor = TFT_BLACK;
    secondaryColor = TFT_LIGHTGREY;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(80, 80); // Mắt rất to
    roboEyes.setHeight(80, 80);
    roboEyes.setBorderradius(40, 40);
    roboEyes.setSpacebetween(20);
    roboEyes.setMood(DEFAULT);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 1, 0); // Blink rất nhanh
    roboEyes.setCuriosity(ON); // Mắt thay đổi khi di chuyển
    roboEyes.setEyeColor(TFT_BLACK);  // Mắt đen trên nền trắng
  }
    void setWinkMode() {
    backgroundColor = TFT_GREEN;
    primaryColor = TFT_WHITE;
    secondaryColor = TFT_DARKGREEN;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(50, 50);
    roboEyes.setHeight(50, 50);
    roboEyes.setBorderradius(12, 12);
    roboEyes.setSpacebetween(30);
    roboEyes.setMood(HAPPY);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 1, 0); // Blink liên tục
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt trắng trên nền xanh
  }
    void setConfusedMode() {
    backgroundColor = TFT_PURPLE;
    primaryColor = TFT_WHITE;
    secondaryColor = TFT_VIOLET;
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(50, 50);
    roboEyes.setHeight(50, 50);
    roboEyes.setBorderradius(12, 12);
    roboEyes.setSpacebetween(35);
    roboEyes.setMood(DEFAULT);
    roboEyes.setPosition(DEFAULT);
    roboEyes.setAutoblinker(ON, 2, 1);
    roboEyes.setHFlicker(ON, 5); // Rung mạnh ngang
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt trắng trên nền tím
    
    // Chạy animation confused
    roboEyes.anim_confused();
  }
  void setTiredMode() {
    backgroundColor = TFT_OLIVE;     // Sáng hơn thay vì DARKGREY
    primaryColor = TFT_WHITE;        // Sáng hơn
    secondaryColor = TFT_YELLOW;     // Sáng hơn
    
    tft.fillScreen(backgroundColor);
    
    roboEyes.setWidth(50, 50);
    roboEyes.setHeight(25, 25); // Mắt nửa nhắm
    roboEyes.setBorderradius(15, 15);
    roboEyes.setSpacebetween(30);
    roboEyes.setMood(TIRED);
    roboEyes.setPosition(S); // Nhìn xuống
    roboEyes.setAutoblinker(ON, 5, 3); // Blink rất chậm
    roboEyes.setVFlicker(ON, 1); // Rung nhẹ
    roboEyes.setEyeColor(TFT_WHITE);  // Mắt trắng trên nền olive
  }
    void playTransitionAnimation() {
    // Animation chuyển đổi giữa các cảm xúc
    switch(currentEmotion) {
      case EMO_CONFUSED:
        roboEyes.anim_confused();
        break;
      case EMO_HAPPY:
        roboEyes.anim_laugh();
        break;
    }
  }
  
  void updateBackgroundEffects() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastBackgroundUpdate > 100) { // Update mỗi 100ms
      lastBackgroundUpdate = currentTime;
        switch(currentEmotion) {
        case EMO_LOVE:
          drawFloatingHearts();
          break;
          
        case EMO_SURPRISE:
          drawShockLines();
          break;
          
        case EMO_ANGRY:
          drawAngryEffects();
          break;
      }
    }
  }
  
  void updateSpecialAnimations() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastAnimationUpdate > 500) { // Update mỗi 500ms
      lastAnimationUpdate = currentTime;
        if (currentEmotion == EMO_WINK) {
        // Tạo hiệu ứng wink đặc biệt
        animationStep = (animationStep + 1) % 4;
      }
      
      if (currentEmotion == EMO_CONFUSED && animationStep < 6) {
        // Chạy animation confused liên tục
        roboEyes.anim_confused();
        animationStep++;
      }
    }
  }
    void drawEmotionOverlay() {
    switch(currentEmotion) {
      case EMO_SAD:
        drawTears();
        break;
        
      case EMO_ANGRY:
        drawEyebrows();
        break;
        
      case EMO_SLEEP:
        drawSleepZzz();
        break;
    }
  }
    void drawFloatingHearts() {
    // Vẽ trái tim bay lơ lửng (điều chỉnh vị trí với padding)
    static int heartY1 = 240, heartY2 = 240, heartY3 = 240;
    
    // Xóa trái tim cũ
    tft.fillRect(60, heartY1-5, 20, 20, backgroundColor);  // +10px padding
    tft.fillRect(160, heartY2-5, 15, 15, backgroundColor); // +10px padding
    tft.fillRect(260, heartY3-5, 18, 18, backgroundColor); // +10px padding
    
    // Di chuyển lên
    heartY1 -= 2;
    heartY2 -= 3;
    heartY3 -= 1;
    
    // Reset khi ra khỏi màn hình
    if (heartY1 < -20) heartY1 = 260;
    if (heartY2 < -20) heartY2 = 270;
    if (heartY3 < -20) heartY3 = 250;
    
    // Vẽ trái tim mới (điều chỉnh vị trí với padding)
    drawHeart(60, heartY1, 8, TFT_RED);   // +10px padding
    drawHeart(160, heartY2, 6, TFT_PINK); // +10px padding
    drawHeart(270, heartY3, 7, TFT_RED);  // +10px padding
  }
  
  void drawHeart(int x, int y, int size, uint16_t color) {
    // Vẽ trái tim đơn giản
    tft.fillCircle(x - size/2, y, size/2, color);
    tft.fillCircle(x + size/2, y, size/2, color);
    tft.fillTriangle(x - size, y + size/4, x + size, y + size/4, x, y + size, color);
  }
    void drawShockLines() {
    // Vẽ các đường shock xung quanh (điều chỉnh với padding)
    static int shockFrame = 0;
    shockFrame = (shockFrame + 1) % 8;
    
    if (shockFrame < 4) {
      // Vẽ các đường shock
      tft.drawLine(20, 50 + shockFrame * 5, 40, 70 + shockFrame * 5, TFT_YELLOW);     // +10px padding
      tft.drawLine(280, 60 + shockFrame * 3, 300, 80 + shockFrame * 3, TFT_YELLOW);   // -10px từ right
      tft.drawLine(25, 180 - shockFrame * 4, 45, 200 - shockFrame * 4, TFT_YELLOW);   // +10px padding
      tft.drawLine(275, 170 - shockFrame * 2, 295, 190 - shockFrame * 2, TFT_YELLOW); // -10px từ right
    } else {
      // Xóa các đường shock
      tft.fillRect(20, 50, 25, 100, backgroundColor);
      tft.fillRect(275, 60, 25, 100, backgroundColor);
      tft.fillRect(25, 140, 25, 80, backgroundColor);
      tft.fillRect(275, 150, 25, 60, backgroundColor);
    }
  }
    void drawAngryEffects() {
    // Vẽ các dấu hiệu tức giận (điều chỉnh với padding)
    static bool showAngrySymbols = true;
    showAngrySymbols = !showAngrySymbols;
    
    if (showAngrySymbols) {
      // Vẽ dấu # tức giận
      tft.setTextColor(TFT_WHITE, backgroundColor);
      tft.setTextSize(3);
      tft.drawString("#", 40, 30);   // +10px padding
      tft.drawString("*", 280, 40);  // -10px từ right
      tft.drawString("!", 290, 180); // -10px từ right
    } else {
      // Xóa các ký hiệu
      tft.fillRect(40, 30, 30, 30, backgroundColor);
      tft.fillRect(280, 40, 30, 30, backgroundColor);
      tft.fillRect(290, 180, 30, 30, backgroundColor);
    }
  }
  
  void drawTears() {
    // Vẽ nước mắt
    static int tearY1 = 140, tearY2 = 145;
    
    // Xóa nước mắt cũ
    tft.fillRect(80, tearY1-5, 8, 20, backgroundColor);
    tft.fillRect(200, tearY2-5, 8, 20, backgroundColor);
    
    // Di chuyển xuống
    tearY1 += 3;
    tearY2 += 2;
    
    // Reset khi chạm đáy
    if (tearY1 > 220) tearY1 = 140;
    if (tearY2 > 220) tearY2 = 145;
      // Vẽ nước mắt mới
    tft.fillCircle(85, tearY1, 3, TFT_BLUE);
    tft.fillRect(83, tearY1, 4, 8, TFT_BLUE);
      tft.fillCircle(205, tearY2, 4, TFT_CYAN);
    tft.fillRect(202, tearY2, 6, 10, TFT_CYAN);
  }
    void drawEyebrows() {
    // Vẽ lông mày giận dữ (điều chỉnh với padding)
    tft.drawLine(80, 70, 120, 85, TFT_BLACK); // Lông mày trái +10px
    tft.drawLine(81, 71, 121, 86, TFT_BLACK);
    
    tft.drawLine(200, 85, 240, 70, TFT_BLACK); // Lông mày phải -10px
    tft.drawLine(201, 86, 241, 71, TFT_BLACK);
  }
  
  void drawSleepZzz() {
    // Vẽ Zzz ngủ
    static int zzzY1 = 80, zzzY2 = 60, zzzY3 = 40;
    static unsigned long lastZzzUpdate = 0;
    
    if (millis() - lastZzzUpdate > 150) {
      lastZzzUpdate = millis();
      
      // Xóa Zzz cũ
      tft.fillRect(250, zzzY1-5, 30, 20, backgroundColor);
      tft.fillRect(270, zzzY2-5, 25, 15, backgroundColor);
      tft.fillRect(290, zzzY3-5, 20, 12, backgroundColor);
      
      // Di chuyển lên
      zzzY1 -= 1;
      zzzY2 -= 1;
      zzzY3 -= 1;
      
      // Reset
      if (zzzY1 < 20) zzzY1 = 100;
      if (zzzY2 < 20) zzzY2 = 120;
      if (zzzY3 < 20) zzzY3 = 140;
      
      // Vẽ Zzz mới
      tft.setTextColor(TFT_WHITE, backgroundColor);
      tft.setTextSize(2);
      tft.drawString("Z", 250, zzzY1);
      tft.setTextSize(1);
      tft.drawString("Z", 270, zzzY2);
      tft.drawString("z", 290, zzzY3);
    }
  }
};

// Khởi tạo đối tượng
EmotionalPet pet;
String serialCommand = "";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Khởi tạo pet
  pet.begin();
  
  Serial.println("=== ESP32 Emotional Pet với FluxGarage RoboEyes ===");
  Serial.println("Commands:");
  Serial.println(":normal   - Normal mode");
  Serial.println(":happy    - Happy mode (vui vẻ)");
  Serial.println(":angry    - Angry mode (tức giận)");
  Serial.println(":sleep    - Sleep mode (ngủ)");
  Serial.println(":sad      - Sad mode (buồn)");
  Serial.println(":love     - Love mode (yêu thương)");
  Serial.println(":surprise - Surprise mode (ngạc nhiên)");
  Serial.println(":wink     - Wink mode (nháy mắt)");
  Serial.println(":confused - Confused mode (bối rối)");
  Serial.println(":tired    - Tired mode (mệt mỏi)");
  Serial.println("================================================");
}

void loop() {
  // Đọc lệnh từ Serial
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialCommand.length() > 0) {
        processCommand(serialCommand);
        serialCommand = "";
      }
    } else {
      serialCommand += c;
    }
  }
  
  // Update pet
  pet.update();
}

void processCommand(String command) {
  command.trim();
  command.toLowerCase();
  
  Serial.println("Received: " + command);
    if (command == ":normal") {
    pet.setEmotion(EMO_NORMAL);
    Serial.println("→ Pet: Normal mode activated");
  }
  else if (command == ":happy") {
    pet.setEmotion(EMO_HAPPY);
    Serial.println("→ Pet: Happy mode activated! (◕‿◕)");
  }
  else if (command == ":angry") {
    pet.setEmotion(EMO_ANGRY);
    Serial.println("→ Pet: Angry mode activated! (ಠ_ಠ)");
  }
  else if (command == ":sleep") {
    pet.setEmotion(EMO_SLEEP);
    Serial.println("→ Pet: Sleep mode activated... (˘▾˘)~♪");
  }
  else if (command == ":sad") {
    pet.setEmotion(EMO_SAD);
    Serial.println("→ Pet: Sad mode activated... (╥﹏╥)");
  }
  else if (command == ":love") {
    pet.setEmotion(EMO_LOVE);
    Serial.println("→ Pet: Love mode activated! (♥‿♥)");
  }
  else if (command == ":surprise") {
    pet.setEmotion(EMO_SURPRISE);
    Serial.println("→ Pet: Surprise mode activated! (◉_◉)");
  }
  else if (command == ":wink") {
    pet.setEmotion(EMO_WINK);
    Serial.println("→ Pet: Wink mode activated! (◕‿-)");
  }
  else if (command == ":confused") {
    pet.setEmotion(EMO_CONFUSED);
    Serial.println("→ Pet: Confused mode activated... (・・?)");
  }
  else if (command == ":tired") {
    pet.setEmotion(EMO_TIRED);
    Serial.println("→ Pet: Tired mode activated... (－_－) zzZ");
  }
  else {
    Serial.println("→ Unknown command: " + command);
    Serial.println("   Type one of: :normal, :happy, :angry, :sleep, :sad, :love, :surprise, :wink, :confused, :tired");
  }
}