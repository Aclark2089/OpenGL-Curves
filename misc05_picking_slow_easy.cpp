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

//#define DEBUG 1

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

// Setup Functions
void initIndicies(void);

// Subdivision Functions
void subdivide(void);
void initSubIndexCounts(void);
void calculateSubdivision(Vertex* thisSubdivision, Vertex* const lastSubdivision, int level);

// Bezier Functions
void bezierCurve(void);
void calculateBezSegment(const Vertex, const Vertex, const Vertex, Vertex *, Vertex *, Vertex *, Vertex *);

// Catmull-Rom Functions
void cRomCurve(void);
void calculateCRomPoints(const Vertex, const Vertex, const Vertex, Vertex *, Vertex *, Vertex *, Vertex *);
void calculateDecastlejauPoints(float u, int currIndex, int startIndex);

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;
int lastkey;

// Window Title
char* windowTitle = "R. Alex Clark (6416-3663)";

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS

const GLuint NumObjects = 9;	// number of different "objects" to be drawn

GLuint VertexArrayId[NumObjects] = { 0,				// Verticies Array
									1, 2, 3, 4, 5,  // Subdivision Arrays
									6,				// Bezier Array
									7, 				// Catmull - Rom Pts Array 
									8 };			// Decastlejau Pts Array					

GLuint VertexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
GLuint IndexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
size_t NumVert[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

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

// Ptr to Verticies
Vertex* verticiesPtr = Vertices;

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
int kCount = 0; // Level of Subdivision, level 0 == no subdivision
unsigned short nSubPoints = IndexCount; // Size of the original Verticies
int kMax = 6; // Max # 5 subdivisions & original the Verticies level

// subdivision indicies
unsigned short subIndicies1[20];
unsigned short subIndicies2[40];
unsigned short subIndicies3[80];
unsigned short subIndicies4[160];
unsigned short subIndicies5[320];

// subdivisions vertex arrays
Vertex subdivision1[20];
Vertex subdivision2[40];
Vertex subdivision3[80];
Vertex subdivision4[160];
Vertex subdivision5[320];

// ptrs to subdivisions
Vertex* subdivision1Ptr = subdivision1;
Vertex* subdivision2Ptr = subdivision2;
Vertex* subdivision3Ptr = subdivision3;
Vertex* subdivision4Ptr = subdivision4;
Vertex* subdivision5Ptr = subdivision5;

// Subdivision Counts
std::vector<int> subIndexCounts; // A vector of sizes for the indicies / subdivision verticies

// Bezier Curves

// bezier points
int nBezPts = 4; // # nBezPts per curve

// bez indicies
unsigned short bezIndicies[40];

// bez array
Vertex bezier[40]; // 40 bezier pts (N = 10 curves with 4 nBezPts)

// bezier ptrs
Vertex* bezierPtr = bezier; // Ptr to bezier array

// Catmull - Rom Curves

// cRom points
int nCRomPts = 4; // 15 cRom points per segment
int nDecastlePts = 15;

// cRom indicies
unsigned short cRomIndicies[40]; // indicies for the cRom bezier pts
unsigned short decastleIndicies[150]; // indicies for cRom curve

// cRom Array
Vertex cRom[40]; // 40 cRom pts (N = 10 segments @ 4 pts per segment)
Vertex decastle[150]; // 150 decastle pts (N = 10 segments @ 15 pts per segment)

// cRom ptr
Vertex* cRomPtr = cRom;	  // ptr to the cRom Segments Array
Vertex* decastlePtr = decastle;

// Vertex Colors
float subdivideColor[] = { 0.0f, 1.0f, 1.0f, 1.0f }; // Cyan Color for subdiv pts
float bezierColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow Color for bezier verticies
float cRomPtsColor[] = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red Color for catmull-rom bezier verts
float cRomCurveColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };

// Setup the indicies for the subdivisions
void initSubIndexCounts() {
	subIndexCounts.resize(kMax);
	for (int i = 0; i < kMax; i++) { // From 0 to kMax subdivisions (including size of original @ (0))
		if (i == 0) {
			subIndexCounts.at(0) = (nSubPoints); // Set size of Verticies
		}
		else {
			subIndexCounts.at(i) = (2 * subIndexCounts.at(i - 1)); // Else Vertex Size @ i = Size @ (i - 1) * 2
		}

#ifdef DEBUG
		printf("subIndexCounts(%d): %d\n", i, subIndexCounts.at(i)); // Log
#endif

	}

#ifdef DEBUG
	printf("\nIndex Counts Completed\n");
	getchar();
#endif

}

void initIndicies() {

	// Subdivisions
	for (int i = 0; i < 20; i++) {
		subIndicies1[i] = i;
	}
	for (int i = 0; i < 40; i++) {
		subIndicies2[i] = i;
	}
	for (int i = 0; i < 80; i++) {
		subIndicies3[i] = i;
	}
	for (int i = 0; i < 160; i++) {
		subIndicies4[i] = i;
	}
	for (int i = 0; i < 320; i++) {
		subIndicies5[i] = i;
	}

	// Bezier
	for (int i = 0; i < nBezPts*IndexCount; i++) {
		bezIndicies[i] = i;
	}

	// Catmull - Rom
	for (int i = 0; i < nCRomPts*IndexCount; i++) {
		cRomIndicies[i] = i;
	}

	// Decastlejau
	for (int i = 0; i < nDecastlePts*IndexCount; i++) {
		decastleIndicies[i] = i;
	}

}

void createObjects(void)
{
	// ATTN: DERIVE YOUR NEW OBJECTS HERE:
	// each has one vertices {pos;color} and one indices array (no picking needed here
	
	// Check Sub
	if (lastkey == 1) {
		subdivide();
	}

	// Subdivision VAOs
	createVAOs(subdivision1, subIndicies1, sizeof(subdivision1), sizeof(subIndicies1), 1);
	createVAOs(subdivision2, subIndicies2, sizeof(subdivision2), sizeof(subIndicies2), 2);
	createVAOs(subdivision3, subIndicies3, sizeof(subdivision3), sizeof(subIndicies3), 3);
	createVAOs(subdivision4, subIndicies4, sizeof(subdivision4), sizeof(subIndicies4), 4);
	createVAOs(subdivision5, subIndicies5, sizeof(subdivision5), sizeof(subIndicies5), 5);
	
	// Check Bez
	if (lastkey == 2) {
		bezierCurve();
	}

	// Bezier VAO
	createVAOs(bezier, bezIndicies, sizeof(bezier), sizeof(bezIndicies), 6);

	if (lastkey == 3) {
		cRomCurve();
	}

	// Catmull - Rom VAO
	createVAOs(cRom, cRomIndicies, sizeof(cRom), sizeof(cRomIndicies), 7);
	createVAOs(decastle, decastleIndicies, sizeof(decastle), sizeof(decastleIndicies), 8);
	
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

		switch (lastkey)
		{
		case 1:
			switch (kCount) {
				case 5:

					glBindVertexArray(VertexArrayId[5]);	// draw Vertices
					glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivision5), subdivision5);
					//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
					glDrawElements(GL_POINTS, NumVert[5], GL_UNSIGNED_SHORT, (void*)0);
					break;

				case 4:

					glBindVertexArray(VertexArrayId[4]);	// draw Vertices
					glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivision4), subdivision4);
					//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
					glDrawElements(GL_POINTS, NumVert[4], GL_UNSIGNED_SHORT, (void*)0);
					break;

				case 3:

					glBindVertexArray(VertexArrayId[3]);	// draw Vertices
					glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivision3), subdivision3);
					//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
					glDrawElements(GL_POINTS, NumVert[3], GL_UNSIGNED_SHORT, (void*)0);
					break;

				case 2:

					glBindVertexArray(VertexArrayId[2]);	// draw Vertices
					glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivision2), subdivision2);
					//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
					glDrawElements(GL_POINTS, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);
					break;

				case 1:

					glBindVertexArray(VertexArrayId[1]);	// draw Vertices
					glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdivision1), subdivision1);
					//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
					glDrawElements(GL_POINTS, NumVert[1], GL_UNSIGNED_SHORT, (void*)0);
					break;
			}
			break;

		// Create Bez Curve
		case 2:
			glBindVertexArray(VertexArrayId[6]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bezier), bezier);
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[6], GL_UNSIGNED_SHORT, (void*)0);
			break;

		// Create Catmull Rom Curve using CRom pts and Decastlejau pts
		case 3:
			glBindVertexArray(VertexArrayId[7]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cRom), cRom);
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[7], GL_UNSIGNED_SHORT, (void*)0);

			glBindVertexArray(VertexArrayId[8]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(decastle), decastle);
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[8], GL_UNSIGNED_SHORT, (void*)0);

		}


		// Binding All VAOs
		glBindVertexArray(0);

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

	// Vertex VAO
	createVAOs(Vertices, Indices, sizeof(Vertices), sizeof(Indices), 0);

	// Setup Objects
	createObjects();

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
	if (action == GLFW_RELEASE) {
		std::string str = "";
		switch (key) {
		case GLFW_KEY_1:
			str = "\nKey 1 Was Released\n";
			lastkey = 1;
			kCount++;
			break;
		case GLFW_KEY_2:
			str = "\nKey 2 Was Released\n";
			lastkey = 2;
			kCount = 0;
			break;
		case GLFW_KEY_3:
			str = "\nKey 3 Was Released\n";
			lastkey = 3;
			kCount = 0;
			break;
		default:
			str = "\nKey pressed\n";
		}

#ifdef DEBUG
		printf("%s", str);
#endif

	}
}

void subdivide() {

	if (kCount % 6) { // If current kCount is not 0 or has just moved past 5

#ifdef DEBUG
		printf("\nK value: %d\n", kCount); // Current K Level
#endif

		calculateSubdivision(subdivision1Ptr, verticiesPtr, 1); // Calculate the subdiv of level 1

		// For each level of k past 1 we need to work our way down and calculate each subsequent level of subdiv
		if (kCount > 1) { 
			calculateSubdivision(subdivision2Ptr, subdivision1Ptr, 2);
		}
		if (kCount > 2) {
			calculateSubdivision(subdivision3Ptr, subdivision2Ptr, 3);
		}
		if (kCount > 3) {
			calculateSubdivision(subdivision4Ptr, subdivision3Ptr, 4);
		}
		if (kCount > 4) {
			calculateSubdivision(subdivision5Ptr, subdivision4Ptr, 5);
		}
		
	}
	else {

		// Reset Subdivision Layer to 0
		kCount = 0;

#ifdef DEBUG
		printf("\nReseting K count\n");
		printf("\nnSubPoints value reset to %d\n", subIndexCounts.at(0));
#endif
	}
}

void calculateSubdivision(Vertex* thisSubdivision, Vertex* const lastSubdivision, int level) {

	// Values for the coords & keeping position
	float xCoord; // X Coord For The Vertex
	float yCoord; // Y Coord For The Vertex
	int k; // Previous level (i - 1)
	int j; // Next level (i + 1)
	int numVerts = subIndexCounts.at(level-1); // Number of verts in the last subdivision level

	// Loop through arrays and set thisSubdivision based on the values of the indicies of lastSubdivision
	for (int i = 0; i < numVerts; i++) {

		// k = previous index, j = next index
		k = i - 1;
		j = i + 1;

		// If i == 0, k has to be the last index of the previous subdiv
		if (i == 0) {
			k = numVerts - 1;
		}
		// If i == end of the last subdiv, j = the first index of the last subdiv
		if (i == numVerts - 1) {
			j = 0;
		}

		// P[2i]
		xCoord = (lastSubdivision[k].XYZW[0] + (6 * lastSubdivision[i].XYZW[0]) + lastSubdivision[j].XYZW[0]) / 8;
		yCoord = (lastSubdivision[k].XYZW[1] + (6 * lastSubdivision[i].XYZW[1]) + lastSubdivision[j].XYZW[1]) / 8;
		thisSubdivision[(i * 2) + 1].XYZW[0] = xCoord; // X
		thisSubdivision[(i * 2) + 1].XYZW[1] = yCoord; // Y
		thisSubdivision[(i * 2) + 1].XYZW[3] = 1.0f; // W
		thisSubdivision[(i * 2) + 1].SetColor(subdivideColor); // Subdivision Color Set

		// P[2i+1]
		xCoord = ((4 * lastSubdivision[k].XYZW[0]) + (4 * lastSubdivision[i].XYZW[0])) / 8;
		yCoord = ((4 * lastSubdivision[k].XYZW[1]) + (4 * lastSubdivision[i].XYZW[1])) / 8;
		thisSubdivision[i * 2].XYZW[0] = xCoord; // X
		thisSubdivision[i * 2].XYZW[1] = yCoord; // Y
		thisSubdivision[i * 2].XYZW[3] = 1.0f; // W
		thisSubdivision[i * 2].SetColor(subdivideColor); // Subdivision Color Set

#ifdef DEBUG
		printf("Value of subdivision%d[%d]: %f, %f (X, Y)\n", kCount, i * 2, xCoord, yCoord);
		printf("Value of subdivision%d[%d]: %f, %f (X, Y)\n", kCount, (i * 2) + 1, xCoord, yCoord);
#endif

	}

#ifdef DEBUG
	printf("There were %d points in the previous subdiv level\n", numVerts); // Current # of CPoints for the K Level
#endif

}

void bezierCurve() {

	int j;				 
	int k;

	for (int i = 0; i < IndexCount; i++) {
		(i == (IndexCount - 1)) ? (j = 0) : (j = i + 1);
		(i == 0) ? (k = IndexCount - 1) : (k = i - 1);

#ifdef DEBUG
		printf("Setting anchor points of i: %d, j: %d, k: %d\n", i, j, k);
#endif

		calculateBezSegment(verticiesPtr[i], verticiesPtr[j], verticiesPtr[k], &bezierPtr[(4 * i)], &bezierPtr[(4 * i) + 1], &bezierPtr[(4 * i) + 2], &bezierPtr[(4 * i) + 3]);

#ifdef DEBUG
		printf("Bezier c1 and c2 set to %f,%f and %f,%f\n", bezierPtr[(4 * i) + 1].XYZW[0], bezierPtr[(4 * i) + 1].XYZW[1], bezier[(4 * i) + 2].XYZW[0], bezier[(4 * i) + 2].XYZW[1]);
		printf("Bezier c0 and c3 set to %f,%f and %f,%f\n", bezierPtr[(4 * i)].XYZW[0], bezierPtr[(4 * i)].XYZW[1], bezier[(4 * i) + 3].XYZW[0], bezier[(4 * i) + 3].XYZW[1]);
		printf("Positions in Bezier array were c1: %d, c2: %d, c3: %d, and c4: %d\n", (i * 4), (i * 4) + 1, (i * 4) + 2, (i * 4) + 3);
		getchar();
#endif
		
	}
}

void calculateBezSegment(const Vertex p1, const Vertex pPlus1, const Vertex pMinus1, Vertex* c0, Vertex* c1, Vertex* c2, Vertex* c3) {

	// Values for the coords & keeping position

	float x0; // X Coord For The c0 Vertex
	float y0; // Y Coord For The c0 Vertex

	float x1; // X Coord For The c1 Vertex
	float y1; // Y Coord For The c1 Vertex

	float x2; // X Coord For The c2 Vertex
	float y2; // Y Coord For The c2 Vertex

	float x3; // X Coord For The c3 Vertex
	float y3; // Y Coord For The c3 Vertex

	// calc c1 xy
	x1 = ((2 * p1.XYZW[0]) + pPlus1.XYZW[0]) / 3;
	y1 = ((2 * p1.XYZW[1]) + pPlus1.XYZW[1]) / 3;

	// calc c2 xy
	x2 = (p1.XYZW[0] + (2 * pPlus1.XYZW[0])) / 3;
	y2 = (p1.XYZW[1] + (2 * pPlus1.XYZW[1])) / 3;

	// Set xy for c1
	c1->XYZW[0] = x1;
	c1->XYZW[1] = y1;
	c1->XYZW[2] = 0.0f;
	c1->XYZW[3] = 1.0f;
	c1->SetColor(bezierColor);

	// Set xy for c2
	c2->XYZW[0] = x2;
	c2->XYZW[1] = y2;
	c2->XYZW[2] = 0.0f;
	c2->XYZW[3] = 1.0f;
	c2->SetColor(bezierColor);

	// calc c0 xy
	x0 = (pMinus1.XYZW[0] + (2 * p1.XYZW[0])) / 3;
	y0 = (pMinus1.XYZW[1] + (2 * p1.XYZW[1])) / 3;

	// midpoint of i-1 and i Vertex
	x0 = (x0 + x1) / 2;
	y0 = (y0 + y1) / 2;

	// calc c3 xy
	x3 = (p1.XYZW[0] + (2 * pPlus1.XYZW[0])) / 3;
	y3 = (p1.XYZW[1] + (2 * pPlus1.XYZW[1])) / 3;

	// Set xy for c0
	c0->XYZW[0] = x0;
	c0->XYZW[1] = y0;
	c0->XYZW[2] = 0.0f;
	c0->XYZW[3] = 1.0f;
	c0->SetColor(bezierColor);

	// Set xy for c3
	c3->XYZW[0] = x3;
	c3->XYZW[1] = y3;
	c3->XYZW[2] = 0.0f;
	c3->XYZW[3] = 1.0f;
	c3->SetColor(bezierColor);
}

void cRomCurve() {

	int j;
	int k;

	for (int i = 0; i < IndexCount; i++) {
		
		(i == (IndexCount - 1)) ? (j = 0) : (j = i + 1);
		(i == 0) ? (k = IndexCount - 1) : (k = i - 1);

#ifdef DEBUG
			printf("Setting anchor points of i: %d, j: %d, k: %d\n", i, j, k);
#endif

			calculateCRomPoints(verticiesPtr[i], verticiesPtr[j], verticiesPtr[k], &cRomPtr[(4 * i)], &cRomPtr[(4 * i) + 1], &cRomPtr[(4 * i) + 2], &cRomPtr[(4 * i) + 3]);

#ifdef DEBUG
			printf("cRom c1 and c2 set to %f,%f and %f,%f\n", cRomPtr[(4 * i) + 1].XYZW[0], cRomPtr[(4 * i) + 1].XYZW[1], bezier[(4 * i) + 2].XYZW[0], cRomPtr[(4 * i) + 2].XYZW[1]);
			printf("cRom c0 and c3 set to %f,%f and %f,%f\n", cRomPtr[(4 * i)].XYZW[0], cRomPtr[(4 * i)].XYZW[1], bezier[(4 * i) + 3].XYZW[0], cRomPtr[(4 * i) + 3].XYZW[1]);
			printf("Positions in cRom array were c1: %d, c2: %d, c3: %d, and c4: %d\n", (i * 4), (i * 4) + 1, (i * 4) + 2, (i * 4) + 3);
			getchar();
#endif

	}

for (int i = 0; i < IndexCount; i++) {

		(i == (IndexCount - 1)) ? (j = 0) : (j = i + 1);
		(i == 0) ? (k = IndexCount - 1) : (k = i - 1);

		//calculateDecastlejauPoints();

#ifdef DEBUG
		printf("Bezier c1 and c2 set to %f,%f and %f,%f\n", bezierPtr[(4 * i) + 1].XYZW[0], bezierPtr[(4 * i) + 1].XYZW[1], bezier[(4 * i) + 2].XYZW[0], bezier[(4 * i) + 2].XYZW[1]);
		printf("Positions in bezier array were %d and %d", (i * 4) + 1, (i * 4) + 2);
		getchar();
#endif

	}
}

void calculateCRomPoints(const Vertex p1, const Vertex pPlus1, const Vertex pMinus1, Vertex* c0, Vertex* c1, Vertex* c2, Vertex* c3) {

	// Values for the coords & keeping position

	float x0; // X Coord For The c0 Vertex
	float y0; // Y Coord For The c0 Vertex

	float x1; // X Coord For The c1 Vertex
	float y1; // Y Coord For The c1 Vertex

	float x2; // X Coord For The c2 Vertex
	float y2; // Y Coord For The c2 Vertex

	float x3; // X Coord For The c3 Vertex
	float y3; // Y Coord For The c3 Vertex

	float xTangent = pPlus1.XYZW[0] - pMinus1.XYZW[0]; // X tangent
	float yTangent = pPlus1.XYZW[1] - pMinus1.XYZW[1]; // Y tangent
	float tWeight = 0.2; // t value @ position for pt (affects where the cRomPoints are displayed, tried to get it close to the website's)

	// Calculate c0
	x0 = p1.XYZW[0];
	y0 = p1.XYZW[1];

	// Calculate c1
	x1 = x0 - tWeight*xTangent;
	y1 = y0 - tWeight*yTangent;

	// Calculate c2
	x2 = x0 + tWeight*xTangent;
	y2 = y0 + tWeight*yTangent;

	// Calculate c3
	x3 = pPlus1.XYZW[0];
	y3 = pPlus1.XYZW[1];

	// Set c0
	c0->XYZW[0] = x0;
	c0->XYZW[1] = y0;
	c0->XYZW[2] = 0.0f;
	c0->XYZW[3] = 1.0f;
	c0->SetColor(cRomPtsColor);

	// Set c1
	c1->XYZW[0] = x1;
	c1->XYZW[1] = y1;
	c1->XYZW[2] = 0.0f;
	c1->XYZW[3] = 1.0f;
	c1->SetColor(cRomPtsColor);

	// Set c2
	c2->XYZW[0] = x2;
	c2->XYZW[1] = y2;
	c2->XYZW[2] = 0.0f;
	c2->XYZW[3] = 1.0f;
	c2->SetColor(cRomPtsColor);

	// Set c3
	c3->XYZW[0] = x3;
	c3->XYZW[1] = y3;
	c3->XYZW[2] = 0.0f;
	c3->XYZW[3] = 1.0f;
	c3->SetColor(cRomPtsColor);

}

void calculateDecastlejauPoints() {

}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// Setup Subdivision
	initSubIndexCounts();
	initIndicies();

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
		createObjects(); // re-evaluate curves in case vertices have been moved
		drawScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}
