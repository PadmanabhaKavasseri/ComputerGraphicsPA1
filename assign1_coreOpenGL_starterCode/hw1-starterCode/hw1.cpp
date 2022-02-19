/*
	CSCI 420 Computer Graphics, USC
	Assignment 1: Height Fields with Shaders.
	C++ starter code

	Student username: <type your USC username here>
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
	#ifdef _DEBUG
		#pragma comment(lib, "glew32d.lib")
	#else
		#pragma comment(lib, "glew32.lib")
	#endif
#endif

#if defined(WIN32) || defined(_WIN32)
	char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
	char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position
int renderMode = 1;
int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
//these values will change as we move our mouse
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;


GLuint triVertexBuffer, triColorVertexBuffer;
GLuint triVertexArray;


//HeightMap Definitions
GLuint pointVBO, pointColorVBO, pointVAO;
GLuint lineVBO, linesColorVBO, lineVAO;
GLuint triVBO, triColorVBO, triVAO;

int sizePoint, sizeLine, sizeTri;

glm::vec3 * pointVertices;
glm::vec3 * lineVertices;
glm::vec3 * triVertices;



OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete [] screenshotData;
}

/**********************************************/
//HELPER FUNCTIONS
/**********************************************/

//creates three different vertex vectors
void createHeightMap(ImageIO * heightmapImage){

	 int imghi = heightmapImage->getHeight();
	 int imgwd	= heightmapImage->getWidth();

	 int size;

	 /*Generate Three different vertex arrays*/

	 //POINTS
	 size = imghi * imgwd;
	 sizePoint = size;
	 pointVertices = new glm::vec3[size];
	 for (int x = 0; x < imghi; x++) {
		  for (int y = 0; y < imgwd; y++) {
				pointVertices[y * imghi + x] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
		  }
	 }

	 cout << "num points " <<  pointVertices->length() << endl; // this is only three?

	 //LINES
//	 size = (imghi * imgwd * 4) - 2;
	 size = ((imghi-1)*(imgwd-1) * 4) + (((imghi-1)*2) + ((imgwd-1) * 2));
	 sizeLine = size;
	 lineVertices = new glm::vec3[size];

	 int idx = 1;

/*	 lineVertices[0] = pointVertices[0];
	 lineVertices[1] = pointVertices[1];
	 for (int i = 2; i < size - 1; i += 4) {
		  lineVertices[i] = pointVertices[idx];
		  lineVertices[i + 1] = pointVertices[idx];
		  //horizontal lines
		  lineVertices[i+2] = pointVertices[idx];
		  lineVertices[i + 3] = pointVertices[idx+imghi];
		  idx++;
	 }
	 lineVertices[size - 1] = pointVertices[idx];*/

	 idx = 0;
	 for (int x = 0; x < imghi; x++) {
		  for (int y = 0; y < imgwd; y++) {
				if(x<imghi-1){
					 lineVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
					 lineVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
//					 cout << "x" << endl;
				}
				if(y<imgwd-1){
					 lineVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
//					 cout <<"this works" << endl;
					 lineVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y-1);
//					 cout << "y" << endl;
				}

//				cout << "__  " << x<<","<<y<<endl;

		  }
	 }



	 //TRIANGLES
//	 size = (((imghi * imgwd)-3)+1)*3;
	 size = ((imghi-1)*(imgwd-1) * 6);// + (((imghi-1)*2) + ((imgwd-1) * 2));
	 sizeTri = size;
	 triVertices = new glm::vec3[size];

	 idx = 0;
	 for (int x = 0; x < imghi; x++) {
		  for (int y = 0; y < imgwd; y++) {
				if((x<imghi-1)&&(y<imgwd-1)){
					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y-1);

					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y-1);
					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y+1, 0) * 0.25, -y-1);
				}
//				else if(x<imghi-1){
//					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
//					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
////					 cout << "x" << endl;
//				}
//				else if(y<imgwd-1){
////					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
////					 cout <<"this works" << endl;
//					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y+1);
////					 cout << "y" << endl;
//				}

//				cout << "__  " << x<<","<<y<<endl;
		  }
	 }



/*	 idx = 1;

	 triVertices[0] = pointVertices[0];
	 triVertices[1] = pointVertices[1];
	 triVertices[2] = pointVertices[2];
	 for (int i = 3; i < size-2; i += 3) {
		  triVertices[i] = pointVertices[idx];
		  triVertices[i + 1] = pointVertices[idx+1];
		  triVertices[i + 2] = pointVertices[idx+2];
		  idx++;
	 }*/




//	 TODO MODE 4
}

void bindBuffers(){

	 //POINTS
	 glGenBuffers(1, &pointVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizePoint, pointVertices,
					  GL_STATIC_DRAW);

	 glGenVertexArrays(1, &pointVAO);
	 glBindVertexArray(pointVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, pointVAO);

	 //LINES
	 glGenBuffers(1, &lineVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeLine, lineVertices,
					  GL_STATIC_DRAW);

	 glGenVertexArrays(1, &lineVAO);
	 glBindVertexArray(lineVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, lineVAO);

	 //TRIANGLES
	 glGenBuffers(1, &triVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, triVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeTri, triVertices,
					  GL_STATIC_DRAW);

	 glGenVertexArrays(1, &triVAO);
	 glBindVertexArray(triVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, triVAO);
}
/**********************************************/
//MAIN FUNCTIONS
/**********************************************/

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.LoadIdentity();

	//add function to change y based on size of image
	matrix.LookAt(128, 200, 128, 128, 0, -128, 0, 0, -1);


	matrix.Rotate(landRotate[0], 1, 0, 0);
	matrix.Rotate(landRotate[1], 0, 1, 0);
	matrix.Rotate(landRotate[2], 0, 0, 1);
	matrix.Translate(landTranslate[0],landTranslate[1],landTranslate[2]);
	matrix.Scale(landScale[0],landScale[1],landScale[2]);

	/*
	 * m is the modelview matrix
	 * p is the projection matrix
	 * INFO: 07-Shaders-26
	 * */
	float m[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(m);

	float p[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(p);
	//slide 5 hsas to uploade te array p to
	// bind shader


	pipelineProgram->Bind();
	/*slide 7 of tips*/
	//added
	pipelineProgram->Bind();
	// set variable
	pipelineProgram->SetModelViewMatrix(m);
	pipelineProgram->SetProjectionMatrix(p);

	//maybe convert this into a swtich statement
	if(renderMode==1){
		pipelineProgram->Bind();
		glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
		glBindVertexArray(pointVAO);
		glDrawArrays(GL_POINTS, 0, sizePoint);

		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}
	else if(renderMode==2){
		 pipelineProgram->Bind();
		 glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		 glBindVertexArray(lineVAO);
		 glDrawArrays(GL_LINES, 0, sizeLine);

		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}
	else if(renderMode==3){
		 pipelineProgram->Bind();
		 glBindBuffer(GL_ARRAY_BUFFER, triVBO);
		 glBindVertexArray(triVAO);
		 glDrawArrays(GL_TRIANGLES, 0, sizeTri);

		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	glutSwapBuffers();
}

void initScene(int argc, char *argv[])
{
	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	/*
	 * read in the map
	 * get the height for each point
	 * */
/*

	int imghi = heightmapImage->getHeight();
	int imgwd	= heightmapImage->getWidth();
	int size = imghi * imgwd;
	glm::vec3 * map;
	//need to calculate the size of map
	switch (renderMode) {
		case 1: {//points
			size = imghi * imgwd;
			map = new glm::vec3[size];
			for (int x = 0; x < imghi; x++) {
				for (int y = 0; y < imgwd; y++) {
					map[y * imghi + x] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
				}
			}
			break;
		}
		case 2: {//lines
//			exit(1);

			glm::vec3 *temp = new glm::vec3[imghi * imgwd];

			size = (imghi * imgwd * 2) - 2;
			map = new glm::vec3[size];

			for (int x = 0; x < imghi; x++) {
				for (int y = 0; y < imgwd; y++) {
					temp[y * imghi + x] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
				}
			}

			int idx = 1;
			map[0] = temp[0];
			map[1] = temp[1];
			for (int i = 1; i < size - 1; i += 2) {
				map[i] = temp[idx];
				map[i + 1] = temp[idx];
				idx++;
			}
			map[size - 1] = temp[idx];

			for (int i = 0; i < size; ++i) {
				cout << glm::to_string(map[i]) << " " << endl;
			}
			cout << endl;
			break;
		}
		case 3: {

		}
		default: {
			cout << "Render Mode Error" << endl;
			break;
		}
	}
*/




/*
	 modify the following code accordingly
	glm::vec3 triangle[3] = {
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(1, 0, 0)
	};

	color will be all white right...
	glm::vec4 color[3] = {
		{0, 0, 1, 1},
		{1, 0, 0, 1},
		{0, 1, 0, 1},
	};

	GLuint heightMapBuffer;
	GLuint pointVAO;
	glGenBuffers(1, &triVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, triangle,
							 GL_STATIC_DRAW);

	pipelineProgram = new BasicPipelineProgram;
	int ret = pipelineProgram->Init(shaderBasePath);
	if (ret != 0) abort();

	glGenVertexArrays(1, &triVertexArray);
	glBindVertexArray(triVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);

	GLuint loc =
			glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer); //what
	loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	glEnable(GL_DEPTH_TEST);*/

	createHeightMap(heightmapImage);
	bindBuffers();

//	glGenBuffers(1, &pointVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * size, map,
//							 GL_STATIC_DRAW);


	//284... we dont need color rn right?

//	glGenBuffers(1, &hmColorVertexBuffer);
//	glBindBuffer(GL_ARRAY_BUFFER, hmColorVertexBuffer);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * imghi*imgwd, color, GL_STATIC_DRAW);


	pipelineProgram = new BasicPipelineProgram;
	int ret = pipelineProgram->Init(shaderBasePath);
	if (ret != 0) abort();

//	glGenVertexArrays(1, &pointVAO);
//	glBindVertexArray(pointVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, pointVAO);

	GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);


	//301... we dont need color rn right?
//	glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
//	loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
//	glEnableVertexAttribArray(loc);
//	glVertexAttribPointer(loc, imghi*imgwd, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	glEnable(GL_DEPTH_TEST);


//  sizeTri = 3;
//	sizeHm = size;

//	cout << size << endl;

	std::cout << "GL error: " << glGetError() << std::endl;
}

/**********************************************/
//AUXILIARY FUNCTIONS
/**********************************************/

void idleFunc()
{
	// do some stuff...

	// for example, here, you can save the screenshots to disk (to make the animation)

	// make the screen update
	glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);

	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.LoadIdentity();
	matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 1000.0f);
}

void mouseMotionDragFunc(int x, int y)
{

	//xy records the coordinates of the mouse on the screen
	// mouse has moved and one of the mouse buttons is pressed (dragging)

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the landscape
		case TRANSLATE:
			if (leftMouseButton)
			{
				// control x,y translation via the left mouse button
				landTranslate[0] += mousePosDelta[0] * 0.01f;
				landTranslate[1] -= mousePosDelta[1] * 0.01f;
			}
			if (middleMouseButton)
			{
				// control z translation via the middle mouse button
				landTranslate[2] += mousePosDelta[1] * 0.01f;
			}
			break;

		// rotate the landscape
		case ROTATE:
			if (leftMouseButton)
			{
				// control x,y rotation via the left mouse button
				landRotate[0] += mousePosDelta[1];
				landRotate[1] += mousePosDelta[0];
			}
			if (middleMouseButton)
			{
				// control z rotation via the middle mouse button
				landRotate[2] += mousePosDelta[1];
			}
			break;

		// scale the landscape
		case SCALE:
			if (leftMouseButton)
			{
				// control x,y scaling via the left mouse button
				landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
				landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
			}
			if (middleMouseButton)
			{
				// control z scaling via the middle mouse button
				landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
			}
			break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			leftMouseButton = (state == GLUT_DOWN);
		break;

		case GLUT_MIDDLE_BUTTON:
			middleMouseButton = (state == GLUT_DOWN);
		break;

		case GLUT_RIGHT_BUTTON:
			rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers())
	{
		case GLUT_ACTIVE_CTRL:
			controlState = TRANSLATE;
		break;

		case GLUT_ACTIVE_SHIFT:
			controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
		default:
			controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27: // ESC key
			exit(0); // exit the program
		break;
		case '1':
			cout << "Rendering with points" << endl;
			renderMode = 1;
		break;
		case '2':
			cout << "Rendering with lines" << endl;
			renderMode = 2;
			break;
		case '3':
			cout << "Rendering with triangles" << endl;
			renderMode = 3;
			break;
		case '4':
			cout << "You pressed 4" << endl;
			renderMode = 4;
			break;

		case ' ':
			cout << "You pressed the spacebar." << endl;
		break;

		case 'x':
			// take a screenshot
			saveScreenshot("screenshot.jpg");
		break;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc,argv);

	cout << "Initializing OpenGL..." << endl;

	#ifdef __APPLE__
		glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#else
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	#ifdef __APPLE__
		// This is needed on recent Mac OS X versions to correctly display the window.
		glutReshapeWindow(windowWidth - 1, windowHeight - 1);
	#endif

	// tells glut to use a particular display function to redraw
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
	#ifdef __APPLE__
		// nothing is needed on Apple
	#else
		// Windows, Linux
		GLint result = glewInit();
		if (result != GLEW_OK)
		{
			cout << "error: " << glewGetErrorString(result) << endl;
			exit(EXIT_FAILURE);
		}
	#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();
}


