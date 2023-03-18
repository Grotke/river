#include <ctime>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glslprogram.h"
#include <vector>
#include "utils.h"

//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		3. Debugging to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joseph Montgomery

// Window titles
constexpr char* WINDOWTITLE{ "Final Project: River Animated with Flow Tiles -- Joseph Montgomery" };
constexpr char* GLUITITLE{ "User Interface Window" };

// Shader helper class
GLSLProgram* Pattern;

// what the glui package defines as true and false:

constexpr int GLUITRUE{ true };
constexpr int GLUIFALSE{ false };

// Escape key

constexpr int ESCAPE = 0x1b;

// initial window size:

constexpr int INIT_WINDOW_SIZE{ 1200 };

// multiplication factors for input interaction:
//  (these are known from previous experience)

constexpr float ANGFACT{ 1. };
constexpr float SCLFACT{ 0.005f };

// minimum allowable scale factor:

constexpr float MINSCALE{ 0.05f };

// scroll wheel button values:

constexpr int SCROLL_WHEEL_UP{ 3 };
constexpr int SCROLL_WHEEL_DOWN{ 4 };

// equivalent mouse movement when we click a the scroll wheel:

constexpr float SCROLL_WHEEL_CLICK_FACTOR{ 5. };

// active mouse buttons (or them together):

constexpr int LEFT{ 4 };
constexpr int MIDDLE{ 2 };
constexpr int RIGHT{ 1 };

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKGROUND_COLOR[] = { 0., 0., 0., 1. };

// line width for the axes:

constexpr GLfloat AXES_WIDTH{ 3. };

// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info

int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
float Time;

// River globals
GLuint TerrainList;
GLuint TerrainTexture, WaterTexture, WaterNormalMap, FlowMap, RiverMap;
const float BLOCKS = 16.f;
int totalTerrainWidth;
int totalTerrainHeight;
bool UseTransparency;
bool AnimateWater;
bool UseEdgeTransparancy;
bool ShowWater;
bool ShinyWater;

constexpr int MS_IN_THE_ANIMATION_CYCLE = 10000;


// function prototypes:

void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectionMenu(int);
float	ElapsedSeconds();
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);

// main program:
int
main(int argc, char* argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display structures that will not change:

	InitLists();

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset();

	// setup all the user interface stuff:

	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow(MainWindow);
	glutMainLoop();

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:
	
	int ms = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
	ms %= MS_IN_THE_ANIMATION_CYCLE;
	Time = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;        // [ 0., 1. )

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// draw the complete scene:

void
Display()
{
	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 5000.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

		// set the eye position, look-at position, and up-vector:

		gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);


		// rotate the scene:

		glRotatef((GLfloat)Yrot, 0., 1., 0.);
		glRotatef((GLfloat)Xrot, 1., 0., 0.);


		// uniformly scale the scene:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
		glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		GLfloat white[3]{ 1., 1., 1. };
		glColor3fv(white);
		glCallList(AxesList);
	}

	// Activate shader and set up uniforms
	Pattern->Use();
	//uniform float uKa, uKd, uKs; // coefficients of each type of lighting
	//uniform vec3 uColor;		// object color
	//uniform vec3 uSpecularColor; // light color
	//uniform float uShininess;	// specular exponent
	Pattern->SetUniformVariable("uKa", 0.1f);
	Pattern->SetUniformVariable("uKd", 1.0f);
	Pattern->SetUniformVariable("uKs", 0.1f);
	Pattern->SetUniformVariable("uColor", 1.0, 1.0, 1.0);
	Pattern->SetUniformVariable("uSpecularColor", 1.0, 1.0, 1.0);
	Pattern->SetUniformVariable("uShininess", 1.f);
	Pattern->SetUniformVariable("uBlocks", BLOCKS);
	Pattern->SetUniformVariable("uTerrainTextureWidth", totalTerrainWidth);
	Pattern->SetUniformVariable("uTerrainTextureHeight", totalTerrainHeight);
	Pattern->SetUniformVariable("uBlockSize", int(totalTerrainHeight/BLOCKS));
	Pattern->SetUniformVariable("uUseTransparancy",	UseTransparency);
	Pattern->SetUniformVariable("uUseEdgeTransparancy", UseEdgeTransparancy);
	Pattern->SetUniformVariable("uShowWater", ShowWater);
	Pattern->SetUniformVariable("uShinyWater", ShinyWater);


	if (AnimateWater) {
		Pattern->SetUniformVariable("uTime", Time);
	}
	else {
		Pattern->SetUniformVariable("uTime", 0);
	}

	// Set up river textures/maps
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, TerrainTexture);
	Pattern->SetUniformVariable("uTerrainTexUnit", 0 );

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, RiverMap);
	Pattern->SetUniformVariable("uRiverMapTexUnit", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, WaterNormalMap);
	Pattern->SetUniformVariable("uWaterNormalsTexUnit", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, WaterTexture);
	Pattern->SetUniformVariable("uWaterBaseTexUnit", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, FlowMap);
	Pattern->SetUniformVariable("uFlowMapTexUnit", 4);

	// Scale down model since it's pretty big for camera view
	glPushMatrix();
	glScalef(0.3, 0.3, 0.3);
	glCallList(TerrainList);
	glPopMatrix();

	// Turn off shader
	Pattern->Use(0);

	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}


void
DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// main menu callback:

void
DoMainMenu(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoProjectionMenu(int id)
{
	WhichProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// return the number of seconds since the start of the program:
float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus()
{
	glutSetWindow(MainWindow);

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu(DoProjectionMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);

	glutAddSubMenu("Projection", projmenu);

	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics()
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:

	glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// VisibilityFunc -- handle a change in window visibility
	// MenuStateFunc -- declare when a pop-up menu is in use
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutVisibilityFunc(Visibility);
	glutIdleFunc(Animate);

	srand(time(0));

	// Set up textures
	unsigned char* TextureArrayTerrain = BmpToTexture("final_project_assets/final_terrain_texture_v2_revised_banks.bmp", &totalTerrainWidth, &totalTerrainHeight);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &TerrainTexture);
	glBindTexture(GL_TEXTURE_2D, TerrainTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, totalTerrainWidth, totalTerrainHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArrayTerrain);

	int width, height;
	unsigned char* TextureArrayFlow = BmpToTexture("final_project_assets/flow_map.bmp", &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &FlowMap);
	glBindTexture(GL_TEXTURE_2D, FlowMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArrayFlow);

	unsigned char* TextureArrayWater = BmpToTexture("final_project_assets/water_base.bmp", &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &WaterTexture);
	glBindTexture(GL_TEXTURE_2D, WaterTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArrayWater);

	unsigned char* TextureArrayWaterNormals = BmpToTexture("final_project_assets/water_normals_2.bmp", &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &WaterNormalMap);
	glBindTexture(GL_TEXTURE_2D, WaterNormalMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArrayWaterNormals);

	unsigned char* TextureArrayRiverMap = BmpToTexture("final_project_assets/river_mask.bmp", &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &RiverMap);
	glBindTexture(GL_TEXTURE_2D, RiverMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArrayRiverMap);


	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// Create shaders
	Pattern = new GLSLProgram();
	bool valid = Pattern->Create("final_project_assets/river.vert", "final_project_assets/river.frag");

	if (!valid) {
		exit(-10);
	}
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	glutSetWindow(MainWindow);

	// create the axes:
	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();

	// Create riverbed model
	TerrainList = glGenLists(1);
	glNewList(TerrainList, GL_COMPILE);
	glPushMatrix();
	glScalef(0.5, 0.5, 0.5);
	LoadObjFile("final_project_assets/final_terrain.obj");
	glPopMatrix();
	glEndList();
}

// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT);	// will not return here
		break;				// happy compiler

	case 't':
		UseTransparency = !UseTransparency;
		break;
	case 'f':
		AnimateWater = !AnimateWater;
		break;
	case 'e':
		UseEdgeTransparancy = !UseEdgeTransparancy;
		break;
	case 'w':
		ShowWater = !ShowWater;
		break;
	case 's':
		ShinyWater = !ShinyWater;
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);

	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	case SCROLL_WHEEL_UP:
		Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	case SCROLL_WHEEL_DOWN:
		Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	Scale = 1.0;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	UseTransparency = true;
	AnimateWater = true;
	UseEdgeTransparancy = true;
	ShowWater = true;
	ShinyWater = true;
}


// called when user resizes the window:

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}