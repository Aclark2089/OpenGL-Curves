// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <sstream>
#include <string>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

typedef struct Vertex {
	float XYZW[4];
	float RGBA[4];
	void SetCoords(float *coords) {
		XYZW[0] = coords[0];
		XYZW[1] = coords[1];
		XYZW[2] = coords[2];
		XYZW[3] = coords[3];
	}
	void SetColor(float *color) {
		RGBA[0] = color[0];
		RGBA[1] = color[1];
		RGBA[2] = color[2];
		RGBA[3] = color[3];
	}
};

// ATTN: USE POINT STRUCTS FOR EASIER COMPUTATIONS
typedef struct point {
	float x, y, z;
	point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z){};
	point(float *coords) : x(coords[0]), y(coords[1]), z(coords[2]){};
	point operator -(const point& a)const {
		return point(x - a.x, y - a.y, z - a.z);
	}
	point operator +(const point& a)const {
		return point(x + a.x, y + a.y, z + a.z);
	}
	point operator *(const float& a)const {
		return point(x*a, y*a, z*a);
	}
	point operator /(const float& a)const {
		return point(x / a, y / a, z / a);
	}
	float* toArray() {
		float array[] = { x, y, z, 1.0f };
		return array;
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], unsigned short[], size_t, size_t, int);
void createObjects(void);
void pickVertex(void);
void moveVertex(void);
void drawScene(void);
void cleanup(void);

// GLFW Key Callbacks
static void mouseCallback(GLFWwindow*, int, int, int);
static void keyCallback(GLFWwindow*, int, int, int, int);

// Subdivision Functions
void subdivide(void);
void initSubIndicies(void);
void initSubdivisions(void);

// Bezier Functions
void bezierCurve(void);

// Catmull-Rom Functions
void cRomCurve(void);

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;

// Window Title
char* windowTitle = "R. Alex Clark (6416-3663)";

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS
const GLuint NumObjects = 6;	// number of different "objects" to be drawn
GLuint VertexArrayId[NumObjects] = { 0, 1, 2, 3, 4, 5 };
GLuint VertexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5 };
GLuint IndexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5 };
size_t NumVert[NumObjects] = { 0, 1, 2, 3, 4, 5 };

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint LightID;

// Define objects
Vertex Vertices[] =
{
	{ { 1.0f, 0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 0
	{ { 0.5f, 1.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 1
	{ { -0.5f, 1.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 2
	{ { -1.0f, 0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 3
	{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 4
	{ { 1.0f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 5
	{ { 0.5f, -1.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 6
	{ { -0.5f, -1.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 7 
	{ { -1.0f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 8
	{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 9 
};

// Index 0-9
unsigned short Indices[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

const size_t IndexCount = sizeof(Indices) / sizeof(unsigned short);
// ATTN: DON'T FORGET TO INCREASE THE ARRAY SIZE IN THE PICKING VERTEX SHADER WHEN YOU ADD MORE PICKING COLORS

// Verts 0-9
float pickingColor[IndexCount] = { 0 / 255.0f, 1 / 255.0f, 2 / 255.0f, 3 / 255.0f,
									4 / 255.0f, 5 / 255.0f, 6 / 255.0f, 7 / 255.0f,
									8 / 255.0f, 9 / 255.0f };

// ATTN: ADD YOU PER-OBJECT GLOBAL ARRAY DEFINITIONS HERE

// Subdivisions
int kCount = 0;
unsigned short nCPoints = IndexCount;
int kMax = 5;

// Subdivision Indicies
std::vector<unsigned short> subIndicies;

// Subdivisions
std::vector<Vertex*> subdivisions; // A vector of vertex arrays

float subdivideColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };

// Setup the indicies for the subdivisions
void initSubIndicies() {
	for (int i = 0; i < kMax; i++) {
		if (i == 0) {
			subIndicies.push_back (nCPoints * 2);
		}
		else {
			subIndicies.push_back (2 * subIndicies.at(i - 1));
		}
		printf("subIndicies(%d): %d\n", i, subIndicies.at(i));
	}
}

// Declare the size of the subdivisions that we are going to need
void initSubdivisions() {
	subdivisions.resize(kMax);
	for (int i = 0; i < kMax; i++) {
		subdivisions.at(i) = new Vertex[subIndicies.at(i)]; // @ each index is a vetex array of correct size
	}
}

void createObjects(void)
{
	// ATTN: DERIVE YOUR NEW OBJECTS HERE:
	// each has one vertices {pos;color} and one indices array (no picking needed here
	
}


void drawScene(void)
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glEnable(GL_PROGRAM_POINT_SIZE);

		glBindVertexArray(VertexArrayId[0]);	// draw Vertices
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
		//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);

		// ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
		switch (kCount) {
		case 5:

			glBindVertexArray(VertexArrayId[5]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivisions.at(4)), &subdivisions.at(4));
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[5], GL_UNSIGNED_SHORT, (void*)0);

		case 4:

			glBindVertexArray(VertexArrayId[4]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivisions.at(3)), &subdivisions.at(3));
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[4], GL_UNSIGNED_SHORT, (void*)0);

		case 3:

			glBindVertexArray(VertexArrayId[3]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivisions.at(2)), &subdivisions.at(2));
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[3], GL_UNSIGNED_SHORT, (void*)0);

		case 2:

			glBindVertexArray(VertexArrayId[2]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivisions.at(1)), &subdivisions.at(1));
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);

		case 1:

			glBindVertexArray(VertexArrayId[1]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivisions.at(0)), &subdivisions.at(0));
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[1], GL_UNSIGNED_SHORT, (void*)0);
		}

		// Binding All VAOs
		glBindVertexArray(0);
		glBindVertexArray(1);
		glBindVertexArray(2);
		glBindVertexArray(3);
		glBindVertexArray(4);
		glBindVertexArray(5);

	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickVertex(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1fv(pickingColorArrayID, NumVert[0], pickingColor);	// here we pass in the picking marker array

		// Draw the ponts
		glEnable(GL_PROGRAM_POINT_SIZE);
		glBindVertexArray(VertexArrayId[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);	// update buffer data
		glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow !
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel,
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

// fill this function in!
void moveVertex(void)
{
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);
	glm::vec3 worldCoords = glm::unProject(glm::vec3(window_width - xpos, window_height - ypos, 0.0), ModelMatrix, gProjectionMatrix, vp);

	if (gPickedIndex < IndexCount) {
		Vertices[gPickedIndex].XYZW[0] = worldCoords[0];
		Vertices[gPickedIndex].XYZW[1] = worldCoords[1];
	}

	if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, windowTitle, NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
	glfwSetCursorPos(window, window_width / 2, window_height / 2);

	// Register Callback Functions
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetKeyCallback(window, keyCallback);

	return 0;
}

void initOpenGL(void)
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(
		glm::vec3(0, 0, -5), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	createVAOs(Vertices, Indices, sizeof(Vertices), sizeof(Indices), 0);
	createObjects();
	// ATTN: create VAOs for each of the newly created objects here:
	// createVAOs(<fill this appropriately>);
	createVAOs(subdivisions.at(0), &subIndicies.at(0), sizeof(subdivisions.at(0)), sizeof(subIndicies.at(0)), 1);
	createVAOs(subdivisions.at(1), &subIndicies.at(1), sizeof(subdivisions.at(1)), sizeof(subIndicies.at(1)), 2);
	createVAOs(subdivisions.at(2), &subIndicies.at(2), sizeof(subdivisions.at(2)), sizeof(subIndicies.at(2)), 3);
	createVAOs(subdivisions.at(3), &subIndicies.at(3), sizeof(subdivisions.at(3)), sizeof(subIndicies.at(3)), 4);
	createVAOs(subdivisions.at(4), &subIndicies.at(4), sizeof(subdivisions.at(4)), sizeof(subIndicies.at(4)), 5);

}

void createVAOs(Vertex Vertices[], unsigned short Indices[], size_t BufferSize, size_t IdxBufferSize, int ObjectId) {

	NumVert[ObjectId] = IdxBufferSize / (sizeof GLubyte);

	GLenum ErrorCheckValue = glGetError();
	size_t VertexSize = sizeof(Vertices[0]);
	size_t RgbOffset = sizeof(Vertices[0].XYZW);

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	glGenBuffers(1, &IndexBufferId[ObjectId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IdxBufferSize, Indices, GL_STATIC_DRAW);

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color

	// Disable our Vertex Buffer Object
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickVertex();
	}
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE)
	switch (key) {
		case GLFW_KEY_1:
			printf("\nKey 1 Was Released\n");
			subdivide();
			break;
		case GLFW_KEY_2:
			printf("\nKey 2 Was Released\n");
			//bezierCurve();
			break;
		case GLFW_KEY_3:
			printf("\nKey 3 Was Released\n");
			//cRomCurve();
			break;
		default:
			printf("\nKey pressed\n");
	}
}

void subdivide() {
	if (++kCount % 6){
		printf("\nK value: %d\n", kCount); // Current K Level
		// Apply next subdivision layer

		// Values for the coords & keeping position
		float xCoord;
		float yCoord;
		int k;
		int j;

		for (int i = 0; i < (kCount == 1) ? nCPoints : subIndicies.at(kCount-1); i++) {
			k = i - 1;
			j = i + 1;

			if (i == 0) {
				k = nCPoints - 1;
			}
			if (i == nCPoints) {
				j = 0;
			}

				if (i % 2) {
					xCoord = (Vertices[k].XYZW[0] + (6 * Vertices[i].XYZW[0]) + Vertices[j].XYZW[0]) / 8;
					yCoord = (Vertices[k].XYZW[1] + (6 * Vertices[i].XYZW[1]) + Vertices[j].XYZW[1]) / 8;
					subdivisions.at(kCount - 1)[(i * 2) + 1].XYZW[0] = xCoord;
					subdivisions.at(kCount - 1)[(i * 2) + 1].XYZW[1] = yCoord;
					subdivisions.at(kCount - 1)[(i * 2) + 1].SetColor(subdivideColor);
				}
				else {
					xCoord = ((4 * Vertices[k].XYZW[0]) + (4 * Vertices[i].XYZW[0])) / 8;
					yCoord = ((4 * Vertices[k].XYZW[1]) + (4 * Vertices[i].XYZW[1])) / 8;
					subdivisions.at(kCount - 1)[i * 2].XYZW[0] = xCoord;
					subdivisions.at(kCount - 1)[i * 2].XYZW[1] = yCoord;
					subdivisions.at(kCount - 1)[i * 2].SetColor(subdivideColor);
				}
			
			printf("Value of subdivisions.at(%d)[%d]: %f, %f (X, Y)\n", kCount - 1, i * 2, xCoord, yCoord);
			printf("nCPoints value: %d\n", nCPoints); // Current # of CPoints for the K Level
		}
	}
	else {
		kCount = 0;
		printf("\nReseting K count\n");
		nCPoints = IndexCount;
		// Reset Subdivision Layer to 0
		printf("\nnCPoints value reset to %d", nCPoints);
	}
}

void bezierCurve() {}

void catmullRom() {}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// Setup Subdivision
	initSubIndicies();
	initSubdivisions();

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// DRAGGING: move current (picked) vertex with cursor
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
			moveVertex();

		// Subdividing verticies on user key press of 1

		// DRAWING SCENE
		createObjects();	// re-evaluate curves in case vertices have been moved
		drawScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}
