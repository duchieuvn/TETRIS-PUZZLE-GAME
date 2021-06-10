#include <iostream>
#include <string>
#include <windows.h> 
#include <stdlib.h>
#include <time.h>
#include <fstream>


using namespace std;

const int WIDTH = 18;
const int HEIGHT = 15;

const int KEYTIME = 50;
const int SLEEPTIME = 100;

const int BLOCKZ1[4] = {5,6,10,11};
const int BLOCKZ2[4] = {5,6,8,9};
const int BLOCKL1[4] = {2,6,9,10};
const int BLOCKL2[4] = {1,5,9,10};
const int BLOCKI[4] = {2,6,10,14};
const int BLOCKO[4] = {5,6,9,10};


void sort(int a[], int n){
    for (int i = 0 ; i < n-1 ; i++){
        for (int j = i + 1; j < n ; j++ ){
            if (a[i] > a[j])
                swap(a[i], a[j]);
        }
    }
}

void gotoxy( int column, int line ){

    COORD coord;
    coord.X = column;
    coord.Y = line;

    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), coord );
}

class Block{
public:
    int x;
    float y;
    int solid[4] = {5,6,10,11};
    int mostLeft = 1;
    int mostRight = 3;
    int top = 1;
    int bottom = 2;

    Block();
    Block(const int a[]);
    void drop(float speed);
    void moveLeft();
    void moveRight();
    void rotateRight();
    void makeBlock(const int a[]);
    void drawBlock();
    
};

Block::Block(){
    x = WIDTH/2 - 2;
    y = -1;
}

Block::Block(const int a[]){
    Block();

    for (int i = 0 ; i < 4 ; i++)
        solid[i] = a[i];
}

void Block::makeBlock(const int a[]){


    for (int i = 0 ; i < 4 ; i++)
        solid[i] = a[i];
}


void Block::drawBlock(){
    int count = 0;
    int check = 0;
    for (int i = 0 ; i < 4 ; i++){
        for (int j = 0 ; j < 4 ; j++){
            if (count == solid[check]){
                gotoxy(x + j, y + i);
                cout << "#";
                check++;
                if (check > 3)
                    return;
            }

            count++;
        }
    }
}

void Block::drop(float speed){
    
    y += speed;
}

void Block::moveLeft(){
    x--;
}

void Block::moveRight(){
    x++;
}

void Block::rotateRight(){ 
    for (int i = 0 ; i < 4 ; i++){
        int tempY = solid[i] / 4 ;
        int tempX = solid[i] - tempY*4 ;

        int newY = tempX;
        int newX = 3 - tempY;

        solid[i] = newY*4 + newX;
    }

    sort(solid, 4);
    int temp = top;
    top = mostLeft;
    mostLeft = bottom;
    bottom = mostRight;
    mostRight = temp;

}


class Game{
private:
    int score = 0;
    int highScore = 0;
    Block curBlock;
    string Display[HEIGHT];
    bool Container[100][100]; 
    float speed = 0.25;

public:
    Game();
    void updateScreen();

    void drawScreen();

    void checkKey();

    void handleBottom();

    void startGame();

    bool canDrop();
    bool canMoveLeft();
    bool canMoveRight();
    int canRotate();

    void pinBlock();

    void clearBlock();

    void checkRow();

    void clearRow(int row);

    bool loseGame();

    void saveScore();

    void loadScore();

    void drawScore();
    
    Block generateBlock();

};

Game::Game(){

    curBlock = generateBlock();

    for (int i = 0 ; i < HEIGHT-1 ; i++){
        Display[i] = "@";

        for (int j = 1 ; j <= WIDTH-2; j++)
            Display[i].append(" ");

        Display[i].append("@");
    }

    Display[HEIGHT-1] = "";
    for (int j = 0 ; j < WIDTH; j++)
        Display[HEIGHT-1].append("@");

    for (int i = 0 ; i < HEIGHT-1 ; i++){
        for (int j = 0 ; j < WIDTH-2 ; j++){
            Container[i][j] = 0;
        }
    }
}

void Game::saveScore(){

    if (score > highScore){
        ofstream fout("score.bin", ios::binary);
        fout.write((char*)&score, sizeof(score));
        fout.close();

    }

}

void Game::loadScore(){
    ifstream fin("score.bin", ios::binary);

    if (!fin){
        gotoxy(WIDTH+2, 1);
        cout << "Best score: 0";
        return;
    }

    fin.read((char*)&highScore, sizeof(highScore));

    fin.close();

    gotoxy(WIDTH+2, 1);
    cout << "Best score: " << highScore;
}

bool Game::canDrop(){

    for (int i = 0 ; i < 4 ; i++){
        int y = int(curBlock.y) + (curBlock.solid[i] / 4);
        int x = curBlock.x + (curBlock.solid[i] % 4);

        //if the block hit the bottom
        if (y+1 > HEIGHT-2){
            return false;
        }
        
        if (Container[y+1][x-1] == 1 ){
            return false;
        }
    }

    return true;
}

bool Game::loseGame(){ 
    for (int i = 0 ; i < WIDTH-2 ; i++)
        if (Container[0][i] == 1)
            return true;
    
    return false;
}

bool Game::canMoveLeft(){
    for (int i = 0 ; i < 4 ; i++){
        int y = int(curBlock.y) + (curBlock.solid[i] / 4);
        int x = curBlock.x + (curBlock.solid[i] % 4);
        
        // if the Block hit the wall
        if (x-1 < 1){
            return false;
        }

        if (Container[y][x-2] == 1){
            return false;
        }
    }

    return true;

}

bool Game::canMoveRight(){
    for (int i = 0 ; i < 4 ; i++){
        int y = int(curBlock.y) + (curBlock.solid[i] / 4);
        int x = curBlock.x + (curBlock.solid[i] % 4);
        
        // if the Block hit the wall
        if (x+1 > WIDTH-2){
            return false;
        }

        if (Container[y][x] == 1){
            return false;
        }
    }

    return true;

}

int Game::canRotate(){ 
    // Left = 1;
    // Right = 2;
    // Bottom = 0;
    // OK = 3;

    Block tempBlock = curBlock;

    tempBlock.rotateRight();

    for (int i = 0 ; i < 4 ; i++){
        int y = int(tempBlock.y) + (tempBlock.solid[i] / 4);
        int x = tempBlock.x + (tempBlock.solid[i] % 4);
        
        if (Container[y][x-2] == 1 || x < 1)
            return 1;
        
        if (Container[y][x] == 1 || x >= WIDTH-1)
            return 2;

        if (Container[y][x-1] == 1 || y >= HEIGHT-1)
            return 0;
    }

    return 3;

}
void Game::updateScreen(){ 
    
    //create a blank line
    for (int i = 0 ; i < 4 ; i++){
        int y = int(curBlock.y) + (curBlock.solid[i] / 4);
        int x = curBlock.x + (curBlock.solid[i] % 4);

        if (y >= 0)
            Display[y][x] = '#';
    }

    
}

void Game::drawScreen(){
 
    Display[HEIGHT-1] = "";
    for (int j = 0 ; j <= WIDTH-1; j++)
        Display[HEIGHT-1].append("@");

    for (int i = 0; i < HEIGHT ; i++){
        cout << Display[i] << endl;
    }


}

void Game::clearBlock(){
    
    //clear the last block
    for (int i = 0 ; i < 4 ; i++){
        int y = int(curBlock.y) + (curBlock.solid[i] / 4);
        int x = curBlock.x + (curBlock.solid[i] % 4);
        
        if (y >= 0)
            Display[y][x] = ' ';
    }

}

void Game::clearRow(int row){
    for (int i = row; i > 0 ; i--){
        for (int j = 0 ; j < WIDTH-2 ; j++){
            Container[i][j] = Container[i-1][j];
            Display[i][j+1] = Display[i-1][j+1];
        }
    }
}

void Game::checkKey(){ 


        if (GetAsyncKeyState(VK_LEFT)){
            if (canMoveLeft()){
                clearBlock();
                
                curBlock.moveLeft();
                Sleep(KEYTIME);
            }
            

        }
        if (GetAsyncKeyState(VK_RIGHT)){
            if (canMoveRight()){

                clearBlock();
                curBlock.moveRight();
                Sleep(KEYTIME);
            }
            
           
        }

        if (GetAsyncKeyState(VK_SPACE)){
            // Left = 1;
            // Right = 2;
            // Bottom = 0;
            // OK = 3;


            int flag = canRotate();
            switch (flag)
            {
            case 1:
                clearBlock();
                curBlock.moveRight();
                curBlock.rotateRight();
                break;

            case 2:
                clearBlock();
                curBlock.moveLeft();
                curBlock.rotateRight();
                break;

            case 0:
                break;
                
            default:
                clearBlock();
                curBlock.rotateRight();
                break;
            }

            
        }
        
        if (GetAsyncKeyState(VK_DOWN)){
            if (canDrop())
                curBlock.y = int(curBlock.y) + 1;
        }
}

Block Game::generateBlock(){
    
    int r = rand() % 6;

    Block newBlock;

    switch (r)
    {
    case 0:
        newBlock.makeBlock(BLOCKZ1);
        break;
    case 1:
        newBlock.makeBlock(BLOCKL1);
        break;
    case 2:
        newBlock.makeBlock(BLOCKI);
        break;
    case 3:
        newBlock.makeBlock(BLOCKO);
        break;
    case 4:
        newBlock.makeBlock(BLOCKZ2);
        break;
    case 5: 
        newBlock.makeBlock(BLOCKL2);
        break;
    default:
        newBlock.makeBlock(BLOCKO);
        break;
    }

    return newBlock;
}

void Game::pinBlock(){

    for (int i = 0 ; i < 4 ; i++){
        int y = int(curBlock.y) + (curBlock.solid[i] / 4);
        int x = curBlock.x + (curBlock.solid[i] % 4);

        if (y >= 0)
            Container[y][x-1] = 1;
    }
}

void Game::checkRow(){

    int fullRow[HEIGHT];
    int rowNumber = 0;

    // find the full rows
    for (int row = HEIGHT-2; row >= 0 ; row--){
        

        int check = 1;
        for (int j = 0 ; j < WIDTH-2 ; j++){
            check *= Container[row][j];

            if (check == 0){
                
                break;
            }
        }

        // if the Row is full 
        if (check == 1){
            
            //store the index of the row
            fullRow[rowNumber] = row;
            rowNumber++;
            
            score++;
            
        }
    }

    if (rowNumber > 0){
        // display the full row by '============'
        for (int i = 0; i < rowNumber; i++){
            for (int j = 1 ; j < WIDTH-1 ; j++){
                    Display[fullRow[i]][j] = '=';
                }
        }
        
        gotoxy(0,0);
        drawScreen();
        Sleep(500);

        for (int i = rowNumber-1; i >= 0; i--){
            clearRow(fullRow[i]);
            
        }

        drawScore();
    }

    
}


void Game::handleBottom(){

    static float count = 0;

    if (canDrop()){
        
        clearBlock();
        curBlock.drop(speed);

    }
    else{
        
        if (count >= 1){
            count = 0;
            pinBlock();
            
            checkRow();

            curBlock = generateBlock();

            Sleep(KEYTIME);
        }
        else{

            count += speed;
        }

    }
}

void Game::startGame(){
    srand(time(NULL));  
    loadScore();
    drawScore();

    while (!loseGame()){

        updateScreen();
        gotoxy(0, 0);
        drawScreen();

        
        handleBottom();
        
        Sleep(SLEEPTIME);


        checkKey();

    }

    saveScore();

}

void Game::drawScore(){
    gotoxy(WIDTH+2, 4);
    cout << "Your score: " << score;
}

int main(){
    Game Tetris;

    Tetris.startGame();
    
    system("pause");
    
    return 0;
}