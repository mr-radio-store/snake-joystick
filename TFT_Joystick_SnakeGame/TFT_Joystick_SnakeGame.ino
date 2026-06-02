/*
Snake Game 
TFT LCD ST7735S
Analog Joystick

Wire Connection TFT + UNO
ST7735 Pin	Arduino Pin
VCC	        5V or 3.3V
GND	        GND
CS	        D10
RST	        D9
DC (A0)	D8
SDA (MOSI)	D11
SCK	        D13

Wiring for Analog Joystick + Arduino Uno
Joystick Pin	Arduino Uno Pin	Function
VCC	            5V	            Power supply
GND	            GND	            Ground
VRx	            A0	            Horizontal analog input
VRy	            A1	            Vertical analog input
SW	            D2 (optional)	Button (digital read)
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// TFT Pins
#define TFT_CS     10
#define TFT_RST    9
#define TFT_DC     8

// Joystick Pins
#define JOY_X A0
#define JOY_Y A1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define CELL_SIZE 8
#define MAX_SNAKE 100
#define MAX_RESTARTS 10

int WIDTH, HEIGHT;
int cellsX, cellsY;

int snakeX[MAX_SNAKE];
int snakeY[MAX_SNAKE];
int snakeLength = 5;
int dirX = 1;
int dirY = 0;
int foodX, foodY;

int restartCount = 0;
bool gameOverLimitReached = false;

void drawCell(int x, int y, uint16_t color) {
  tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1, color);
}

void spawnFood() {
  foodX = random(cellsX);
  foodY = random(cellsY);
}

void resetGame() {
  tft.fillScreen(ST77XX_BLACK);
  snakeLength = 5;
  dirX = 1;
  dirY = 0;

  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = 5 - i;
    snakeY[i] = 5;
  }

  spawnFood();
}

void showGameOverAndRestart() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 60);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.print("Game Over");
  delay(1000);

  restartCount++;
  if (restartCount >= MAX_RESTARTS) {
    gameOverLimitReached = true;
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 50);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.print("Max restarts reached!");
    tft.setCursor(10, 70);
    tft.print("Reset Arduino");
    while (true);  // halt forever
  }

  resetGame();
}

void setup() {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  WIDTH = tft.width();
  HEIGHT = tft.height();
  cellsX = WIDTH / CELL_SIZE;
  cellsY = HEIGHT / CELL_SIZE;

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("Snake Game Ready");
  delay(1000);

  randomSeed(analogRead(A2));
  resetGame();
}

void loop() {
  if (gameOverLimitReached) return;

  // Read joystick
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  if (xVal < 300 && dirX == 0) { dirX = -1; dirY = 0; }
  else if (xVal > 700 && dirX == 0) { dirX = 1;  dirY = 0; }
  else if (yVal < 300 && dirY == 0) { dirX = 0;  dirY = -1; }
  else if (yVal > 700 && dirY == 0) { dirX = 0;  dirY = 1; }

  // Compute new head position
  int newX = snakeX[0] + dirX;
  int newY = snakeY[0] + dirY;

  // Check wall collision
  if (newX < 0 || newX >= cellsX || newY < 0 || newY >= cellsY) {
    showGameOverAndRestart();
    return;
  }

  // Check self collision
  for (int i = 0; i < snakeLength; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
      showGameOverAndRestart();
      return;
    }
  }

  // Shift body
  for (int i = min(snakeLength, MAX_SNAKE - 1); i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // Move head
  snakeX[0] = newX;
  snakeY[0] = newY;

  // Eat food
  bool ate = false;
  if (newX == foodX && newY == foodY) {
    if (snakeLength < MAX_SNAKE) snakeLength++;
    spawnFood();
    ate = true;
  }

  // Erase tail if not eating
  if (!ate) {
    drawCell(snakeX[snakeLength], snakeY[snakeLength], ST77XX_BLACK);
  }

  // Draw snake head and food
  drawCell(snakeX[0], snakeY[0], ST77XX_GREEN);
  drawCell(foodX, foodY, ST77XX_RED);

  delay(150);  // Game speed
}
