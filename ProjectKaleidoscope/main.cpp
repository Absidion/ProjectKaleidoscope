#include <string>
#include <iostream>

#include "Shader.h"

#define GL3_PROTOTYPES 1
#include <SDL.h>
#include <glew.h>


#define PROGRAM_NAME "Project Kaleidoscope"
std::string programName = "My Game";

SDL_Window* mainWindow = NULL;					//SDL Window
SDL_GLContext mainContext;						//OpenGL Context Handle


const uint32_t points = 4;						//Our Object has 4 points
	
const uint32_t floatsPerPoint = 3;				//Our point has three values (x, y, z)

const uint32_t floatsPerColor = 4;				//Each color has 4 values (red, green, blue, alpha)

//To create a VBO we need some data.
//For a Square, we need four positions, and we can use an array to store the positions. 
//Each position below is in 3D space. (x, y, z)
const GLfloat square[points][floatsPerPoint] =
{
	{ -0.5,  0.5, 0.5 },	//Top Left
	{  0.5,  0.5, 0.5 },	//Top right
	{  0.5, -0.5, 0.5 },	//Bottom right
	{ -0.5, -0.5, 0.5 }		//Bottom Left
};

const GLfloat colors[points][floatsPerColor] =
{
	{ 0.0, 1.0, 0.0, 1.0 }, // Top left
	{ 1.0, 1.0, 0.0, 1.0 }, // Top right
	{ 1.0, 0.0, 0.0, 1.0 }, // Bottom right 
	{ 0.0, 0.0, 1.0, 1.0 }, // Bottom left
};

GLuint vbo[2], vao[1];

const uint32_t positionAttributeIndex = 0, colorAttributeIndex = 1;

bool SetOpenGLAttributes();
void PrintSDL_GL_Attributes();
void CheckSDLError(int line);

// Our wrapper to simplify the shader code
Shader myShader;

void RunGame();
void Cleanup();

void Render()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//Invoke glDrawArrays telling that our data is a line loop and we want to draw 2-4 vertices
	glDrawArrays(GL_LINE_LOOP, 0, 4);

	//Swap our buffers to make our changes visible
	SDL_GL_SwapWindow(mainWindow);

	std::cout << "Press Enter to render next frame\n";
	std::cin.ignore();

	//Enable our attribute within the current VAO.
	glEnableVertexAttribArray(colorAttributeIndex);

	// Make our background black
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Invoke glDrawArrays telling that our data is a line loop and we want to draw 2-4 vertexes
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Swap our buffers to make our changes visible
	SDL_GL_SwapWindow(mainWindow);
}

bool SetupBufferObjects()
{
	glGenBuffers(2, vbo);	//This function generates a VBO for us, that we can store vertex attribute data into it. It returns an ID that we can use to refer to the VBO later.
	
	glGenVertexArrays(1, vao); //This function generates a VAO for us, so that we can store data for multiple Vertex Attributes.

	glBindVertexArray(vao[0]); //Similar to glBindBuffer. This function sets the VAO as the active one.

	//Bind our first VBO as being the active buffer and storing vertex attributes (positions/coordinates)
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);		//Sets a Buffer as the current buffer. Using this wrong can cause the application to crash.

	uint32_t sizeInBytes = (points * floatsPerPoint) * sizeof(GLfloat);

	glBufferData(GL_ARRAY_BUFFER, sizeInBytes, square, GL_STATIC_DRAW); //This tells opengl what data we want to store and how we want to use it.

	//Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex.
	//GLBufferData() and GLBindVertexArray() should be called before this, otherwise opengl does not know which VAO and VBO to use.
	glVertexAttribPointer(positionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Enable our attribute within the current VAO
	glEnableVertexAttribArray(positionAttributeIndex);

	// Colors
	// =======================
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	// Copy the vertex data from diamond to our buffer
	glBufferData(GL_ARRAY_BUFFER, (points * floatsPerColor) * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	// Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex
	glVertexAttribPointer(colorAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);

	if (!myShader.Init())
		return false;

	myShader.UseProgram();

	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

bool Init()
{
	//Initialize SDL's Video subsystem
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "SDL could not init. SDL Error: " << SDL_GetError() << std::endl;
		return 0;
	}

	//Create our windows centered at 512x512 Resolution
	//Tells SDL2 that we are using the window for OpenGL
	mainWindow = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL);
	

	//Check that mainWindow is not null
	if (!mainWindow)
	{
		std::cout << "Unable to create window\n";
		CheckSDLError(__LINE__);
		return false;
	}

	//Create our opengl context and attach it to our window.
	mainContext = SDL_GL_CreateContext(mainWindow);

	SetOpenGLAttributes();

	//This makes our buffer swap synchroized with the monitor's vertical refresh.
	SDL_GL_SetSwapInterval(1);

	// Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	return true;
}

bool SetOpenGLAttributes()
{
	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	PrintSDL_GL_Attributes();

	return true;
}

void Cleanup()
{
	//Cleanup all the things we found and allocated
	myShader.CleanUp();

	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, vao);

	// Delete our OpengL context
	SDL_GL_DeleteContext(mainContext);

	// Destroy our window
	SDL_DestroyWindow(mainWindow);

	// Shutdown SDL 2
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	if (!Init())
		return -1;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(mainWindow);

	std::cout << "Setting up VBO + VAO..." << std::endl;
	if (!SetupBufferObjects())
		return -1;

	std::cout << "Rendering..." << std::endl;
	Render();

	std::cout << "Rendering done!\n";
	std::cin.ignore();

	//Game Loop
	//RunGame();

	//Only called when our game loop has ended
	Cleanup();

	return 0;
}

void RunGame()
{
	bool loop = true;

	while (loop)
	{
		//Using SDL's Event system, we can get keyboard inputs.
		//Using a switch statement within a while loop we can keep the gameloop going unless we choose to break out of it
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				loop = false;

			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym) 
				{
				case SDLK_ESCAPE:
					loop = false;
					break;
				case SDLK_r:
					// Cover with red and update
					glClearColor(1.0, 0.0, 0.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
					SDL_GL_SwapWindow(mainWindow);
					break;
				case SDLK_g:
					// Cover with green and update
					glClearColor(0.0, 1.0, 0.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
					SDL_GL_SwapWindow(mainWindow);
					break;
				case SDLK_b:
					// Cover with blue and update
					glClearColor(0.0, 0.0, 1.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
					SDL_GL_SwapWindow(mainWindow);
					break;
				default:
					break;
				}
			}
		}
	}
}




void CheckSDLError(int line = -1)
{
	std::string error = SDL_GetError();

	if (error != "")
	{
		std::cout << "SLD Error : " << error << std::endl;

		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;

		SDL_ClearError();
	}
}

//This function is used to just print our Major and Minor versions to the command window.
void PrintSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}
