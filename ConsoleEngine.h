#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

class ConsoleWindow {
public:
	
};

enum Color {
	DEF = 7,
	GREY = 8,
	BLUE = 9,
	LIME = 10,
	CYAN = 11,
	RED = 12,
	PINK = 13,
	YELLOW = 14,
	WHITE = 15,
};

enum Pixel {
	SOLID = 0x2588,
	THREE = 0x2593,
	HALF = 0x2592,
	TRANS = 0x2591,
};

class ConsoleEngine {
public:
	int screenWidth;
	int screenHeight;
	int screenArea;

	int bufferWidth;
	int bufferHeight;
	int bufferSize;

	std::string title;
	std::thread gameThread;;

	class Shape{};

private:
	HANDLE hConsole;
	CHAR_INFO *screenBuffer;
	SMALL_RECT dwBytesWritten;
	bool isRunning = false;


public:

	ConsoleEngine(int width, int height, std::string title) {

		screenWidth = width;
		screenHeight = height;
		this->title = title;
		screenArea = width * height;

		InitializeNewConsole();

		title = title + " " + std::to_string(bufferWidth) + " x " + std::to_string(bufferHeight);

		SetWindowTitle(title);
		toggleConsoleCursor(false);
	}

	void StartGameLoop()
	{
		gameThread = std::thread(&ConsoleEngine::applicationThread, this);
		gameThread.join();
	}

	virtual void Start() = 0;
	virtual void Update(float timeSinceLastFrame) = 0;


	void repaintBuffer() {
		WriteConsoleOutput(hConsole, screenBuffer, {(short)bufferWidth, (short)bufferHeight}, { 0, 0 }, &dwBytesWritten);
	}


	void drawChar(int x, int y, char c, int color = Color::DEF)
	{
		int i = coordinateToIndex(x, y, bufferWidth);
		screenBuffer[i].Char.UnicodeChar = c;
		screenBuffer[i].Attributes = color;
	}


	void drawChar(int index, char c, int color = Color::DEF)
	{
		screenBuffer[index].Char.UnicodeChar = c;
	}

	void drawChar(int index, int c, int color = Color::DEF)
	{
		screenBuffer[index].Char.UnicodeChar = c;
	}


	void drawChar(int x, int y, int c, int color = Color::DEF)
	{
		int i = coordinateToIndex(x, y, bufferWidth);
		screenBuffer[i].Char.UnicodeChar = c;
		screenBuffer[i].Attributes = color;
	}


	void drawString(int x, int y, std::string str, int color = Color::DEF)
	{
		int index = coordinateToIndex(x, y, bufferWidth);
		int len = str.size();
		int j = 0;
		for (int i = index; i <= index + (len-1); i++)
		{
			screenBuffer[i].Char.UnicodeChar = str[j];
			screenBuffer[i].Attributes = color;
			j++;
		}
		
	}

	void drawRect(int x, int y, int w, int h, int c, int color = Color::DEF)
	{
		for (int i = x; i < x+w; i++)
		{
			for (int j = y; j < y+h; j++)
			{
				drawChar(i, j, c, color);
			}
		}
	}


	void clearScreen()
	{
		for (int i = 0; i < bufferSize; i++)
		{
			screenBuffer[i].Char.UnicodeChar = ' ';
			screenBuffer[i].Attributes = Color::DEF;
		}
		repaintBuffer();
	}


private:
	void InitializeNewConsole()
	{

		// Sets the console window size to the screenWidth and screenHeigh set in constructor
		RECT r;
		HWND hwConsole = GetConsoleWindow();
		GetWindowRect(hwConsole, &r);
		MoveWindow(hwConsole, r.left, r.top, screenWidth, screenHeight, TRUE);

		// Creates reference to new console screen buffer and sets it as active
		hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(hConsole);
		
		// Gets reference to console screen buffer size (seperate from window size) and creates a buffer to hold characters
		_CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		bufferSize = csbi.dwSize.X * csbi.dwSize.Y;
		bufferWidth = csbi.dwSize.X;
		bufferHeight = csbi.dwSize.Y;

		dwBytesWritten = { 0, 0, csbi.dwSize.X, csbi.dwSize.Y };
		SetConsoleWindowInfo(hConsole, TRUE, &dwBytesWritten);

		screenBuffer = new CHAR_INFO[bufferSize];

		// Initalise the screen buffer to all spaces
		int i = 0;
		for (i = 0; i < bufferSize; i++)
		{
			screenBuffer[i].Char.UnicodeChar = ' ';
			screenBuffer[i].Attributes = Color::DEF;
		}

		// Write the inital buffer to the console
		WriteConsoleOutput(hConsole, screenBuffer, { (short)bufferWidth, (short)bufferHeight }, { 0, 0 }, &dwBytesWritten);
	}

	void applicationThread()
	{
		Start();
		isRunning = true;

		auto t1 = std::chrono::system_clock::now();
		auto t2 = std::chrono::system_clock::now();

		while (isRunning) {

			t2 = std::chrono::system_clock::now();
			std::chrono::duration<float> elapsed = t2 - t1;
			t1 = t2;

			float timeSinceLastFrame = elapsed.count();

			Update(timeSinceLastFrame);
			
		}
	}

public:
	void toggleConsoleCursor(bool flag)
	{
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(hConsole, &cursorInfo);
		cursorInfo.bVisible = flag;
		SetConsoleCursorInfo(hConsole, &cursorInfo);
	}

	void SetWindowTitle(std::string Title)
	{
		std::wstring stemp = std::wstring(Title.begin(), Title.end());
		LPCWSTR lpstr = stemp.c_str();
		SetConsoleTitle(lpstr);
	}

	int coordinateToIndex(int x, int y, int width)
	{
		return (y * width) + x;
	}

	void indexToCoordinate(int i, int *x, int *y, int width) {
		*x = i % width;
		*y = i / width;
	}

	int getBufferSize() { int r = bufferSize; return r; }
};
