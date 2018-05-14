// ship.cpp
// By Frank Luna
// August 24, 2004.

//=========================================================
// Includes
//=========================================================
#include <string>
#include "resource.h"
#include "Sprite.h"
#include "BackBuffer.h"
#include <list>
using namespace std;

//=========================================================
// Globals
//=========================================================
HWND        ghMainWnd  = 0;
HINSTANCE   ghAppInst  = 0;
HMENU       ghMainMenu = 0;
HDC         ghSpriteDC = 0;

BackBuffer* gBackBuffer = 0;
Sprite*     gBackground = 0;
Sprite*     gF15        = 0;
Sprite*     gF18        = 0;
Sprite*     gF117       = 0;
Sprite*     gBullet     = 0;

list<Vec2> gBulletPos;
RECT gMapRect = {0, 0, 800, 600};

string gWndCaption = "Game Institute Ship Sample";

// Client dimensions exactly equal dimensions of 
// background bitmap.  This is found by inspecting 
// the bitmap in an image editor, for example.
const int gClientWidth  = 800;
const int gClientHeight = 600;

// Center point of client rectangle.
const POINT gClientCenter = 
{
	gClientWidth  / 2,
	gClientHeight / 2
};

// Pad window dimensions so that there is room for window
// borders, caption bar, and menu.
const int gWindowWidth  = gClientWidth  + 6;
const int gWindowHeight = gClientHeight + 52;

//=========================================================
// Function Prototypes
//=========================================================

bool InitMainWindow();
int  Run();
void DrawFramesPerSecond(float deltaTime);

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//=========================================================
// Name: WinMain
// Desc: Program execution starts here.
//=========================================================

int WINAPI 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		PSTR cmdLine, int showCmd)
{
	ghAppInst = hInstance;

	// Create the main window.
	if( !InitMainWindow() )
	{
		MessageBox(0, "Window Creation Failed.", "Error", MB_OK);
		return 0;
	}

	// Enter the message loop.
	return Run();
}

//=========================================================
// Name: InitMainWindow
// Desc: Creates the main window upon which we will
//       draw the game graphics onto.  
//=========================================================
bool InitMainWindow()
{
	WNDCLASS wc; 
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = ghAppInst;
	wc.hIcon         = ::LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = ::LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = "MyWndClassName";

	RegisterClass( &wc );

	// WS_OVERLAPPED | WS_SYSMENU: Window cannot be resized
	// and does not have a min/max button.  
	ghMainMenu = LoadMenu(ghAppInst, MAKEINTRESOURCE(IDR_MENU));
	ghMainWnd = ::CreateWindow("MyWndClassName", 
		gWndCaption.c_str(), WS_OVERLAPPED | WS_SYSMENU, 
		200, 200, gWindowWidth, gWindowHeight, 0, 
		ghMainMenu, ghAppInst, 0);

	if(ghMainWnd == 0)
	{
		::MessageBox(0, "CreateWindow - Failed", 0, 0);
		return 0;
	}

	ShowWindow(ghMainWnd, SW_NORMAL);
	UpdateWindow(ghMainWnd);

	return true;
}

//=========================================================
// Name: Run
// Desc: Encapsulates the message loop.
//=========================================================
int Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// Get the performance timer frequency.
	__int64 cntsPerSec = 0;
	bool perfExists = QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec)!=0;
	if( !perfExists )
	{
		MessageBox(0, "Performance timer does not exist!", 0, 0);
		return 0;
	}

	double timeScale = 1.0 / (double)cntsPerSec;
	// Get the current time.
	__int64 lastTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&lastTime);
	
	double timeElapsed = 0.0f;

	while(msg.message != WM_QUIT)
	{
		// IF there is a Windows message then process it.
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// ELSE, do game stuff.
		else
        {	
			// Get the time now.
			
			__int64 currTime = 0; 
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			// Compute the differences in time from the last
			// time we checked.  Since the last time we checked
			// was the previous loop iteration, this difference
			// gives us the time between loop iterations...
			// or, I.e., the time between frames.
			double deltaTime = (double)(currTime - lastTime) * timeScale;

			timeElapsed += deltaTime;

			// Clamp speed to 100 units per second.
			if(gF15->mVelocity.length() > 100.0f)
				gF15->mVelocity.normalize() *= 100.0f;

			// Update ship.
			gF15->update((float)deltaTime);

			// Make sure F15 stays in the map boundary.
			if( gF15->mPosition.x < gMapRect.left )
			{
				gF15->mPosition.x = (float)gMapRect.left;
				gF15->mVelocity.x = 0.0f;
				gF15->mVelocity.y = 0.0f;
			}
			if( gF15->mPosition.x > gMapRect.right )
			{
				gF15->mPosition.x = (float)gMapRect.right;
				gF15->mVelocity.x = 0.0f;
				gF15->mVelocity.y = 0.0f;
			}
			if( gF15->mPosition.y < gMapRect.top )
			{
				gF15->mPosition.y = (float)gMapRect.top;
				gF15->mVelocity.x = 0.0f;
				gF15->mVelocity.y = 0.0f;
			}
			if( gF15->mPosition.y > gMapRect.bottom )
			{
				gF15->mPosition.y = (float)gMapRect.bottom;
				gF15->mVelocity.x = 0.0f;
				gF15->mVelocity.y = 0.0f;
			}

			// Draw objects.
			gBackground->draw(gBackBuffer->getDC(), ghSpriteDC);
			gF15->draw(gBackBuffer->getDC(), ghSpriteDC);
			gF18->draw(gBackBuffer->getDC(), ghSpriteDC);
			gF117->draw(gBackBuffer->getDC(), ghSpriteDC);

			list<Vec2>::iterator i = gBulletPos.begin();
			while( i != gBulletPos.end() )
			{
				Vec2 bulletVelocity(0.0f, -300.0f);

				// Update the position.
				*i += bulletVelocity * (float)deltaTime;

				POINT p = *i;

				// Only draw bullet if it is still inside the map boundaries.
				// Otherwise, delete it.
				if( !PtInRect(&gMapRect, p) )
					i = gBulletPos.erase(i);
				else
				{
					gBullet->mPosition = *i;
					gBullet->draw(gBackBuffer->getDC(), ghSpriteDC);
					++i; // Next in list.
				}
			}

			DrawFramesPerSecond((float)deltaTime);

			// Now present the backbuffer contents to the main
			// window client area.
			gBackBuffer->present();
				
			// We are at the end of the loop iteration, so
			// prepare for the next loop iteration by making
			// the "current time" the "last time."
			lastTime = currTime;

			// Free 20 miliseconds to Windows so we don't hog
			// the system resources.
			Sleep(20);
        }
    }
	// Return exit code back to operating system.
	return (int)msg.wParam;
}

//=========================================================
// Name: DrawFramesPerSecond
// Desc: This function is called every frame and updates
//       the frame per second display in the main window
//       caption.
//=========================================================
void DrawFramesPerSecond(float deltaTime)
{
	// Make static so the variables persist even after
	// the function returns.
	static int   frameCnt    = 0;
	static float timeElapsed = 0.0f;
	static char  buffer[256];

	// Function called implies a new frame, so increment
	// the frame count.
	++frameCnt;

	// Also increment how much time has passed since the
	// last frame.  
	timeElapsed += deltaTime;

	// Has one second passed?
	if( timeElapsed >= 1.0f )
	{
		// Yes, so compute the frames per second.
		// FPS = frameCnt / timeElapsed, but since we
		// compute only when timeElapsed = 1.0, we can 
		// reduce to:
		// FPS = frameCnt / 1.0 = frameCnt.
		
		sprintf(buffer, "--Frames Per Second = %d", frameCnt);

		// Add the frames per second string to the main
		// window caption--that is, we'll display the frames
		// per second in the window's caption bar.
		string newCaption = gWndCaption + buffer;

		// Now set the new caption to the main window.
		SetWindowText(ghMainWnd, newCaption.c_str());
	
		// Reset the counters to prepare for the next time
		// we compute the frames per second.
		frameCnt    = 0;
		timeElapsed = 0.0f;
	}
}

//=========================================================
// Name: WndProc
// Desc: The main window procedure.
//=========================================================

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	Circle bc; // Not needed in this demo, leave default.
	Vec2   p0(gClientCenter);
	Vec2   v0(0.0f, 0.0f);

	switch( msg )
	{	
	// Create application resources.
	case WM_CREATE:

		// Create the sprites
		gBackground = new Sprite(ghAppInst,
			IDB_BACKGROUND, IDB_BACKGROUNDMASK, bc, p0, v0);

		gF15 = new Sprite(ghAppInst, IDB_F15, IDB_F15MASK,
			bc, p0, v0);

		p0.x = 100;
		p0.y = 100;
		gF18 = new Sprite(ghAppInst, IDB_F18, IDB_F18MASK,
			bc, p0, v0);

		p0.x = 600;
		p0.y = 100;
        gF117 = new Sprite(ghAppInst, IDB_F117, IDB_F117MASK,
			bc, p0, v0);

		p0.x = 0.0f;
		p0.y = 0.0f;
		gBullet = new Sprite(ghAppInst, IDB_BULLET, IDB_BULLETMASK,
			bc, p0, v0);

		// Create system memory DCs 
		ghSpriteDC = CreateCompatibleDC(0);

		// Create the backbuffer.
		gBackBuffer = new BackBuffer(
			hWnd, 
			gClientWidth,
			gClientHeight);

		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		// Destroy the window when the user selects the 'exit'
		// menu item.
		case ID_FILE_EXIT:
			DestroyWindow(ghMainWnd);
			break;
		}
		return 0;

	case WM_KEYDOWN:
		switch(wParam)
		{
		// Accelerate left.
		case 'A':
			gF15->mVelocity.x -= 5.0f;
			break;
		// Accelerate right.
		case 'D':
			gF15->mVelocity.x += 5.0f;
			break;
		// Accelerate up (remember +y goes down and -y goes up)
		case 'W':
			gF15->mVelocity.y -= 5.0f;
			break;
		// Accelerate down.
		case 'S':
			gF15->mVelocity.y += 5.0f;
			break;
		case VK_SPACE:
			// Add a bullet to the bullet list.
			gBulletPos.push_back(gF15->mPosition);
			break;
		}
		return 0;

	// Destroy application resources.
	case WM_DESTROY: 	
		delete gBackground;
		delete gF15;
		delete gF18;
		delete gF117;
		delete gBullet;
		delete gBackBuffer;
		DeleteDC(ghSpriteDC);
		PostQuitMessage(0); 
		return 0;	
	}	
	// Forward any other messages we didn't handle to the
	// default window procedure.
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
