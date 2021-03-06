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
#include <chrono>
#include <thread>
#include <math.h>

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
using namespace std::chrono;
using namespace std::this_thread;


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
int imghi, imgwd;

int sidx = 0;

GLuint triVertexBuffer, triColorVertexBuffer;
GLuint triVertexArray;


//HeightMap Definitions
GLuint pointVBO, pointColorVBO, pointVAO;
GLuint lineVBO, linesColorVBO, lineVAO;
GLuint triVBO, triColorVBO, triVAO;

GLuint leftV, rightV, upV, downV, smoothVAO;

int sizePoint, sizeLine, sizeTri;

glm::vec3 * pointVertices;
glm::vec3 * lineVertices;
glm::vec3 * triVertices;

glm::vec3 * leftVertices;
glm::vec3 * rightVertices;
glm::vec3 * upVertices;
glm::vec3 * downVertices;

glm::vec4 * pointVerticesColor;
glm::vec4 * lineVerticesColor;
glm::vec4 * triVerticesColor;


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

void nborVertFill(int x, int y){
	 float height;// = heightmapImage->getPixel(x, y, 0) * 0.25f;
	 //left
	 if(x>0){
		  height = heightmapImage->getPixel(x-1, y, 0) * 0.25f;
		  leftVertices[sidx] = glm::vec3(x-1,height,-y);
	 }
	 else{
		  height = heightmapImage->getPixel(x, y, 0) * 0.25f;
		  leftVertices[sidx] = glm::vec3(x,height,-y);
	 }
	 //right
	 if(x < imgwd-1){
		  height = heightmapImage->getPixel(x+1, y, 0) * 0.25f;
		  rightVertices[sidx] = glm::vec3(x+1,height,-y);
	 }
	 else{
		  height = heightmapImage->getPixel(x, y, 0) * 0.25f;
		  rightVertices[sidx] = glm::vec3(x,height,-y);
	 }
	 //up
	 if(y < imghi - 1){
		  height = heightmapImage->getPixel(x, y+1, 0) * 0.25f;
		  upVertices[sidx] = glm::vec3(x,height,-y-1);
	 }
	 else{
		  height = heightmapImage->getPixel(x, y, 0) * 0.25f;
		  upVertices[sidx] = glm::vec3(x,height,-y);
	 }
	 //down
	 if(y > 0){
		  height = heightmapImage->getPixel(x, y-1, 0) * 0.25f;
		  downVertices[sidx] = glm::vec3(x,height,-y+1);
	 }
	 else{
		  height = heightmapImage->getPixel(x, y, 0) * 0.25f;
		  downVertices[sidx] = glm::vec3(x,height,-y);
	 }
	 sidx++;
}

//creates three different vertex vectors
void createHeightMap(ImageIO * heightmapImage){

	 imghi = heightmapImage->getHeight();
	 imgwd = heightmapImage->getWidth();

	 int size;
	 float color;

	 //POINTS
	 size = imghi * imgwd;
	 sizePoint = size;
	 pointVertices = new glm::vec3[size];
	 pointVerticesColor = new glm::vec4[size];
	 for (int x = 0; x < imghi; x++) {
		  for (int y = 0; y < imgwd; y++) {
				pointVertices[y * imghi + x] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
				color = ((float) heightmapImage->getPixel(x, y, 0)) / (float)255.0f;
				pointVerticesColor[y * imghi + x] = glm::vec4(color,color,color,1.0f);

		  }
	 }


	 //LINES
//	 size = (imghi * imgwd * 4) - 2;
	 size = ((imghi-1)*(imgwd-1) * 4) + (((imghi-1)*2) + ((imgwd-1) * 2));
	 sizeLine = size;
	 lineVertices = new glm::vec3[size];
	 lineVerticesColor = new glm::vec4[size];
	 int idx = 1;

	 idx = 0;
	 for (int x = 0; x < imghi; x++) {
		  for (int y = 0; y < imgwd; y++) {
				if(x<imghi-1){
					 lineVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
					 color = ((float) heightmapImage->getPixel(x, y, 0)) / (float)255.0f;
					 lineVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);

					 lineVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
					 color = ((float) heightmapImage->getPixel(x+1, y, 0)) / (float)255.0f;
					 lineVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);
				}
				if(y<imgwd-1){
					 lineVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
					 color = ((float) heightmapImage->getPixel(x, y, 0)) / (float)255.0f;
					 lineVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);

					 lineVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y-1);
					 color = ((float) heightmapImage->getPixel(x, y+1, 0)) / (float)255.0f;
					 lineVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);
				}
		  }
	 }


	 //TRIANGLES
//	 size = (((imghi * imgwd)-3)+1)*3;
	 size = ((imghi-1)*(imgwd-1) * 6);// + (((imghi-1)*2) + ((imgwd-1) * 2));
	 sizeTri = size;
	 triVertices = new glm::vec3[size];
	 leftVertices = new glm::vec3[size];
	 rightVertices =  new glm::vec3[size];
	 upVertices = new glm::vec3[size];
	 downVertices = new glm::vec3[size];
	 triVerticesColor = new glm::vec4[size];

	 idx = 0;
	 for (int x = 0; x < imghi; x++) {
		  for (int y = 0; y < imgwd; y++) {
				if((x<imghi-1)&&(y<imgwd-1)){
					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y, 0) * 0.25, -y);
					 nborVertFill(x,y);
					 color = ((float) heightmapImage->getPixel(x, y, 0)) / (float)255.0f;
					 triVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);

					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
					 nborVertFill(x+1,y);
					 color = ((float) heightmapImage->getPixel(x+1, y, 0)) / (float)255.0f;
					 triVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);

					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y-1);
					 nborVertFill(x,y+1);
					 color = ((float) heightmapImage->getPixel(x, y+1, 0)) / (float)255.0f;
					 triVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);


					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y, 0) * 0.25, -y);
					 nborVertFill(x+1,y);
					 color = ((float) heightmapImage->getPixel(x+1, y, 0)) / (float)255.0f;
					 triVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);

					 triVertices[idx++] = glm::vec3(x, heightmapImage->getPixel(x, y+1, 0) * 0.25, -y-1);
					 nborVertFill(x,y+1);
					 color = ((float) heightmapImage->getPixel(x, y+1, 0)) / (float)255.0f;
					 triVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);

					 triVertices[idx++] = glm::vec3(x+1, heightmapImage->getPixel(x+1, y+1, 0) * 0.25, -y-1);
					 nborVertFill(x+1,y+1);
					 color = ((float) heightmapImage->getPixel(x+1, y+1, 0)) / (float)255.0f;
					 triVerticesColor[idx-1] = glm::vec4(color,color,color,1.0f);
				}
		  }
	 }
}

void bindBuffers(){

	 //POINTS
	 glGenBuffers(1, &pointVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizePoint, pointVertices,
					  GL_STATIC_DRAW);

	 glGenBuffers(1, &pointColorVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, pointColorVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * sizePoint, pointVerticesColor,
					  GL_STATIC_DRAW);

	 glGenVertexArrays(1, &pointVAO);
	 glBindVertexArray(pointVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, pointVAO);

	 //LINES
	 glGenBuffers(1, &lineVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeLine, lineVertices,
					  GL_STATIC_DRAW);

	 glGenBuffers(1, &linesColorVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, linesColorVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * sizeLine, lineVerticesColor,
					  GL_STATIC_DRAW);

	 glGenVertexArrays(1, &lineVAO);
	 glBindVertexArray(lineVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, lineVAO);

	 //TRIANGLES
	 glGenBuffers(1, &triVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, triVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeTri, triVertices,
					  GL_STATIC_DRAW);

	 glGenBuffers(1, &triColorVBO);
	 glBindBuffer(GL_ARRAY_BUFFER, triColorVBO);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * sizeTri, triVerticesColor,
					  GL_STATIC_DRAW);

	 glGenVertexArrays(1, &triVAO);
	 glBindVertexArray(triVAO);
	 glBindBuffer(GL_ARRAY_BUFFER, triVAO);

	 //SMOOTHER
	 glGenBuffers(1, &leftV);
	 glBindBuffer(GL_ARRAY_BUFFER, leftV);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeTri, leftVertices,
					  GL_STATIC_DRAW);

	 glGenBuffers(1, &rightV);
	 glBindBuffer(GL_ARRAY_BUFFER, rightV);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeTri, rightVertices,
					  GL_STATIC_DRAW);

	 glGenBuffers(1, &upV);
	 glBindBuffer(GL_ARRAY_BUFFER, upV);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeTri, upVertices,
					  GL_STATIC_DRAW);

	 glGenBuffers(1, &downV);
	 glBindBuffer(GL_ARRAY_BUFFER, downV);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sizeTri, downVertices,
					  GL_STATIC_DRAW);
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
//	matrix.LookAt(89, 250, 89, 128, 0, -128, 0, 0, -1);
	matrix.LookAt(128, 250, 128, 128, 0, -128, 0, 0, -1);


	matrix.Rotate(landRotate[0], 1, 0, 0);
	matrix.Rotate(landRotate[1], 0, 1, 0);
	matrix.Rotate(landRotate[2], 0, 0, 1);
	matrix.Translate(landTranslate[0],landTranslate[1],landTranslate[2]);
	matrix.Scale(landScale[0],landScale[1],landScale[2]);

	float m[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(m);

	float p[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(p);

	pipelineProgram->Bind();

	pipelineProgram->SetModelViewMatrix(m);
	pipelineProgram->SetProjectionMatrix(p);
	pipelineProgram->SetRenderMode(0);
	//maybe convert this into a swtich statement
	if(renderMode==1){
		 pipelineProgram->Bind();
		 glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, pointColorVBO);
		 GLuint col = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
		 glEnableVertexAttribArray(col);
		 glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		glBindVertexArray(pointVAO);
		glDrawArrays(GL_POINTS, 0, sizePoint);

	}
	else if(renderMode==2){
//		 pipelineProgram->Bind();
		 glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, linesColorVBO);
		 GLuint col = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
		 glEnableVertexAttribArray(col);
		 glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		 glBindVertexArray(lineVAO);
		 glDrawArrays(GL_LINES, 0, sizeLine);

	}
	else if(renderMode==3){
		 pipelineProgram->Bind();
		 glBindBuffer(GL_ARRAY_BUFFER, triVBO);
		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, triColorVBO);
		 loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindVertexArray(triVAO);
		 glDrawArrays(GL_TRIANGLES, 0, sizeTri);
	}
	else if(renderMode==4){

		 pipelineProgram->Bind();
		 pipelineProgram->SetRenderMode(1);
		 glBindBuffer(GL_ARRAY_BUFFER, leftV);
		 GLuint l = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "left");
		 glEnableVertexAttribArray(l);
		 glVertexAttribPointer(l, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, rightV);
		 GLuint r = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "right");
		 glEnableVertexAttribArray(r);
		 glVertexAttribPointer(r, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, upV);
		 GLuint u = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "up");
		 glEnableVertexAttribArray(u);
		 glVertexAttribPointer(u, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, downV);
		 GLuint d = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "down");
		 glEnableVertexAttribArray(d);
		 glVertexAttribPointer(d, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, triVBO);
		 GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindBuffer(GL_ARRAY_BUFFER, triColorVBO);
		 loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
		 glEnableVertexAttribArray(loc);
		 glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		 glBindVertexArray(triVAO);
		 glDrawArrays(GL_TRIANGLES, 0, sizeTri);
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

	glClearColor(0.224f, 0.098f, 0.298f, 1.0f);

	createHeightMap(heightmapImage);
	bindBuffers();

	pipelineProgram = new BasicPipelineProgram;
	int ret = pipelineProgram->Init(shaderBasePath);
	if (ret != 0) abort();

	glEnable(GL_DEPTH_TEST);

	std::cout << "GL error: " << glGetError() << std::endl;
}

/**********************************************/
//AUXILIARY FUNCTIONS
/**********************************************/
int img_num = 0;
float theta;
string pathToFolder = "/Users/padmanabha/School/ComputerGraphics/ComputerGraphicsPA1/assign1_coreOpenGL_starterCode/screenshots/";
string fileName = pathToFolder;
void idleFunc()
{
	 sleep_for(nanoseconds(10000000));


	 if(img_num<=300){
		  fileName += "p" + to_string(img_num) + ".jpg";
		  saveScreenshot(fileName.c_str());
		  fileName = pathToFolder;
		  img_num++;
	 }

//	float r = 128.0f;
//	 for (int i = 0; i < 360; ++i) {
//		  matrix.LookAt(r* cos(i), 250, r* sin(i), 128, 0, -128, 0, 0, -1);
//	 }

/*//	 sleep_for(nanoseconds(1000000));
//	 matrix.Translate(0,-1.8,0);
//	 matrix.Rotate(0.5f, 0, 1, 0);
////	 matrix.Rotate(0.5f, 1, 0, 0);
//	 matrix.Translate(0,1.8,0);
//
//	 sleep_for(nanoseconds(10000));
//	 sleep_until(system_clock::now() + seconds(2));*/
/*	 matrix.LookAt(128.0 + (r * cos(M_PI/16)), 250, -128.0 + (r *sin(M_PI/16)), 128, 0, -128, 0, 0, -1);

	 matrix.LookAt(89, 250, 128, 128, 0, -128, 0, 0, -1);*/
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
				landTranslate[0] += mousePosDelta[0] * 0.1f;
				landTranslate[1] -= mousePosDelta[1] * 0.1f;
			}
			if (middleMouseButton)
			{
				// control z translation via the middle mouse button
				landTranslate[2] += mousePosDelta[1] * 0.1f;
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
//			glUniform1i(mode,0);
		break;
		case '2':
			cout << "Rendering with lines" << endl;
			renderMode = 2;
//			glUniform1i(mode,0);
			break;
		case '3':
			cout << "Rendering with triangles" << endl;
			renderMode = 3;
//			glUniform1i(mode,0);
			break;
		case '4':
			cout << "Rendering with smoothing" << endl;
			renderMode = 4;
//			glUniform1i(mode,1);
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


