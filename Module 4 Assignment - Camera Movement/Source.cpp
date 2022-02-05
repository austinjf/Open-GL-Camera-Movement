#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" // Camera class

using namespace std; // Uses the standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Module 4 Assignment: Camera Movement - Austin Fuchs"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbo[2];		// Handle for the vertex buffer objects (vertex data and indice data)
		GLuint nIndices;	// Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* window = nullptr;
	// Triangle mesh data
	GLMesh mesh;
	// Shader program
	GLuint programId;

	// camera
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	float lastX = WINDOW_WIDTH / 2.0f;
	float lastY = WINDOW_HEIGHT / 2.0f;
	bool firstMouse = true;

	// time between frames
	float deltaTime = 0.0f;
	float lastFrameTime = 0.0f;
}

// user defined methods
bool initOpenGL(GLFWwindow** window);
void resizeWindow(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mousePositionCallback(GLFWwindow* window, double xPos, double yPos);
void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void createMesh(GLMesh &mesh);
void destroyMesh(GLMesh &mesh);
void render();
bool createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource, GLuint &programId);
void destroyShaderProgram(GLuint programId);

/* Vertex Shader Source Code*/
const GLchar * vertexShaderSource = GLSL(440,
    layout (location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout (location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

    out vec4 vertexColor; // variable to transfer color data to the fragment shader

    //Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
        vertexColor = color; // references incoming color data
    }
);

/* Fragment Shader Source Code*/
const GLchar * fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

    out vec4 fragmentColor;

    void main()
    {
        fragmentColor = vec4(vertexColor);
    }
);

int main() {

	if (!initOpenGL(&window))
		return EXIT_FAILURE;

	// create the mesh of triangle
	createMesh(mesh);

	// create the shader program
	if (!createShaderProgram(vertexShaderSource, fragmentShaderSource, programId))
		return EXIT_FAILURE;

	// Sets the background color of the window to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// render loop - one frame per interation
	while (!glfwWindowShouldClose(window)) {

		// time between frames
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		// keyboard/mouse inputs
		processInput(window);

		// render this frame
		render();

		glfwPollEvents();
	}

	// de-allocate mesh data
	destroyMesh(mesh); //

	// de-allocate shader program
	destroyShaderProgram(programId);

	exit(EXIT_SUCCESS);
}


// Initialize GLFW, GLEW, and create a window
bool initOpenGL(GLFWwindow** window)
{
	// initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, resizeWindow);
	glfwSetCursorPosCallback(*window, mousePositionCallback);
	glfwSetScrollCallback(*window, mouseScrollCallback);
	glfwSetMouseButtonCallback(*window, mouseButtonCallback);

	// capture mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// initialize glew
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Display GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void resizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// process all input - check glfw for keypresses this frame for camera movement
void processInput(GLFWwindow* window)
{
	// closes the window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// camera movement key bindings
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: callback for camera view whenever the mouse moves
void mousePositionCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse) {
		lastX = xPos;
		lastY = xPos;
		firstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

// glfw: callback for camera zoom whenever the mouse wheel scrolls
void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	// update this method to have the mouse scroll wheel adjust the camera speed

	camera.ProcessMouseScroll(yOffset);
}

// glfw: callback for mouse button events
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}

// create the mesh of triangles
void createMesh(GLMesh& mesh)
{
	// Vertex data
	GLfloat vertices[] = {
		// Vertex Positions		// Colors (r,g,b,a)			// VERTEX DESCRIPTION	// COLOR DESCRIPTION
		 0.0f,  0.0f,  0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		// Vertex 0				Red
		 1.0f,  0.0f,  0.0f,	0.0f, 0.0f, 1.0f, 1.0f,		// Vertex 1				Blue
		 1.0f,  0.0f,  1.0f,	0.0f, 1.0f, 0.0f, 1.0f,		// Vertex 2				Green
		 0.0f,  0.0f,  1.0f,	0.0f, 1.0f, 0.0f, 1.0f,		// Vertex 3				Green
		 0.5f,  1.0f,  0.5f,	1.0f, 1.0f, 0.0f, 1.0f		// Vertex 4				Yellow
	};

	GLushort indices[] = {
		0, 1, 2,	// bottom
		0, 2, 3,	// bottom
		0, 1, 4,	// front side
		1, 2, 4,	// left side
		2, 3, 4,	// right side
		0, 3, 4		// back side
	};

	// number of coordinates and colors (r, g, b, a) per vertex
	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerColor = 4;
	mesh.nIndices = sizeof(vertices) / (sizeof(vertices[0] * (floatsPerVertex * floatsPerColor)));

	// generate VAO
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	// generate two VBOs for vertex data and indices data
	glGenBuffers(2, mesh.vbo);

	// activate VBO for vertex data and send the data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// activate VBO for indice data and send the data to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// number of items per vertex (position & color)
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

	// create vertex attribute pointer
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	// create color attribute pointer
	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);
}

// de-allocate the mesh data
void destroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(2, mesh.vbo);
}

// render a single frame
void render()
{
	// enable z-depth
	glEnable(GL_DEPTH_TEST);

	// clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// scale the object by 2
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	// rotate object by 45 degrees in the x axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// place object at the origin
	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	// generate the model matrix
	glm::mat4 model = translation * rotation * scale;

	// generate camera/view transformation
	glm::mat4 view = camera.GetViewMatrix();

	// generate perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	// set shader program
	glUseProgram(programId);

	// retrieves transform matrices for the shader program
	GLint modelLocation = glGetUniformLocation(programId, "model");
	GLint viewLocation = glGetUniformLocation(programId, "view");
	GLint projectionLocation = glGetUniformLocation(programId, "projection");

	// passes transform matrices for the shader program
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// activate the VBOs in the mesh's VAO
	glBindVertexArray(mesh.vao);

	// draw the triangles
	glDrawElements(GL_TRIANGLES, mesh.nIndices, GL_UNSIGNED_SHORT, NULL);

	// deactivate the VBOs in the mesh's VAO
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events
	glfwSwapBuffers(window);
	glfwPollEvents();
}

// create the color shading function between vertices
bool createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource, GLuint& programId)
{
	int success = 0; // shader program compile error reporting
	char infoLog[512]; // shader program linkage error reporting

	// create a Shader program object.
	programId = glCreateProgram();

	// create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// retrive the shader source code
	glShaderSource(vertexShaderId, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, NULL);


	// compile the vertex shader source code
	glCompileShader(vertexShaderId);

	// check for vertex shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}


	// compile the fragment shader source code
	glCompileShader(fragmentShaderId);

	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}


	// attach compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);


	// link the shader program for use
	glLinkProgram(programId);

	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	
	// use the shader program
	glUseProgram(programId);

	return true;
}

// de-allocate the shader program data
void destroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}
