#include "FEHLCD.h"
#include "FEHImages.h"
#include "FEHRandom.h"
#include "FEHUtility.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <fstream>

/********************************************************************************************************************************
 * FEH Software Design Project - Rockstar Games (Group B1)
 * Shape Run by Avery Abke, Naomi Mukka, and Samantha Smith
 *******************************************************************************************************************************/

/*FUNCTION PROTOTYPES*/

void createButton(char [], int, int); //creates indivudal buttons with text at a specified point
void mainMenu(); //displays the main menu
void changeScreen(int); //switches to the proper menu screen
void statsScreen(); //displays the stats screen
void instructionsScreen(); //displays the instructions screen
void creditsScreen(); //displays the credits screen
void selectDifficultyScreen(); //displays the select difficulty/start screen
void selectCharacterScreen(); //displays the character select screen
void game(int); //main game code
void generateObstacle(); //used to create new obstacles as game runs
void refresh(); //refreshes screen
void drawBackground(int); //writes the correct background to the screen
void moveBackground(); //makes the background scroll
void flushList(); //clears out the generated list

/*CONSTANTS*/
#define SCREEN_HEIGHT 239
#define SCREEN_WIDTH 319
#define BUTTON_COLOR BLUE //used for the background of buttons
#define OUTLINE_COLOR WHITE //used for button outlines
#define FLOOR_HEIGHT 206

/*VARIABLES*/
int currentScreen = 0; //0 - main menu, 1 - stats, 2 - instructions, 3 - credits, 4 - start/difficulty, 5 - actual game
int xTouch, yTouch;
int difficulty; //1 - easiest -> 3 - hardest
int highScores[5]; //keeps track of top 5 scores
bool gameRunning; 
bool playerDead = false;
int jumpingCounter = 0; //used to check which frame the jump is on
int generatedIndex = 0; 
bool jumpingUp; //true if movement is up, false if movement is down
int bgx = 0; //background position
int frequency; //time between obstacle spawns
int speed; //speed of obstacles

/********************************************************************************************************************************
    CLASSES
********************************************************************************************************************************/

/****************************************************************
 * Player Class
****************************************************************/
class Player{
    private:
        int score;
        bool updateStats(); //updates stats of player 
        int x = 36, y = SCREEN_HEIGHT - 64;
    public:
        char spriteFileName[30];
        void gameOver();
        bool jump(int); //makes the player jump
        void drawPlayer(); //writes player to screen
        int getX(); //returns x value of player 
        int getY(); //returns y value of player 
        void setY(int); //sets Y value of player
        int getScore(); //returns player score
        void setScore(int); //updates player score
        int yAcceleration;
};

/****************************************************************
 * Obstacle class
****************************************************************/
class Obstacle{
    private:
        int posx, posy, width, height;
        int type; // 0-spikes, 1-tall, 2-long, 3-triangle
    public:
        Obstacle(int);
        Obstacle();
        bool onScreen;
        void moveObstacle(); //moves obstacle along the screen
        void drawObstacle(); //writes the obstacle to the screen
        bool checkCollisions(Player); //returns if a collision was detected
};

/****************************************************************
 * Button class
****************************************************************/
class Button{
    private:
        char printString[100];
        int posx, posy, height, width;
    public:
        Button(char [], int, int);
        Button(int, int, int, int);
        Button();
        void drawButton(); //writes button to the screen
        bool pressed(); //returns true if button is pressed
        bool hovered(); //returns true if the cursor is on the button
};

/****************************************************************
 * Character class
****************************************************************/
class Character{
    private:
        int posx, posy, height, width;
    public:
        Button button;
        char fname[100],name[20];
        Character(int, int, char[], char[]);
        void drawCharacter(); //draws the character on the screen
};

/*OBJECTS*/
Button stats("Stats", 7, 215);
Button instructions("Instructions", 75, 215);
Button credits("Credits", 228, 215);
Button start("Start", 130, 170);
Button easy("Easy",40,110);
Button medium("Medium",122,110);
Button hard("Hard",228,110);
Button back("back",0,0);

Character guy(81,82,"guy","Assets/guyFEH.pic");
Character smile(123,82,"smile","Assets/smileFEH.pic");
Character brutus(165,82,"brutus","Assets/brutusFEH.pic");
Character dogBrutus(207,82,"Brutus","Assets/brutusDogFEH.pic");
Character annie(81,120,"annie","Assets/annieFEH.pic");
Character jason(123,120,"jason","Assets/jasonFEH.pic");
Character adam(165,120,"adam","Assets/adamAFEH.pic");
Character pom(207,120,"pomeranian","Assets/pomFEH.pic");
Character ben(81,162,"ben","Assets/benFEH.pic");
Character jimmy(123,162,"jimmy","Assets/jimmyFEH.pic");
Character alex(165,162,"alex","Assets/alexFEH.pic");
Character stevan(207,162,"stevan","Assets/stevanFEH.pic");

Player player;

Obstacle generated[10];
Character characters[12] = {guy,smile,brutus,dogBrutus,annie,jason,adam,pom,ben,jimmy,alex,stevan};

/********************************************************************************************************************************
    MAIN FUNCTION
    Authors: Samantha Smith, Avery Abke, Naomi Mukka
********************************************************************************************************************************/
int main()
{
    //Create Start Screen
    mainMenu();
    while (1) {
        /*MAIN MENU*/
        while (currentScreen == 0){ //MAIN MENU BUTTONS
            if (LCD.Touch(&xTouch, &yTouch)){
                //stats button pressed
                if (stats.pressed()){
                    changeScreen(1);
                }
                //instructions button pressed
                if (instructions.pressed()){
                    changeScreen(2);
                }
                //credits button pressed
                if (credits.pressed()){
                    changeScreen(3);
                }
                //start button pressed
                if (start.pressed()){
                    changeScreen(6);
                }
            }
        } //end main menu buttons

        /*CHARACTER SELECT SCREEN*/
        bool sameScreen = true;
        while (currentScreen == 6){//CHARACTER SELECT BUTTONS
            for(int i=0;i<12;i++) {
                //Check if the cursor is on one of the characters
                while (characters[i].button.hovered()) {
                    //displays name of character
                    LCD.SetFontColor(WHITE);
                    LCD.WriteAt(characters[i].name,159-strlen(characters[i].name)*12/2,46);

                    //character pressed
                    if(LCD.Touch(&xTouch,&yTouch)) {
                        strcpy(player.spriteFileName,characters[i].fname);
                        Sleep(500);
                        changeScreen(4);
                        sameScreen = false;     
                        break;
                    }
                }
                
                if(sameScreen) {
                    LCD.Clear();
                    selectCharacterScreen();
                }
            }

            //back button pressed
            if (back.pressed()){
                changeScreen(0);
                Sleep(500);
            }
        }//end character select buttons
        
        /*DIFFICULTY SELECT SCREEN*/
        while (currentScreen == 4){//DIFFICULTY SELECT BUTTONS
            if (LCD.Touch(&xTouch, &yTouch)){
                //easy button pressed
                if (easy.pressed()){
                    difficulty = 1;
                    changeScreen(5);
                }
                //medium button pressed
                if (medium.pressed()){
                    difficulty = 2;
                    changeScreen(5);
                }
                //hard button pressed
                if (hard.pressed()){
                    difficulty = 3;
                    changeScreen(5);
                }
                //back button pressed
                if (back.pressed()){
                    changeScreen(0);
                }
            }
        }//end difficulty select buttons

        /*Return to main menu if the back button is pressed on any screen*/
        if(back.pressed()){
            changeScreen(0);
        }

        LCD.Update();
    }//program loop
    return 0;
}

/********************************************************************************************************************************
    FUNCTIONS
********************************************************************************************************************************/

//Button class member functions

/****************************************************************
 * Creates a properly sized rectangle with text inside
 * Authors: Samantha Smith
 * @param printString[] String to be printed 
 * @param x x position 
 * @param y y position
****************************************************************/
Button::Button(char string[], int x, int y){
    strcpy(printString,string);
    posx = x;
    posy = y;
    width = strlen(string)*12+2;
    height = 19;
}

/****************************************************************
 * Creates a rectangular button with no text
 * Authors: Naomi Mukka
 * @param x x position 
 * @param y y position
 * @param w width
 * @param h height
****************************************************************/
Button::Button(int x, int y, int w, int h){
    posx = x;
    posy = y;
    width = w;
    height = h;
    strcpy(printString,"");
}

/****************************************************************
 * Default Button Constructor - creates a NULL button
 * Authors: Naomi Mukka
****************************************************************/
Button::Button() {
    posx=posy=width=height=NULL;
}

/****************************************************************
 * Displays the button on the screen
 * Authors: Samantha Smith
****************************************************************/
void Button::drawButton() {
    LCD.SetFontColor(BUTTON_COLOR);
    LCD.FillRectangle(posx, posy, width,height);
    LCD.SetFontColor(OUTLINE_COLOR);
    LCD.DrawRectangle(posx, posy, width,height);
    LCD.WriteAt(printString, posx, posy+3);
}

/****************************************************************
 * Checks if the button is pressed
 * Authors: Samantha Smith
****************************************************************/
bool Button::pressed() {
    if (LCD.Touch(&xTouch,&yTouch)) {
        if(yTouch >= posy && yTouch <= posy+height && xTouch >= posx && xTouch <= posx+width) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/****************************************************************
 * Checks if the cursor is on the button
 * Authors: Samantha Smith
****************************************************************/
bool Button::hovered() {
    LCD.Touch(&xTouch,&yTouch);
    if(yTouch >= posy && yTouch <= posy+height && xTouch >= posx && xTouch <= posx+width) {
        return true;
    } else {
        return false;
    }
}

//Obstacle class member functions

/****************************************************************
 * Obstacle Constructor - creates an obstacle based on a preset 
 * size and places it into the correct starting position
 * Authors: Samantha Smith
 * @param t type of obstacle to create 
****************************************************************/
Obstacle::Obstacle(int t) {
    type = t;
    switch (type){
        case 0: //spike
            width = 32;
            height = 16;
            break;
        case 1: //tall
            width = 32;
            height = 57;
            break;
        case 2: //long
            width = 64;
            height = 30;
            break;
        case 3: //triangle
            width = 31;
            height = 27;
            break;
    }
    posx = SCREEN_WIDTH-width;
    posy = FLOOR_HEIGHT-height+1;
}

/****************************************************************
 * Default Obstacle Constructor - creates a NULL obstacle
 * Authors: Naomi Mukka
****************************************************************/
Obstacle::Obstacle(){
    posx=posy=width=height=NULL;
}

/****************************************************************
 * Move Obstacle member function - moves the obstacle forward if it
 * is not hitting the end of the screen
 * Also clears out the generated list if the index reaches the 
 * end of the list
 * Authors: Naomi Mukka, Samantha Smith
****************************************************************/
void Obstacle::moveObstacle() {
    //float start = TimeNow();
    if(posx<=10) {
       onScreen = false;
        if (generatedIndex == 10) {
            flushList();
        }
    } else {
        posx -= speed;
    }
}

/****************************************************************
 * Flush List Function - wipes the generated list to prevent
 * seg faults
 * Authors: Samantha Smith
****************************************************************/
void flushList(){
    for (int i = 0; i < 10; i++){
        generated[i] = NULL;
    }
    generatedIndex = 0;
}

/****************************************************************
 * Draw Obstacle Member function - draws the proper obstacle image
 * to the screen based on the obstacle's type
 * Authors:
****************************************************************/
void Obstacle::drawObstacle() {
    FEHImage obstacleSprite; //sprite
    switch (type){
        case 0:
            obstacleSprite.Open("Assets/spikesFEH.pic"); //get sprite
            break;
        case 1: 
            obstacleSprite.Open("Assets/tallObstacleFEH.pic"); //get sprite
            break;
        case 2: 
            obstacleSprite.Open("Assets/longObstacleFEH.pic"); //get sprite
            break;
        case 3: 
            obstacleSprite.Open("Assets/triangleFEH.pic"); //get sprite
            break;

    }
    obstacleSprite.Draw(posx,posy); // draw the character
    obstacleSprite.Close(); //close
}

/****************************************************************
 * Check Collisions Member Function
 * Authors: Samantha Smith
 * @param player the player object
 * @return Returns true if a collision is detected between the player
 * and the obstacle
****************************************************************/
bool Obstacle::checkCollisions(Player p){
    bool collision = false;
    //get x/y value of each side of the player 
    int playerLeft = p.getX();
    int playerRight = (playerLeft + 32);
    int playerTop = p.getY();
    int playerBottom = (playerTop + 32);

    //get x/y value of each side of the obstacle 
    int obstacleLeft = posx;
    int obstacleRight = obstacleLeft + width;
    int obstacleTop = posy;
    int obstacleBottom = obstacleTop + height;
    //for all normal shapes except triangle
    if (type != 3){
        //collisions for right side of player body
        if ((playerRight >= obstacleLeft && playerRight <= obstacleRight) && (playerBottom >= obstacleTop && playerBottom <= obstacleBottom)) {
            collision = true;
        }
        //collisions for left side of body
        else if ((playerLeft <= obstacleRight && playerLeft >= obstacleLeft) && (playerBottom >= obstacleTop && playerBottom <= obstacleBottom)) {
            collision = true;
        }
    }
    //collisions with triangle
    else {
        //triangle collision with right side of body to left side of obstacle
        if ((playerRight >= obstacleLeft)&& playerRight <= obstacleLeft + width/2 && playerBottom >= obstacleTop && playerBottom >= (pow(3, 0.5)*(playerRight - obstacleLeft) +obstacleTop)) {
            collision = true;
        }
        //triangle collision with left side of body to left side of obstacle
        else if ((playerLeft >= obstacleLeft)&& playerLeft <= obstacleLeft + width/2 && playerBottom >= obstacleTop && playerBottom >= (pow(3, 0.5)*(obstacleLeft- playerLeft) +obstacleTop)) {
            collision = true;
        }
        //triangle collision with left side of body with right side of obstacle
        if ((playerLeft >= obstacleLeft+ width/2)&& playerLeft <= obstacleRight && playerBottom >= obstacleTop && playerBottom >= (-pow(3, 0.5)*(obstacleRight - playerLeft)+obstacleTop)) {
            collision = true;
        }
        //triangle collision with right side of body with right side of obstacle
        else if ((playerRight >= obstacleLeft+ width/2)&& playerRight <= obstacleRight && playerBottom >= obstacleTop && playerBottom >= (-pow(3, 0.5)*(playerRight - obstacleRight)+obstacleTop)) {
            collision = true;
        }
    }
    //no collsion detected

    if (collision){
        return true;
    }
    else {
        return false;
    } 
}

//Player class member functions 

/****************************************************************
 * Player Jump Member Function - Makes the player jump
 * in a parabolic shape
 * Authors: Samantha Smith, Avery Abke
 * @param time time for calculations
 * @return true if the player has completed the jump
****************************************************************/
bool Player::jump(int time){
    //Adjusts speed of jump based on difficulty
    switch (difficulty) {
        case 1:
            break;
        case 2:
            time *= 2;
            break;
        case 3:
            time *= 2;
            break;
    }
    
    yAcceleration = -2;

    int initialVelocity = 20;
    int yInitial = SCREEN_HEIGHT - 64; //175
    //calculate the height and set y to that height
    setY(SCREEN_HEIGHT - (yInitial + initialVelocity*(time-5) + 0.5 *yAcceleration*pow(time,2)));
    //check if the player is still in the process of jumping
    if(getY() <= yInitial){
        return true;
    }
    //stop the jump process
    else{
        setY(yInitial);
        return false;
    }
}

/****************************************************************
 * Player Draw Member Function - Writes the player image to the screen
 * in the correct position
 * Authors: Avery Abke
****************************************************************/
void Player::drawPlayer(){
    FEHImage playerSprite; //sprite
    playerSprite.Open(spriteFileName); //get sprite
    playerSprite.Draw(player.getX(), player.getY()); // draw the character
    playerSprite.Close(); //close
}

/****************************************************************
 * Player Get X Member Function
 * Authors: Avery Abke
 * @return x value of player
****************************************************************/
int Player::getX(){
    return x;
}

/****************************************************************
 * Player Get Y Member Function
 * Authors: Avery Abke
 * @return y value of player
****************************************************************/
int Player::getY(){
    return y;
}

/****************************************************************
 * Player Set Y Member Function
 * Authors: Avery Abke
 * @param newY new y value of the player
****************************************************************/
void Player::setY(int newY){
    y = newY;
}

/****************************************************************
 * Player Get Score Member Function
 * Authors: Avery Abke
 * @return score of the player
****************************************************************/
int Player::getScore(){
    return score;
}

/****************************************************************
 * Player Get Score Member Function
 * Authors: Avery Abke
 * @param timeElapsed time the score should be set to
****************************************************************/
void Player::setScore(int timeElapsed){
    score = timeElapsed;
}

//Character class member functions

/****************************************************************
 * Character Constructor - creates a 32x32 character
 * Authors: Naomi Mukka
 * @param x x position
 * @param y y position
 * @param n character name
 * @param f character sprite file name
****************************************************************/
Character::Character (int x, int y, char n[], char f[]) {
    strcpy(name,n);
    strcpy(fname,f); 
    posx = x;
    posy = y;
    height = width = 32;
    Button b(posx, posy, width, height);
    button = b;
}

/****************************************************************
 * Draw Character Member function - draws the character to the 
 * screen
 * Authors: Naomi Mukka
****************************************************************/
void Character::drawCharacter() {
    button.drawButton();

    FEHImage name;
    name.Open(fname);
    name.Draw(posx,posy);
    name.Close();
}

//MENU STUFF 

/****************************************************************
 * Creates the main menu screen
 * Authors: Samantha Smith
****************************************************************/
void mainMenu(){
    stats.drawButton();
    instructions.drawButton();
    credits.drawButton();
    start.drawButton();
    FEHImage title;
    title.Open("Assets/titleFEH.pic");
    title.Draw(SCREEN_WIDTH/2 - 78/2, 50);
    title.Close();
    FEHImage playerImage;
    playerImage.Open("Assets/guyFEH.pic");
    playerImage.Draw(45, 80+32);
    playerImage.Close();
    FEHImage obstacleImage;
    obstacleImage.Open("Assets/spikesFEH.pic");
    obstacleImage.Draw(100, 80+48);
    obstacleImage.Draw(180, 80+48);
    obstacleImage.Close();
    obstacleImage.Open("Assets/tallObstacleFEH.pic");
    obstacleImage.Draw(240, 80+10);
    obstacleImage.Close();
    obstacleImage.Open("Assets/triangleFEH.pic");
    obstacleImage.Draw(290, 80+37);
    obstacleImage.Close();
    LCD.SetFontColor(WHITE);
}

/****************************************************************
 * Changes to the specified screen
 * Authors: Samantha Smith
 * @param screen which screen the PROTEUS should display to the user: 
 * 0 - Main Menu,
 * 1 - Stats,
 * 2 - Instructions,
 * 3 - Credits,
 * 4 - Start,
 * 5 - Game
****************************************************************/
void changeScreen(int screen){
    LCD.Clear();
    currentScreen = screen;
    switch (screen){
        case 0: //main menu
            mainMenu();
            break;
        case 1: //stats
            statsScreen();
            break;
        case 2: //instructions
            instructionsScreen();
            break;
        case 3: //Credits
            creditsScreen();
            break;
        case 4: //start
            selectDifficultyScreen();
            break;
        case 5: //game start
            game(difficulty);
            break;
        case 6: //character select screen
            selectCharacterScreen();
            break;
    }
} 

/****************************************************************
 * Creates the stats screen
 * Authors: Naomi Mukka
****************************************************************/
void statsScreen(){
    LCD.WriteAt("High Scores",94,60);
    LCD.DrawHorizontalLine(80,94,226);

    //Display top 5 scores
    int y = 90;
    for(int i=0;i<5;i++) {
        LCD.WriteAt(highScores[i],130,y);
        LCD.WriteAt(" m",166,y);
        y += 20;
    }
    
    //Add back button
    back.drawButton();
}

/****************************************************************
 * Creates the credits screen
 * Authors: Naomi Mukka
****************************************************************/
void creditsScreen(){
    LCD.WriteAt("Developers:",80,80);
    LCD.SetFontColor(PALETURQUOISE);
    LCD.WriteAt("Avery Abke",80,100);
    LCD.WriteAt("Naomi Mukka",80,120);
    LCD.WriteAt("Samantha Smith",80,140);
    back.drawButton();
}

/****************************************************************
 * Creates the instructions screen
 * Authors: Avery Abke
****************************************************************/
void instructionsScreen(){
    back.drawButton();
    LCD.WriteAt("How to Play:", 80, 40);
    LCD.SetFontColor(MAGENTA);
    LCD.WriteAt("---------------", 60,50);
    LCD.SetFontColor(WHITE);
    LCD.WriteAt("Tap to Jump", 80, 80);
    LCD.WriteAt("Avoid anything with spikes", 3, 110);
    LCD.WriteAt("Go for as long as you can", 10, 140);
}

/****************************************************************
 * Creates the start/difficulty select screen
 * Authors: Naomi Mukka
****************************************************************/
void selectDifficultyScreen(){
    easy.drawButton();
    medium.drawButton();
    hard.drawButton();
    back.drawButton();
}

/****************************************************************
 * Creates the character select screen
 * Authors: Naomi Mukka
****************************************************************/
void selectCharacterScreen(){
    back.drawButton();

    LCD.SetFontColor(WHITE);
    LCD.DrawRectangle(96,40,126,30);

    for(int i=0;i<12;i++) {
        characters[i].drawCharacter();
    }        
}


//GAME STUFF

/****************************************************************
 * Refresh Function - redraws the screen and moves the obstacles
 * Authors: Naomi Mukka
****************************************************************/
void refresh() {
    LCD.Clear();
    
    //draws the background based on the difficulty level
    drawBackground(difficulty);

    //draws the floor
    LCD.SetFontColor(BLACK);
    LCD.FillRectangle(0, FLOOR_HEIGHT, 320, 35);
    
    //draws the obstacles currently on screen
    for(int i=0; i<10;i++) {
        if (generated[i].onScreen) {
            generated[i].moveObstacle();
            generated[i].drawObstacle();
        }
    }

    //draws the player
    player.drawPlayer();

    //writes the score
    LCD.WriteAt(player.getScore(), SCREEN_WIDTH - 40, 5);   
    
    //Updates screen
    Sleep(1);
    LCD.Update();
}

/****************************************************************
 * Main Game function
 * Authors: Samantha Smith
 * @param difficulty (1 easiest -> 3 hardest) 
****************************************************************/
void game(int difficulty) {
    //initialize values
    jumpingCounter = 0;
    int onStartWait = 3;
    gameRunning = false;
    generatedIndex = 0;
    player.setY(SCREEN_HEIGHT-64);
    player.setScore(0);
    flushList();
    drawBackground(difficulty);

    //Adjusts speed and obstacle frequency based on the difficulty selected
    switch (difficulty){
        case 1: 
            speed = 10;
            frequency = 3;
            break;
        case 2: 
            speed = 14;
            frequency = 2;
            break;
        case 3:
            speed = 19;
            frequency = 1;
            break;
    }

    LCD.SetFontColor(WHITE);
    LCD.WriteAt("Press Screen to Start!", 20, 120);
    back.drawButton();
    Sleep(500);

    //wait until player presses the screen to start the game
    while (gameRunning == false){
        if (!(back.pressed())){ //triggers if the screen is pressed and not the back button
            if (LCD.Touch(&xTouch, &yTouch)){
                gameRunning = true;
                break;
            }
        }
        else { //triggers if back is pressed to return to menu
            changeScreen(0);
            gameRunning = false;
            break;
        }
    }

    bool jumping = false; //detects if the player is in the air or not, prevents double jumping
    bool generatedThisSecond = false; 

    int startTime;

    //GAMEPLAY LOOP
    playerDead = false;
    Sleep(500);
    while (gameRunning){
        //CREATES A COUNTDOWN IN THE FIRST 3 SECONDS
        if (onStartWait > 0) {
            refresh();
            LCD.WriteAt(onStartWait, SCREEN_WIDTH/2-12, 60);
            LCD.Update();
            Sleep(1.0);
            onStartWait--;
            startTime = time(NULL);
            continue;
        }
        else if (onStartWait == 0){
            refresh();
            LCD.WriteAt("GO!", SCREEN_WIDTH/2-24, 60);
            LCD.Update();
            Sleep(1.0);
            onStartWait--;
            startTime = time(NULL);
        }

        
        player.setScore(time(NULL)-startTime);
        if (playerDead == true){ //check if the player is still alive
            player.gameOver();
            gameRunning = false;
            break;
        }
        
        //Generates an obstacle based on the frequency timing
        int thisSecond = time(NULL)-startTime;
        int timeGenerated;
        if(!generatedThisSecond && thisSecond%frequency == 0) {
            generateObstacle();
            timeGenerated = time(NULL)-startTime;
            generatedThisSecond = true;
        }
        if(thisSecond != timeGenerated) {
            generatedThisSecond = false;
        }

        //check if screen is pressed
        if (LCD.Touch(&xTouch, &yTouch)){
            if (jumping == false){
                jumping = !(jumping);
            }
        }

        //deals with player jumping
        if (jumping == true){
            if (player.jump(jumpingCounter)){ //returns true until the jump has been completed
                jumpingCounter += 1; //increment the time counter for the jump function
            }
            //reset jump variables
            else {    
                jumping = false;
                jumpingCounter = 0;
            }
        }

        //check for collisions with each of the obstacles in the list
        for (int i = 0; i < 10; i++){
            if (generated[i].onScreen){
                if (generated[i].checkCollisions(player)){
                    playerDead = true;
                }
            }
        }

        refresh();
    }//main game loop
}

/****************************************************************
 * Generate Obstacles Function - creates a new obstacle object
 * with a random type and adds it to the generated array
 * Authors: Naomi Mukka
****************************************************************/
void generateObstacle(){
    //creates an obstacle with a random type
    Obstacle a(rand()%4);

    //adds obstacle to screen, array of generated obstacles
    a.onScreen = true;
    generated[generatedIndex] = a;
    generatedIndex++;
}

//POST GAME STUFF 

/****************************************************************
 * Game over function - called when the player dies
 * Pauses gameplay and deals with post game menu displays
 * Authors: Naomi Mukka
****************************************************************/
void Player::gameOver(){
    //Display game over and the player's score
    LCD.Clear(); 
    LCD.WriteAt("GAME OVER",106,60);
    LCD.WriteAt("Score: ",88,80);
    LCD.WriteAt(score,172,80);
    LCD.WriteAt(" m",208,80);

    if(updateStats()){//Display if the player earned a high score
        LCD.WriteAt("HIGH SCORE",100,100);
        LCD.WriteAt("Congratulations!",64,120);
    }
    back.drawButton();
    LCD.Update();
    flushList();
}

/****************************************************************
 * Update Stats Function - evaluates whether the player has made a
 * new record and updates the other stats
 * Authors: Naomi Mukka
 * @return true if the player's score is a high score,
 * @return false if the player's score is not a high score
****************************************************************/
bool Player::updateStats(){
    int placeIndex;

    //Check if the player's score is one of the top 5 scores
    if(score > highScores[4]) {
        //Determine what place the score belongs in
        if(score>highScores[0]){
            placeIndex = 0;
        } else if(score>highScores[1]){
            placeIndex = 1;
        } else if(score>highScores[2]) {
            placeIndex = 2;
        } else if(score>highScores[3]) {
            placeIndex = 3;
        } else {
            placeIndex = 4;
        }

        //Update the scores on the top 5 below the player's score
        for(int i=4; i>placeIndex;i--){
            highScores[i] = highScores[i-1];
        }
        
        //Add the player's score to the list of high scores
        highScores[placeIndex] = score;
        return true;
    } 
    else {
        return false;
    }
}

//BACKGROUND STUFF

/****************************************************************
 * Writes the correct background to the screen based on the difficulty
 * Authors: Avery Abke
 * @param difficulty the difficulty selected by the player
****************************************************************/
void drawBackground(int diff){
    FEHImage background;
    if (gameRunning){
        moveBackground();
    }
    if (diff == 1){
        background.Open("Assets/greenFEH.pic");
        background.Draw(bgx, 0);
        background.Close();
    }
    else if (diff == 2){
        background.Open("Assets/yellowFEH.pic");
        background.Draw(bgx, 0);
        background.Close();
    }
    else if (diff == 3){
        background.Open("Assets/redFEH.pic");
        background.Draw(bgx, 0);
        background.Close();
    }
}

/****************************************************************
 * Scrolls the background
 * Authors: Avery Abke
****************************************************************/
void moveBackground(){
    if (bgx == -320){
        bgx = 0;
    }
    else{
        bgx -= 2;
    }
}