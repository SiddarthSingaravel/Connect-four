#include <iostream>
#include <vector>
#include "Board.h"

using namespace std;

Board::Board()
{
    board.resize(this->ROWS, vector<int>(this->COLS, 0));
}

int Board::getCols() { return this->COLS; }

int Board::getRows() { return this->ROWS; }

bool Board::isColumnEmpty(int col) { return this->board[0][col] == 0; }

void Board::showBoard()
{
    for (int i = 0; i < this->ROWS; i++)
    {
        for (int j = 0; j < this->COLS; j++)
        {
            int cell = board[i][j];
            switch (cell)
            {
            case 0:
                cout << "\x1B[90m" << cell << "\033[0m ";
                break;
            case 1:
                cout << "\x1B[31m" << cell << "\033[0m ";
                break;
            case 2:
                cout << "\x1B[34m" << cell << "\033[0m ";
                break;
            }
        }
        cout << endl;
    }
}

bool Board::isCellWithinBoundaries(int row, int col)
{
    return row >= 0 && row < this->ROWS && col >= 0 && col < this->COLS;
}

bool Board::isPlayerCell(pair<int, int> cell, int player)
{
    return this->isCellWithinBoundaries(cell.first, cell.second) &&
           this->board[cell.first][cell.second] == player;
}

int Board::makeMove(int col, int player)
{
    int row = 0;
    // check row, col, player, cell occupancy
    if (col < 0 && col >= this->COLS)
    {
        cout << "Col out of bounds" << endl;
    }
    else if (player != 1 && player != 2)
    {
        cout << "Invalid player" << endl;
    }
    else if (board[0][col])
    {
        cout << "Column is already full" << endl;
    }
    else
    {
        while (row < this->ROWS && !board[row][col])
        {
            row++;
        }
        row--;
        board[row][col] = player;
    }
    // append to moves list
    this->moves.push_back(pair<int, int>(row, col));
    // return win ?
    return this->checkWin(row, col, player) ? player : 0;
}

void Board::undoMove()
{
    // get latest move
    pair<int, int> lastMove = this->moves.back();
    int row = lastMove.first;
    int col = lastMove.second;
    // set cell to empty
    board[row][col] = 0;
    // pop last move
    this->moves.pop_back();
}

int Board::checkSlice(int row, int col, pair<int, int> incr)
{
    // check for contiguous block of 4 from cell(row, col) in slice
    int count = 0;
    int prev = board[row][col];
    // next cell
    row += incr.first;
    col += incr.second;
    while (isCellWithinBoundaries(row, col))
    {
        if (board[row][col] == prev)
        {
            count++;
            prev = board[row][col];
            // next cell
            row += incr.first;
            col += incr.second;
        }
        else
        {
            break;
        }
    }
    return count;
}

bool Board::checkColumnWin(int row, int col, int player)
{
    int colTotal = checkSlice(row, col, pair<int, int>(1, 0)) + 1;
    return colTotal == 4;
}

bool Board::checkRowWin(int row, int col, int player)
{
    // Eg: OXX(X)OXX -> not a win
    // Eg: OOX(X)XXO -> win
    int left = checkSlice(row, col, pair<int, int>(0, -1));
    int right = checkSlice(row, col, pair<int, int>(0, 1));
    // win condition
    return left + 1 + right >= 4;
}

bool Board::checkDiagonalsWin(int row, int col, int player) {
    // diag1
    int topRight = checkSlice(row, col, pair<int, int>(-1, 1));
    int bottomLeft = checkSlice(row, col, pair<int, int>(1, -1));
    // diag2
    int topLeft = checkSlice(row, col, pair<int, int>(-1, -1));
    int bottomRight = checkSlice(row, col, pair<int, int>(1, 1));
    // no of consec player coins in diagonals
    int diag1 = bottomLeft + 1 + topRight;
    int diag2 = topLeft + 1 + bottomRight;
    // win condition
    return diag1 >= 4 || diag2 >= 4;
}



bool Board::checkWin(int row, int col, int player)
{
    return this->checkColumnWin(row, col, player) ||
           this->checkRowWin(row, col, player) ||
           this->checkDiagonalsWin(row, col, player);
}


int Board::countWindow(vector<int> window,int piece)
{
    int count=0;
    int len=window.size();
    for(int i=0;i<len;i++)
    {
        if(window[i]==piece)
        {
            count++;
        }
    }
    return count;
}


int Board::evaluateWindow(vector<int> window,int player)
{
    int score=0;
    int opp = 1;
    if(player==1)
    {
        opp=2;
    }
    
    if(countWindow(window,player)==4)
    {
        score+=100;
    }
    else if(countWindow(window,player)==3 && countWindow(window,0)==1)
    {
        score+=5;
    }
    else if(countWindow(window,player)==2 && countWindow(window,0)==2)
    {
        score+=2;
    }

    if(countWindow(window,opp)==3 && countWindow(window,0)==1)
    {
        score-=4;
    }

    return score;
    
}


int Board::evaluateBoard(int player)
{
    int score=0;
    int i,j,c;
    vector<int> window;

    //Horizontal score calculation
    #pragma omp parallel for num_threads(3) collapse(2)
    for(i=0;i<this->ROWS;i++)
    {        
        for(j=0;j<this->COLS-3;j++)
        {
            window.push_back(board[i][j]);
            window.push_back(board[i][j+1]);
            window.push_back(board[i][j+2]);
            window.push_back(board[i][j+3]);
            score+=evaluateWindow(window,player);
            window.clear();
        }
    }

    //Vertcal score calculation
    #pragma omp parallel for num_threads(3) collapse(2)
    for(j=0;j<this->COLS;j++)
    {
        for(i=0;i<this->ROWS-3;i++)
        {
            window.push_back(board[i][j]);
            window.push_back(board[i+1][j]);
            window.push_back(board[i+2][j]);
            window.push_back(board[i+3][j]);
            score+=evaluateWindow(window,player);
            window.clear();
        }
    }

    //Positive Diagonal score calculation
    #pragma omp parallel for num_threads(3) collapse(2)
    for(i=0;i<this->ROWS-3;i++)
    {
        for(j=0;j<this->COLS-3;j++)
        {
            window.push_back(board[i][j]);
            window.push_back(board[i+1][j+1]);
            window.push_back(board[i+2][j+2]);
            window.push_back(board[i+3][j+3]);
            score+=evaluateWindow(window,player);
            window.clear();
        }
    }

    //Negative Diagonal score calculation
    #pragma omp parallel for num_threads(3) collapse(2)
    for(i=0;i<this->ROWS-3;i++)
    {
        for(j=0;j<this->COLS-3;j++)
        {
            window.push_back(board[i+3][j]);
            window.push_back(board[i+2][j+1]);
            window.push_back(board[i+1][j+2]);
            window.push_back(board[i][j+3]);
            score+=evaluateWindow(window,player);
            window.clear();
        }
    }

    //Center preference
    vector<int> center;
    j=this->COLS/2;
    for(i=0;i<this->ROWS;i++)
    {
        center.push_back(board[i][j]);
    }
    score+=(countWindow(center,player)*3);
    return score;

}