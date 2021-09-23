#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char moves[5] = {'a', 'd', 'w', 's', 'q'};
char charIcon = ' ', poweredIcon = ' ', activeIcon = ' ';
int currentX, currentY;
int items = 0, score = 0;
bool powerMode = false, won = false, collision = false, quit = false;
int turnsOfPower = 0;
char* endMessage = "";

struct Board
{
    int height;
    int width;
    char** board;
};

void setEmptyBoard(const struct Board* b)
{
    for(int i = 0; i < b->height + 2; ++i)
    {
        for(int j = 0; j < b->width + 2; ++j)
        {
            if(i == 0 || i == b->height + 1 || j == 0 || j == b->width + 1)
                b->board[i][j] = '*';
            else
                b->board[i][j] = ' ';
        }
    }
}

void printBoard(const struct Board* b)
{
    for(int i = 0; i < b->height + 2; ++i)
    {
        for(int j = 0; j < b->width + 2; ++j)
            printf("%c", b->board[i][j]);
        printf("\n");
    }
}

void changeBoard(const struct Board* b, char c, int x, int y)
{
    b->board[x + 1][y + 1] = c;
}

void setMoves(FILE *file)
{
    for(int i = 0; i < 5; ++i)
    {
        char scanned[3];
        fgets(scanned, 3, file);
        if(scanned[0] == '\n')
            break;
        if(scanned[0] != ' ')
            moves[i] = scanned[0];
        else
            i--;
    }
}

void setIcons(FILE *file)
{
    fgets(&charIcon, 3, file);
    fgets(&poweredIcon, 3, file);
    activeIcon = charIcon;
}

void addBuilding(struct Board* b, int x, int y)
{
    int realX = x + 1, realY = y + 1;
    for(int i = realX; i < realX+4; ++i)
    {
        for(int j = realY; j < realY+6; ++j)
        {
            b->board[i][j] = '-';
            if((i == realX+2 || i == realX+3) && (j == realY+2 || j == realY+3))
                b->board[i][j] = '&';
        }
    }
}

char getInput()
{
    int valid = 0;
    char input = ' ';
    while(!valid)
    {
        printf("Enter input: ");
        scanf("%s", &input);
        for(int i = 0; i < 5; ++i)
            if(input == moves[i])
                valid = 1;
        if(!valid)
            printf("Invalid input.\n");
    }
    return input;
}

void checkCollision(struct Board* b, char collide)
{
    if(collide == 'X')
    {
        if(!powerMode)
        {
            changeBoard(b, '@', currentX, currentY);
            endMessage = "You have died.\n";
            collision = true;
        }
        else
            score++;

    }
    else if(collide == '$')
    {
        items--;
        score++;
        if(items == 0)
        {
            won = true;
            endMessage = "Congratulations! You have won.\n";
        }
    }
    else if(collide == '*')
    {
        powerMode = true;
        turnsOfPower = 7;
        activeIcon = poweredIcon;
    }
}

void moveChar(struct Board* board, const char* direction)
{
    changeBoard(board, ' ', currentX , currentY);
    if(strcmp(direction, "left") == 0)
    {
        char left = board->board[currentX+1][currentY];
        if(left != '-' && left != '&' && currentY != 0)
        {
            currentY--;
            checkCollision(board, left);
        }
    }
    else if(strcmp(direction, "right") == 0)
    {
        char right = board->board[currentX+1][currentY+2];
        if(right != '-' && right != '&' && currentY != board->width-1)
        {
            currentY++;
            checkCollision(board, right);
        }
    }
    else if(strcmp(direction, "up") == 0)
    {
        char above = board->board[currentX][currentY+1];
        if(above != '-' && above != '&' && currentX != 0)
        {
            currentX--;
            checkCollision(board, above);
        }
    }
    else if(strcmp(direction, "down") == 0)
    {
        char down = board->board[currentX+2][currentY+1];
        if(down != '-' && down != '&' && currentX != board->height-1)
        {
            currentX++;
            checkCollision(board, down);
        }
    }
    if(!collision)
        changeBoard(board, activeIcon, currentX, currentY);
}

void destroyAll(struct Board* board)
{
    for(int i = 0; i < board->height + 2; ++i)
            free(board->board[i]);
    free(board->board);
    free(board);
}

/**
 * Returns 0 if @filename is NULL.
 * Otherwise, returns 1 after the game ends.
 */
int playGame(const char* filename)
{
    if(!filename)
        return 0;
    FILE *file = fopen(filename, "r");

    int height = 0, width = 0;
    fscanf(file, "%d %d", &height, &width);
    struct Board* board = malloc(sizeof(struct Board));
    char** boardArr = malloc((height + 2) * sizeof(char*));
    for(int i = 0; i < height + 2; ++i)
        boardArr[i] = malloc((width + 2) * sizeof(char));
    board->height = height;
    board->width = width;
    board->board = boardArr;
    setEmptyBoard(board);

    char dump = ' ';
    fgets(&dump, 2, file); //end of first line

    setMoves(file); //end of second line

    setIcons(file);
    int startX = 0, startY = 0;
    fscanf(file, "%d %d", &startX, &startY);
    currentX = startX; //coordinate excluding border
    currentY = startY;
    fgets(&dump, 2, file); //end of third line

    changeBoard(board, activeIcon, startX, startY);

    char code = ' ';
    while(1) //setting board with buildings, enemies, items, orbs
    {
        if(fscanf(file, "%c", &code) == -1) //fourth line and beyond
            break;
        int x = 0, y = 0;
        fscanf(file, "%d %d", &x, &y);
        if(code == 'B')
            addBuilding(board, x, y);
        else if(code == 'E')
            changeBoard(board, 'X', x, y);
        else if(code == 'I')
        {
            changeBoard(board, '$', x, y);
            items++;
        }
        else if(code == 'P')
            changeBoard(board, '*', x, y);
        fgets(&dump, 2, file); //end of line (new line dumped)
    }
    printBoard(board);

    bool endCondition = false;

    while(!endCondition)
    {
        printf("Score: %d\n", score);
        printf("Items remaining: %d\n", items);
        char input = getInput();
        //inputs: left (0), right (1), up (2), down (3), quit (4)
        if(turnsOfPower > 0)
            turnsOfPower--;
        if(turnsOfPower == 0)
        {
            powerMode = false;
            activeIcon = charIcon;
        }
        if(input == moves[0])
        {
            moveChar(board, "left");
        }
        else if(input == moves[1])
        {
            moveChar(board, "right");
        }
        else if(input == moves[2])
        {
            moveChar(board, "up");
        }
        else if(input == moves[3])
        {
            moveChar(board, "down");
        }
        else if(input == moves[4])
        {
            endMessage = "You have quit.\n";
            quit = true;
        }
        printBoard(board);
        if(won || collision || quit)
            endCondition = true;
    }
    printf("%s", endMessage);
    printf("Final score: %d\n", score);
    destroyAll(board);
    fclose(file);
    return 1;
}

