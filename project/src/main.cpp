// External includes
#include "SDL.h"
#include "SDL_surface.h"
#undef main

// Standard includes
#include <iostream>
#include <memory>

// Project includes
#include "Renderer.h"
#include "Timer.h"
#include "Scene.h"

#if defined( _DEBUG )
#	include "LeakDetector.h"
#endif

using namespace dae;

void ShutDown( SDL_Window* pWindow )
{
	SDL_DestroyWindow( pWindow );
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	// Unreferenced parameters
	(void)argc;
	(void)args;

// Leak detection
#if defined( _DEBUG )
	LeakDetector detector{};
#endif

	// Create window + surfaces
	SDL_Init( SDL_INIT_VIDEO );

	const uint32_t width{ 640 };
	const uint32_t height{ 480 };

	/*
	const uint32_t width{ 1920 };
	const uint32_t height{ 1080 };
	*/

	SDL_Window* pWindow = SDL_CreateWindow(
		"Rasterizer - Luna Pype", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0 );

	if ( !pWindow )
		return 1;

	// Initialize "framework"
	Timer timer{};
	Renderer renderer{ pWindow };

	/*
	Why was this on the heap????????
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer( pWindow );
	*/

	// Initialize scene
	auto upScene{ std::make_unique<SceneW5>() };
	upScene->Initialize();

	// Start loop
	timer.Start();

	// Start Benchmark
	// TODO pTimer->StartBenchmark();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while ( isLooping )
	{
		//--------- Get input events ---------
		SDL_Event e;
		while ( SDL_PollEvent( &e ) )
		{
			switch ( e.type )
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if ( e.key.keysym.scancode == SDL_SCANCODE_X )
					takeScreenshot = true;
				break;
			}
		}

		//--------- Update ---------
		upScene->Update( &timer );
		renderer.Update( &timer );

		//--------- Render ---------
		renderer.Render( upScene.get() );

		//--------- Timer ---------
		timer.Update();
		printTimer += timer.GetElapsed();
		if ( printTimer >= 1.f )
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << timer.GetdFPS() << std::endl;
		}

		// Save screenshot after full render
		if ( takeScreenshot )
		{
			if ( !renderer.SaveBufferToImage() )
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	timer.Stop();

	ShutDown( pWindow );
	return 0;
}
