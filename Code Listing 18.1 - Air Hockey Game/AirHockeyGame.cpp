// AirHockeyGame.cpp
// By Frank Luna
// August 24, 2004.
 
#include "AirHockeyGame.h"
#include <cstdio>
#include "resource.h" // For Bitmap resource IDs

AirHockeyGame::AirHockeyGame(HINSTANCE hAppInst, 
							 HWND hMainWnd, 
		                     Vec2 wndCenterPt)
 : MAX_PUCK_SPEED(1000.0f), RED_SPEED(300.0f)
{
	// Save input parameters.
	mhAppInst    = hAppInst;
	mhMainWnd    = hMainWnd;
	mWndCenterPt = wndCenterPt;

	// Players start game with score of zero.
	mBlueScore = 0;
	mRedScore  = 0;

	// The game is initially paused.
	mPaused    = true;

	// No recovery time for red to start.
	mRedRecoverTime = 0.0f;

	// Create the sprites:

	Circle bc;
	Vec2   p0 = wndCenterPt;
	Vec2   v0(0.0f, 0.0f);

	mGameBoard = new Sprite(mhAppInst, IDB_GAMEBOARD, 
		IDB_GAMEBOARD_MASK,	bc, p0, v0);

	bc.c = p0;
	bc.r = 18.0f; // Puck radius = 18
	mPuck = new Sprite(mhAppInst, IDB_PUCK,
		IDB_PUCK_MASK, bc, p0, v0);

	p0.x = 700;
	p0.y = 200;
 
	bc.c = p0;
	bc.r = 25.0f; // Paddle radius = 25
	mRedPaddle = new Sprite(mhAppInst, IDB_REDPADDLE,
		IDB_PADDLE_MASK, bc, p0, v0);

	p0.x = 100;
	p0.y = 100;

	bc.c = p0;
	bc.r = 25.0f; // Paddle radius = 25
	mBluePaddle = new Sprite(mhAppInst, IDB_BLUEPADDLE,
		IDB_PADDLE_MASK, bc, p0, v0);

	// Initialize the rectangles.
	mBlueBounds  = Rect(7, 40, 432, 463); 
	mRedBounds   = Rect(432, 40, 854, 463);
	mBoardBounds = Rect(7, 40, 854, 463);
	mBlueGoal    = Rect(0, 146, 30, 354);
	mRedGoal     = Rect(833, 146, 863, 354);
}

AirHockeyGame::~AirHockeyGame()
{
	delete mGameBoard;
	delete mBluePaddle;
	delete mRedPaddle;
	delete mPuck;
}

void AirHockeyGame::pause()
{
	mPaused = true;

	// Game is unpaused--release capture on mouse.
	ReleaseCapture();

	// Show the mouse cursor when paused.
	ShowCursor(true); 
}

void AirHockeyGame::unpause()
{
	// Fix cursor to paddle position.
	POINT p = mBluePaddle->mPosition;
	ClientToScreen(mhMainWnd, &p);

	SetCursorPos(p.x, p.y);
	GetCursorPos(&mLastMousePos);

	mPaused = false;

	// Capture the mouse when not paused.
	SetCapture(mhMainWnd);

	// Hide the mouse cursor when not paused.
	ShowCursor(false); 
}

void AirHockeyGame::update(float dt)
{
	// Only update the game if the game is not paused.
	if( !mPaused )
	{
		updateBluePaddle(dt);
		updateRedPaddle(dt);
		updatePuck(dt);

		// Decrease recovery time as time passes.
		if( mRedRecoverTime > 0.0f )
			mRedRecoverTime -= dt;
	}
}

void AirHockeyGame::draw(HDC hBackBufferDC, HDC hSpriteDC)
{
	// Draw the sprites.
	mGameBoard->draw(hBackBufferDC, hSpriteDC);
	mBluePaddle->draw(hBackBufferDC, hSpriteDC);
	mRedPaddle->draw(hBackBufferDC, hSpriteDC);
	mPuck->draw(hBackBufferDC, hSpriteDC);


	// Draw the player scores.
	char score[32];
	sprintf(score, "Blue's score = %d", mBlueScore);

	SetBkMode(hBackBufferDC, TRANSPARENT);
	TextOut(hBackBufferDC, 15, 45, score, (int)strlen(score));

	sprintf(score, "Red's score = %d", mRedScore);
	TextOut(hBackBufferDC, 740, 45, score, (int)strlen(score));
}

void AirHockeyGame::updateBluePaddle(float dt)
{
	GetCursorPos(&mCurrMousePos);

	// Change in mouse position.
	int dx = mCurrMousePos.x - mLastMousePos.x;
	int dy = mCurrMousePos.y - mLastMousePos.y;

	Vec2 dp((float)dx, (float)dy);

	// Velocity is change in position with respect to time.
	mBluePaddle->mVelocity = dp / dt;

	// Update the blue paddle's position.
	mBluePaddle->update(dt);

	// Make sure the blue paddle stays inbounds.
	mBlueBounds.forceInside(mBluePaddle->mBoundingCircle);
	mBluePaddle->mPosition = mBluePaddle->mBoundingCircle.c;

	// The current position is now the last mouse position.
	mLastMousePos = mBluePaddle->mPosition;

	// Keep mouse cursor fixed to paddle.
	ClientToScreen(mhMainWnd, &mLastMousePos);
	SetCursorPos(mLastMousePos.x, mLastMousePos.y);
}

void AirHockeyGame::updateRedPaddle(float dt)
{
	// The red paddle's AI is overly simplistic: When the
	// puck moves into red's boundary, the red paddle
	// simply moves directly towards the puck to hit it.
	// When the puck leaves red's boundaries, the red 
	// paddle returns to the center of its boundary.

	if( mRedRecoverTime <= 0.0f )
	{
		// Is the puck in red's boundary?  If yes, then
		// move the red paddle directly toward the puck.
		if( mRedBounds.isPtInside(mPuck->mPosition) )
		{
			// Vector directed from paddle to puck.
			Vec2 redVel = mPuck->mPosition - mRedPaddle->mPosition;

			redVel.normalize();
			redVel *= RED_SPEED;
			mRedPaddle->mVelocity = redVel;
		}
		// If no, then move the red paddle to the point (700, 200).
		else
		{
			Vec2 redVel = Vec2(700, 200) - mRedPaddle->mPosition;
			if(redVel.length() > 5.0f)
			{
				redVel.normalize();
				redVel *= RED_SPEED;
				mRedPaddle->mVelocity = redVel;
			}
			// Within 5 units--close enough.
			else
				mRedPaddle->mVelocity = Vec2(0.0f, 0.0f);
		}

		// Update the red paddle's position.
		mRedPaddle->update(dt);

		// Make sure the red paddle stays inbounds.
		mRedBounds.forceInside(mRedPaddle->mBoundingCircle);
		mRedPaddle->mPosition = mRedPaddle->mBoundingCircle.c;
	}
}

void AirHockeyGame::updatePuck(float dt)
{
	paddlePuckCollision(mBluePaddle);

	// If red hits the puck then make a small 10th of a second
	// delay before the red paddle can move away as sort of a
	// "recovery period" after the hit.  This is to model the
	// fact that when a human player hits something, it takes
	// a short period of time to recover from the collision.  
	if(paddlePuckCollision(mRedPaddle))
		mRedRecoverTime = 0.1f;

	// Clamp puck speed to some maximum velocity; this provides
	// good stability.
	if( mPuck->mVelocity.length() >= MAX_PUCK_SPEED )
		mPuck->mVelocity.normalize() *= MAX_PUCK_SPEED;

	// Did the puck hit a wall?  If so, reflect about edge.  
	Circle puckCircle = mPuck->mBoundingCircle;
	if( puckCircle.c.x - puckCircle.r < mBoardBounds.minPt.x )
		mPuck->mVelocity.x *= -1.0f;
	if( puckCircle.c.x + puckCircle.r > mBoardBounds.maxPt.x )
		mPuck->mVelocity.x *= -1.0f;
	if( puckCircle.c.y - puckCircle.r < mBoardBounds.minPt.y )
		mPuck->mVelocity.y *= -1.0f;
	if( puckCircle.c.y + puckCircle.r > mBoardBounds.maxPt.y )
		mPuck->mVelocity.y *= -1.0f;

	// Make sure puck stays inbounds of the gameboard.
	mBoardBounds.forceInside(mPuck->mBoundingCircle);
	mPuck->mPosition = mPuck->mBoundingCircle.c;

	mPuck->update(dt);

	if( mBlueGoal.isPtInside(mPuck->mPosition) )
		increaseScore(false);

	if( mRedGoal.isPtInside(mPuck->mPosition) )
		increaseScore(true);	
}

bool AirHockeyGame::paddlePuckCollision(Sprite* paddle)
{
	Vec2 normal;
	if(paddle->mBoundingCircle.hits(mPuck->mBoundingCircle, normal))
	{
		// Hit updates cirle's position.  So update pucks position as well
		// since the two correspond.
		mPuck->mPosition = mPuck->mBoundingCircle.c;

		// Compute the paddle's velocity relative to the puck's
		// velocity.
		Vec2 relVel = paddle->mVelocity - mPuck->mVelocity;

		// Get the component of the relative velocity along the normal.
		float impulseMag = relVel.dot(normal);

		// Are the objects getting closer together?
		if( impulseMag >= 0.0f )
		{
			// Project the relative velocity onto the normal.
			Vec2 impulse = impulseMag * normal; 

			// Add the impulse to the puck.
			mPuck->mVelocity += 2.0f * impulse;

			return true;
		}
	}
	return false;
}

void AirHockeyGame::increaseScore(bool blue)
{
	if( blue )
		++mBlueScore;
	else
		++mRedScore;

	// A point was just scored, so reset puck to center and pause game.
	mPuck->mPosition = Vec2(mWndCenterPt.x, mWndCenterPt.y);
	mPuck->mVelocity = Vec2(0.0f, 0.0f);
	mPuck->mBoundingCircle.c = Vec2(mWndCenterPt.x, mWndCenterPt.y);

	// After score, pause the game so player can prepare for 
	// next round.
	pause();
}

