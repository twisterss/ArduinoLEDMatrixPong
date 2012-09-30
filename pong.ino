/* Code adapted from the code written by dataTeam in August 2011 */
/* Pong game for LED matrix 16x32 by sure electronics */

#include "led_matrix.h"

/**
 * Set these constants to the values of the pins connected to the SureElectronics Module.
 * Think to connect the ground and 5V to the module as well
 */
#define DATA_PIN 6
#define CLK_PIN 9
#define WRCLK_PIN 7
#define CS_PIN 8

/**
 * Set these constants to the pins of the controls
 */
#define PLAYER1_PIN A0
#define PLAYER2_PIN A1
#define BALL_SPEED_PIN A2

/**
 * Possible values for the controls.
 * Should be 0 to 1023 but might be different
 */
#define PLAYER_CONTROL_START 0
#define PLAYER_CONTROL_END 950

/**
 * Game settings
 */
#define PONG_MAX_POINTS 10
#define PONG_BAR_SIZE 5
#define PONG_STEP_DELAY 5      // This is a delay in ms between each game step
#define PONG_FASTEST_BALL 2    // This is a multiple of step_delay (int >= 1)
#define PONG_SLOWEST_BALL 10   // This is a multiple of step_delay (int >= 1)
#define PONG_START_X 2
#define PONG_START_VX 0.5
#define PONS_START_Y 1
#define PONG_START_VY 1

//Pong variables
static float ballX=PONG_START_X;
static float ballY=PONS_START_Y;
static float ballvX=PONG_START_VX;
static float ballvY=PONG_START_VY;
static int barX[2];
static int barY[2];
static int p1 = PONG_MAX_POINTS;
static int p2 = PONG_MAX_POINTS;
static LEDMatrix* matrix;
static int moveDuration = 4;
static int stepIndex = 0;

void initGame() {
  // Display all objects of the game
  setPoints(p1, p2);
  moveBall(ballX, ballY);
  putBar(0, barY[0]);
  putBar(1, barY[1]);
} 


void moveBall(float x, float y) {
  // Move the ball to a new position
  matrix->plot(ballX, ballY, BLACK);
  matrix->plot(x,y,ORANGE);
  ballX = x;
  ballY = y;
}

void putBar(byte bar, byte y) {
  // Move a bar to a new position
  for (int i = 0; i < PONG_BAR_SIZE; i++)
    matrix->plot(barX[bar], barY[bar] + i, BLACK);
  for (int i = 0; i < PONG_BAR_SIZE; i++)
    matrix->plot(barX[bar], y + i, GREEN);
  barY[bar] = y;
}

void setPoints(int points1, int points2) {
  // Display new points for both players
  for (int a=0; a<PONG_MAX_POINTS; a++)
    matrix->plot(0,a,a < points1 ? RED : BLACK);
  for (int a=0; a<PONG_MAX_POINTS; a++)
    matrix->plot(31,a,a < points2 ? RED : BLACK);
  p1 = points1;
  p2 = points2;
}

void gameOver(byte player) {
  // Display a game over animation
  // Choose the direction of the wave depending on the loser
  int from, to, plus;
  if (player == 0) {
    from = 0;
    to = WIDTH;
    plus = 1;
  } else {
    from = WIDTH-1;
    to = -1;
    plus = -1;
  }
  // Display a red wave
  for (int a=from; a!=to; a+=plus) {
    for (int b=0;b<HEIGHT;b++)
      matrix->plot(a,b,RED);
    matrix->commit();
    delay(100);
  }
  delay(3000);
  matrix->clear();
  // Reinitalize the game
  p1 = PONG_MAX_POINTS;
  p2 = PONG_MAX_POINTS;
  ballX = WIDTH/2;
  ballY = HEIGHT/2-1;
  initGame();
}

void manageBall(){
  // moves the balls
  float x = ballX + ballvX;
  float y = ballY + ballvY;
  int absX = x;
  int absY = y;
  
  //up and down collisions
  if((absY == 0 && ballvY < 0) || (absY == 15 && ballvY>0))
    ballvY*=-1;
    
  //collisions with the bars
  if(absX==2 && ballvX<0){
    if(absY>=barY[0] &&  absY<barY[0]+PONG_BAR_SIZE)
      ballvX*=-1;
  }
  
  if(absX==(WIDTH-3) && ballvX>0){
    if(absY>=barY[1] && absY<barY[1]+PONG_BAR_SIZE)
      ballvX*=-1;
  }
  
  //score control
  if(absX == 0){
    x=WIDTH/2 - 1;
    setPoints(p1-1, p2);
    if (p1<1)
      return gameOver(0);    
  }
  if(absX == WIDTH-1){
    x=WIDTH/2 - 1;
    setPoints(p1, p2-1);
    if (p2<1)
      return gameOver(1);
  }
  moveBall(x, y);
}

void readControls() {
  // move the bars
  putBar(0, map(analogRead(PLAYER1_PIN), PLAYER_CONTROL_START, PLAYER_CONTROL_END, 0, HEIGHT - PONG_BAR_SIZE));
  putBar(1, map(analogRead(PLAYER2_PIN), PLAYER_CONTROL_START, PLAYER_CONTROL_END, 0, HEIGHT - PONG_BAR_SIZE));
  // get the speed of the ball
  moveDuration = map(analogRead(BALL_SPEED_PIN), 0, 1023, PONG_FASTEST_BALL, PONG_SLOWEST_BALL);
}


void setup()
{
  // Arduino initalization function
  barX[0] = 2;
  barX[1] = WIDTH-3;
  barY[0] = 0;
  barY[1] = 0;
  // Create the matrix object, which initializes the screen
  matrix = new LEDMatrix(DATA_PIN, CLK_PIN, WRCLK_PIN, CS_PIN);
  initGame();
}

void loop()
{
  // Arduino loop
  // Moves the ball if necessary
  if (stepIndex == (moveDuration - 1))
    manageBall();
  // Read the user controls
  readControls();
  // Display changes on the screen
  matrix->commit();
  // Wait
  delay(PONG_STEP_DELAY);
  stepIndex = (stepIndex + 1) % moveDuration;
}
