#include <iostream>
#include <conio.h>     // For _kbhit() and _getch() (keyboard input)
#include <windows.h>   // For Sleep() and system functions
#include <stdlib.h>    // For rand() and srand()
#include <ctime>       // For time() to seed random number
#include <fstream>     // For file handling (high scores)

using namespace std;

// ==================== GLOBAL CONSTANTS ====================
const int WIDTH = 60;      // Road width (characters)
const int HEIGHT = 25;     // Road height (rows)
const int MAX_CARS = 10;   // Maximum opponent cars (dynamic allocation ke liye)

// ==================== CAR CLASS ====================
class Car {
private:
    int x;          // X position (left to right)
    int y;          // Y position (top to bottom)
    char symbol;    // Car ka symbol (ASCII)
    
public:
    // Constructor
    Car() {
        x = WIDTH / 2;
        y = 0;
        symbol = 'V';  // V shape for car
    }
    
    // Parameterized constructor
    Car(int startX, int startY, char sym) {
        x = startX;
        y = startY;
        symbol = sym;
    }
    
    // Getter functions
    int getX() { return x; }
    int getY() { return y; }
    char getSymbol() { return symbol; }
    
    // Setter functions
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    
    // Move functions
    void moveLeft() { 
        if(x > 2) x--;  // Left boundary check
    }
    
    void moveRight() { 
        if(x < WIDTH - 2) x++;  // Right boundary check
    }
    
    void moveDown() { 
        y++;  // Neeche ki taraf move
    }
};

// ==================== OPPONENT CAR CLASS ====================
// Dynamic memory ke liye separate class
class OpponentCar {
public:
    int x;           // X position
    int y;           // Y position
    char symbol;     // Car symbol
    bool active;     // Car active hai ya destroy ho gayi
    
    // Constructor
    OpponentCar() {
        x = 2 + rand() % (WIDTH - 4);  // Random X position (boundaries ke andar)
        y = 0;                          // Top se start
        symbol = '^';                   // ^ shape for opponent car
        active = true;                  // Initially active
    }
    
    // Move opponent car down
    void moveDown() {
        if(active) {
            y++;
        }
    }
};

// ==================== GAME CLASS ====================
class RacingGame {
private:
    Car playerCar;                      // Player ki car
    OpponentCar* opponents;             // Dynamic array of opponent cars
    int opponentCount;                  // Current number of opponent cars
    int score;                          // Player score
    int lives;                          // Player lives
    bool isRunning;                     // Game running or not
    int level;                          // Difficulty level
    int speed;                          // Game speed (delay time)
    
public:
    // Constructor - Game initialize karta hai
    RacingGame() {
        // Initial values set karo
        playerCar = Car(WIDTH/2, HEIGHT-2, 'V');
        opponents = NULL;               // Initially NULL (no opponents)
        opponentCount = 0;
        score = 0;
        lives = 3;
        isRunning = true;
        level = 1;
        speed = 100;                    // Starting speed (milliseconds)
        
        // Random number generator seed karo
        srand(time(0));
        
        // High score file check
        loadHighScore();
    }
    
    // Destructor - Memory free karta hai
    ~RacingGame() {
        // Saari opponent cars ki memory free karo
        if(opponents != NULL) {
            delete[] opponents;
            opponents = NULL;
        }
        cout << "\nGame destroyed. Memory cleaned!" << endl;
    }
    
    // ========== DYNAMIC MEMORY FUNCTIONS ==========
    
    // Add new opponent car (Dynamic array resize)
    void addOpponent() {
        // Naya array create karo (size + 1)
        OpponentCar* newOpponents = new OpponentCar[opponentCount + 1];
        
        // Purani cars ko naye array mein copy karo
        for(int i = 0; i < opponentCount; i++) {
            newOpponents[i] = opponents[i];
        }
        
        // Nayi car add karo
        newOpponents[opponentCount] = OpponentCar();
        
        // Purani array delete karo
        if(opponents != NULL) {
            delete[] opponents;
        }
        
        // Naye array ko point karo
        opponents = newOpponents;
        opponentCount++;
    }
    
    // Remove opponent car at index (Dynamic array shrink)
    void removeOpponent(int index) {
        if(index < 0 || index >= opponentCount) return;
        
        // Agar sirf ek car hai toh
        if(opponentCount == 1) {
            delete[] opponents;
            opponents = NULL;
            opponentCount = 0;
            return;
        }
        
        // Naya array create karo (size - 1)
        OpponentCar* newOpponents = new OpponentCar[opponentCount - 1];
        
        // Copy all cars except the one to remove
        int newIndex = 0;
        for(int i = 0; i < opponentCount; i++) {
            if(i != index) {
                newOpponents[newIndex] = opponents[i];
                newIndex++;
            }
        }
        
        // Purani array delete karo
        delete[] opponents;
        
        // Naye array ko point karo
        opponents = newOpponents;
        opponentCount--;
    }
    
    // ========== GAME MECHANICS ==========
    
    // Spawn new cars randomly
    void spawnCars() {
        // Level ke hisaab se spawn chance badhta hai
        int spawnChance = 5 + (level * 2);  // Level 1: 7%, Level 2: 9%, etc.
        
        // Random number (1 to 100)
        int random = rand() % 100;
        
        // Agar spawn chance mil gaya aur max cars se kam hain
        if(random < spawnChance && opponentCount < MAX_CARS) {
            addOpponent();
        }
    }
    
    // Update all opponent positions
    void updateOpponents() {
        for(int i = 0; i < opponentCount; i++) {
            // Car ko neeche move karo
            opponents[i].moveDown();
            
            // Agar car screen se bahar gayi (bottom touch kiya)
            if(opponents[i].y >= HEIGHT - 1) {
                removeOpponent(i);
                score += 10;  // Score badhao
                i--;  // Index adjust karo because array shrink ho gaya
            }
        }
    }
    
    // Check collisions between player and opponents
    void checkCollisions() {
        for(int i = 0; i < opponentCount; i++) {
            // Check if positions match (collision)
            if(opponents[i].active && 
               opponents[i].x == playerCar.getX() && 
               opponents[i].y == playerCar.getY()) {
                
                // Collision hui!
                lives--;                    // Ek life kam
                removeOpponent(i);          // Opponent car remove karo
                
                // Sound effect ke liye beep
                cout << '\a';  // Beep sound
                
                // Agar lives khatam ho gayin
                if(lives <= 0) {
                    isRunning = false;
                    gameOver();
                }
                
                break;  // Ek frame mein sirf ek collision check karo
            }
        }
    }
    
    // Update level based on score
    void updateLevel() {
        if(score >= 300) {
            level = 5;
            speed = 50;    // Fastest
        }
        else if(score >= 200) {
            level = 4;
            speed = 60;
        }
        else if(score >= 100) {
            level = 3;
            speed = 70;
        }
        else if(score >= 50) {
            level = 2;
            speed = 85;
        }
        else {
            level = 1;
            speed = 100;
        }
    }
    
    // ========== DISPLAY FUNCTIONS ==========
    
    // Draw boundaries (roads)
    void drawBoundaries() {
        // Top boundary
        for(int i = 0; i < WIDTH; i++) {
            cout << "=";
        }
        cout << endl;
        
        // Left and right boundaries for each row will be drawn in drawGame()
    }
    
    // Draw the game screen
    void drawGame() {
        system("cls");  // Clear screen
        
        // Top boundary with score and lives
        cout << "+";
        for(int i = 0; i < WIDTH; i++) cout << "-";
        cout << "+" << endl;
        
        // Score and lives display
        cout << "¦ SCORE: " << score;
        for(int i = 0; i < WIDTH - 12; i++) cout << " ";
        cout << "LIVES: ";
        for(int i = 0; i < lives; i++) cout << "??";
        for(int i = lives; i < 3; i++) cout << "??";
        cout << " ¦" << endl;
        
        cout << "¦";
        for(int i = 0; i < WIDTH; i++) cout << "-";
        cout << "¦" << endl;
        
        // Game area (rows)
        for(int y = 0; y < HEIGHT; y++) {
            cout << "¦";  // Left boundary
            
            // Draw each column in this row
            for(int x = 0; x < WIDTH; x++) {
                bool drawn = false;
                
                // Check if player car is at this position
                if(y == playerCar.getY() && x == playerCar.getX()) {
                    cout << playerCar.getSymbol();
                    drawn = true;
                }
                
                // Check if any opponent car is at this position
                if(!drawn) {
                    for(int i = 0; i < opponentCount; i++) {
                        if(opponents[i].active && 
                           opponents[i].y == y && 
                           opponents[i].x == x) {
                            cout << opponents[i].symbol;
                            drawn = true;
                            break;
                        }
                    }
                }
                
                // Draw road lane lines (every 5 characters)
                if(!drawn) {
                    if(x % 5 == 2) {
                        cout << "|";
                    }
                    else {
                        cout << " ";
                    }
                }
            }
            
            cout << "¦" << endl;  // Right boundary
        }
        
        // Bottom boundary
        cout << "+";
        for(int i = 0; i < WIDTH; i++) cout << "-";
        cout << "+" << endl;
        
        // Controls display
        cout << "CONTROLS: [A] LEFT  [D] RIGHT  [ESC] EXIT" << endl;
        cout << "LEVEL: " << level << "  SPEED: " << (100 - speed + 50) << "%" << endl;
    }
    
    // Handle keyboard input
    void handleInput() {
        if(_kbhit()) {  // Agar koi key press hui hai
            char key = _getch();  // Key read karo
            
            if(key == 'a' || key == 'A') {
                playerCar.moveLeft();
            }
            else if(key == 'd' || key == 'D') {
                playerCar.moveRight();
            }
            else if(key == 27) {  // ESC key (ASCII 27)
                isRunning = false;
                gameOver();
            }
        }
    }
    
    // ========== HIGH SCORE SYSTEM ==========
    
    int highScore;
    
    void loadHighScore() {
        ifstream file("racing_highscore.txt");
        if(file.is_open()) {
            file >> highScore;
            file.close();
        }
        else {
            highScore = 0;  // Default high score
        }
    }
    
    void saveHighScore() {
        if(score > highScore) {
            highScore = score;
            ofstream file("racing_highscore.txt");
            file << highScore;
            file.close();
        }
    }
    
    void showHighScore() {
        cout << "\n?? HIGH SCORE: " << highScore << " ??" << endl;
        if(score > highScore) {
            cout << "?? NEW HIGH SCORE! ??" << endl;
        }
    }
    
    // Game over screen
    void gameOver() {
        system("cls");
        cout << "\n\n";
        cout << "+----------------------------------------+" << endl;
        cout << "¦                                        ¦" << endl;
        cout << "¦          ?? GAME OVER! ??             ¦" << endl;
        cout << "¦                                        ¦" << endl;
        cout << "¦      YOUR SCORE: " << score << "                     ¦" << endl;
        cout << "¦                                        ¦" << endl;
        
        saveHighScore();
        showHighScore();
        
        cout << "¦                                        ¦" << endl;
        cout << "¦   Press SPACE to play again            ¦" << endl;
        cout << "¦   Press ESC to exit                    ¦" << endl;
        cout << "¦                                        ¦" << endl;
        cout << "+----------------------------------------+" << endl;
        
        while(true) {
            if(_kbhit()) {
                char key = _getch();
                if(key == ' ') {  // Space bar
                    resetGame();
                    run();  // Restart game
                    break;
                }
                else if(key == 27) {  // ESC
                    exit(0);
                }
            }
        }
    }
    
    // Reset game (play again)
    void resetGame() {
        // Purani opponent cars delete karo
        if(opponents != NULL) {
            delete[] opponents;
            opponents = NULL;
        }
        
        // Reset variables
        opponentCount = 0;
        score = 0;
        lives = 3;
        level = 1;
        speed = 100;
        isRunning = true;
        playerCar = Car(WIDTH/2, HEIGHT-2, 'V');
    }
    
    // Welcome screen
    void welcomeScreen() {
        system("cls");
        cout << "\n\n";
        cout << "+----------------------------------------------+" << endl;
        cout << "¦                                              ¦" << endl;
        cout << "¦         ???  CAR RACING GAME  ???            ¦" << endl;
        cout << "¦                                              ¦" << endl;
        cout << "¦   AVOID THE ^ CARS!                         ¦" << endl;
        cout << "¦   USE [A] AND [D] TO MOVE                   ¦" << endl;
        cout << "¦   EACH CAR PASSED = 10 POINTS               ¦" << endl;
        cout << "¦   HIGHER LEVEL = FASTER GAME                ¦" << endl;
        cout << "¦                                              ¦" << endl;
        cout << "¦   PRESS ANY KEY TO START...                  ¦" << endl;
        cout << "¦                                              ¦" << endl;
        cout << "+----------------------------------------------+" << endl;
        
        _getch();  // Wait for key press
    }
    
    // ========== MAIN GAME LOOP ==========
    void run() {
        welcomeScreen();
        
        // Game loop
        while(isRunning) {
            handleInput();      // Check keyboard input
            spawnCars();        // Spawn new opponent cars
            updateOpponents();  // Move opponent cars
            checkCollisions();  // Check for collisions
            updateLevel();      // Update difficulty level
            drawGame();         // Draw everything on screen
            
            // Game speed control (Sleep milliseconds mein)
            Sleep(speed);
        }
    }
};

// ==================== MAIN FUNCTION ====================
int main() {
    // Console window size set karo (optional)
    system("title CAR RACING GAME - C++ Project");
    system("mode con: cols=70 lines=35");
    
    // Game object create karo
    RacingGame game;
    
    // Game start karo
    game.run();
    
    cout << "\nThanks for playing!" << endl;
    
    return 0;
}