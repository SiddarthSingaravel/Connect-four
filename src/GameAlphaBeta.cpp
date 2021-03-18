#include <iostream>
#include <vector>
#include "Game.h"
#include <chrono>
#include <omp.h>

using namespace std;


Game::Game()
{
	Board board;
	this->currentPlayer = 1;
	this->gameOver = false;
}

void Game::start()
{
	this->loop();
}

void Game::togglePlayer()
{
	this->currentPlayer = this->currentPlayer == 1 ? 2 : 1;
}

void Game::loop()
{
	while (!this->gameOver)
	{
		cout << "Player " << this->currentPlayer << ": ";

		if (this->currentPlayer == 1)
		{
			//human
			int move;
			cin >> move;
			this->gameOver = board.makeMove(move, this->currentPlayer);
		}
		if (this->currentPlayer == 2)
		{
			// ai
			int move = this->findBestMove();
			cout << "best ai move: " << move << endl;
			this->gameOver = board.makeMove(move, this->currentPlayer);
		}

		board.showBoard();

		if (this->gameOver)
		{
			winner = this->currentPlayer;
		}
		else
		{
			this->togglePlayer();
		}
	}
	cout << "Player " << winner << " won" << endl;
}

int Game::findBestMove()
{
	int bestCol = 0;
	int bestScore = INT_MIN;
	int maxDepth = 6;
	int player = 2; // AI id
	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < board.getCols(); i++)
	{
		// can we play in this column ?
		if (board.isColumnEmpty(i))
		{
			// make move in the col
			board.makeMove(i, player);
			// get score
			int score = this->alphabetapruning(1,maxDepth,INT_MIN,INT_MAX,false,player);//this->minimax(1, maxDepth, false, player);
			// update score
			if (score > bestScore)
			{
				bestScore = score;
				bestCol = i;
			}
			// undo move
			board.undoMove();
		}
	}
	auto end = chrono::high_resolution_clock::now();
	auto duration  = chrono::duration_cast<chrono::milliseconds>(end-start);
	cout<<"\nTime to calculate best move: "<< duration.count() <<endl;
	return bestCol;
}

int Game::alphabetapruning(int depth, int maxDepth, int alpha, int beta, bool isMaximizingPlayer, int player)
{
	if (depth == maxDepth)
	{
		return board.evaluateBoard(player);
	}

	int nextPlayer = player == 1 ? 2 : 1;
	#pragma omp parallel for num_threads(4) if(isMaximizingPlayer)
	if (isMaximizingPlayer)
	{
		int bestScore = INT_MIN;
		for (int i = 0; i < board.getCols(); i++)
		{
			// can we play in this column ?
			if (board.isColumnEmpty(i))
			{
				// simulate move in each column
				board.makeMove(i, player);
				// get score
				int score = alphabetapruning(depth + 1, maxDepth, alpha, beta, false, nextPlayer);
				// update score
				bestScore = max(bestScore, score);
				//updating alpha
				alpha=max(alpha,score);
				// undo move
				board.undoMove();
				//pruning
				if(beta<=alpha)
					break;
				// cout << "max player " << score << " " << bestScore << endl;
			}
		}
		return bestScore;
	}
	else
	{
		int bestScore = INT_MAX;
		#pragma omp parallel for num_threads(2) 
		for (int i = 0; i < board.getCols(); i++)
		{
			// can we play in this column ?
			if (board.isColumnEmpty(i))
			{
				// simulate move in each column
				board.makeMove(i, player);
				// get score
				int score = this->alphabetapruning(depth + 1, maxDepth,alpha,beta,true, nextPlayer);
				// update score
				bestScore = min(bestScore, score);
				//updating beta
				beta=min(beta,score);
				// undo move
				board.undoMove();
				//pruning
				if(beta<=alpha)
					break;
				// cout << "min player " << score << " " << bestScore << endl;
			}
		}
		return bestScore;
	}
}



int main()
{
	Game g;
	g.start();
	return 0;
}
