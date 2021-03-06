/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt
*/
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdlib>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
#include "time.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

#ifdef __WIN32
	// Use dedicated GPU on windows
	extern "C"
	{
	  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	}
#endif

class Application : public EventCallbacks
{

public:
 
	WindowManager * windowManager = nullptr;

	// Our shader program
    std::shared_ptr<Program> prog_wall, prog_mouse, prog_deferred, prog_screen;
 
	// Shape to be used (from obj file)
    shared_ptr<Shape> wall, mouse;
 
	//camera
	camera mycam;

	//texture for sim
	GLuint wall_texture, wall_normal_texture, ghost_texture, starburst_texture;

	// textures for position, color, and normal
	GLuint fb, depth_rb, FBOpos, FBOcol, FBOnorm, fb2, FBOghost, FBOhalo, FBOstarburst;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	GLuint VertexArrayIDBox, VertexBufferIDBox, VertexBufferTex;
    
    double mouse_posX, mouse_posY;

	float uGhostCount = 4;
	float uGhostSpacing = .1f;
	// 2.0
	float uGhostThreshold = -1.0f;
	float uHaloRadius = 0.4f;
	float uHaloThickness = 0.1f;
	float uHaloThreshold = 0.5f;
	float uHaloAspectRatio = 1.0f;
	float uChromaticAberration = 0.05f;
	float uDownsample = 1.0f;
	float uGlobalBrightness = 0.01;
	float uStarburstOffset = 0.5;
	int debug_on = 0;
	int pass = 0;
	float ghost_offset = 0;
	int starburst_on = 1;

	int current_image_file = 0;

	float texture_x_offset = 0.0;

	std::vector<std::string> image_files;
	std::string resourceDirectory;

	void print_menu() {
		cout << "uGhostThreshold = W/S" << endl;
		cout << "uGhostSpacing = I/K" << endl;
		cout << "uGhostCount = UP/DOWN" << endl;
		cout << "uHaloRadius = 1/2" << endl;
		cout << "uHaloThreshold = 3/4" << endl;
		cout << "uHaloAspectRatio = 5/6" << endl;
		cout << "uStarburstOffset = 7/8" << endl;
		cout << "Change Image = Left/Right" << endl;
		cout << endl;
	}

	void print_values() {
		cout << "uGhostThreshold = " << uGhostThreshold << endl;
		cout << "uGhostSpacing = " << uGhostSpacing << endl;
		cout << "uGhostCount = " << uGhostCount << endl;
		cout << "uHaloRadius = " << uHaloRadius << endl;
		cout << "uHaloThreshold = " << uHaloThreshold << endl;
		cout << "uHaloAspectRatio = " << uHaloAspectRatio << endl;
		cout << "uStarburstOffset = " << uStarburstOffset << endl;
	}

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			uGhostThreshold += .1;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			uGhostThreshold -= .1;
		}
		if (key == GLFW_KEY_I && action == GLFW_PRESS)
		{
			uGhostSpacing += .1;
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS)
		{
			uGhostSpacing -= .1;
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			uGhostCount += 1;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			uGhostCount -= .1;
		}
		if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		{
			uHaloRadius += .1;
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		{
			uHaloRadius -= .1;
		}
		if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		{
			uHaloThreshold += .1;
		}
		if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		{
			uHaloThreshold -= .1;
		}
		if (key == GLFW_KEY_5 && action == GLFW_PRESS)
		{
			uHaloAspectRatio += .1;
		}
		if (key == GLFW_KEY_6 && action == GLFW_PRESS)
		{
			uHaloAspectRatio -= .1;
		}
		if (key == GLFW_KEY_7 && action == GLFW_PRESS)
		{
			uStarburstOffset += .1;
		}
		if (key == GLFW_KEY_8 && action == GLFW_PRESS)
		{
			uStarburstOffset -= .1;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			texture_x_offset -= .1;
			//ghost_offset -= .1;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			texture_x_offset += .1;
			//ghost_offset += .1;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			starburst_on++;
			if (starburst_on > 1) {
				starburst_on = 0;
			}
		}
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		{
			debug_on++;
			if (debug_on > 3){
				debug_on = 0;
			}
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			current_image_file -= 1;
			if (current_image_file < 0){
				current_image_file = 0;
			}
			change_texture();
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			current_image_file += 1;
			if (current_image_file > image_files.size()-1){
				current_image_file = image_files.size()-1;
			}
			change_texture();
		}
		if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
		{
			print_values();
		}
		print_menu();
		print_values();
	}

	void init_image_files(std::string resourceDirectory){
		image_files.push_back(resourceDirectory + "/mountains.jpg");
        image_files.push_back(resourceDirectory + "/autumn_scene.jpg");
        image_files.push_back(resourceDirectory + "/grass.jpg");
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		glfwGetCursorPos(window, &mouse_posX, &mouse_posY);
		cout << "Pos X " << mouse_posX << " Pos Y " << mouse_posY << endl;
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//culling:
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		//transparency
		glEnable(GL_BLEND);
		//next function defines how to mix the background color with the transparent pixel in the foreground. 
		//This is the standard:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        // Initialize the GLSL program.
        prog_wall = make_shared<Program>();
        //prog_wall->setVerbose(true);
        prog_wall->setVerbose(false);
        prog_wall->setShaderNames(resourceDirectory + "/wall_vert.glsl", resourceDirectory + "/wall_frag.glsl");

        if (! prog_wall->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
        }

        prog_wall->init();
        prog_wall->addUniform("M");
        prog_wall->addUniform("uScale");
        prog_wall->addUniform("uBias");
        prog_wall->addUniform("txSize");
        prog_wall->addUniform("uSrcLevel");
        prog_wall->addUniform("M");
        prog_wall->addAttribute("vertPos");
        prog_wall->addAttribute("vertNor");
        prog_wall->addAttribute("vertTex");
        
        // Initialize the GLSL program.
        prog_mouse = make_shared<Program>();
        prog_mouse->setVerbose(false);
        prog_mouse->setShaderNames(resourceDirectory + "/mouse_vert.glsl", resourceDirectory + "/mouse_frag.glsl");

        if (! prog_mouse->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
        }

        prog_mouse->init();
        prog_mouse->addUniform("M");
        prog_mouse->addAttribute("vertPos");
        prog_mouse->addAttribute("vertNor");
        prog_mouse->addAttribute("vertTex");

		// Initialize the GLSL program.
		prog_deferred = make_shared<Program>();
		prog_deferred->setVerbose(true);
		prog_deferred->setShaderNames(resourceDirectory + "/deferred_vert.glsl", resourceDirectory + "/deferred_frag.glsl");

		if (!prog_deferred->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}

		prog_deferred->init();
        prog_deferred->addUniform("uGhostCount");
        prog_deferred->addUniform("uGhostSpacing");
		prog_deferred->addUniform("uGhostThreshold");
		prog_deferred->addUniform("uHaloRadius");
		prog_deferred->addUniform("uHaloThickness");
		prog_deferred->addUniform("uHaloThreshold");
		prog_deferred->addUniform("uHaloAspectRatio");
		prog_deferred->addUniform("uChromaticAberration");
		prog_deferred->addUniform("uDownsample");
		prog_deferred->addUniform("uGlobalBrightness");
		prog_deferred->addUniform("uStarburstOffset");
		prog_deferred->addUniform("debug_on");
		prog_deferred->addUniform("pass");
		prog_deferred->addUniform("ghost_offset");
		prog_deferred->addAttribute("vertPos");
		prog_deferred->addAttribute("vertTex");

		// Initialize the GLSL program.
		prog_screen = make_shared<Program>();
		prog_screen->setVerbose(true);
		prog_screen->setShaderNames(resourceDirectory + "/screen_vert.glsl", resourceDirectory + "/screen_frag.glsl");

		if (!prog_screen->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog_screen->addUniform("txSize");
		prog_screen->addUniform("uSrcLevel");
		prog_screen->addUniform("starburst_on");
		prog_screen->addUniform("debug_on");
		prog_screen->addAttribute("vertPos");
		prog_screen->addAttribute("vertTex");
    }

	void change_texture() {
		int width, height, channels;
		char filepath[1000];
		//texture
		string str = image_files[current_image_file];
		if (current_image_file == 0) {
			uGhostThreshold = -0.6;
			uGhostSpacing = 0.9;
			uGhostCount = 5;
			uHaloRadius = 0.3;
			uHaloThreshold = 0.1;
			uHaloAspectRatio = 0.9;
			uStarburstOffset = 0.5;
		}
		else {
			uGhostThreshold = -0.6;
			uGhostSpacing = 0.9;
			uGhostCount = 4;
			uHaloRadius = 0.4;
			uHaloThreshold = 0.6;
			uHaloAspectRatio = 0.7;
			uStarburstOffset = 0.5;
		}
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &wall_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

    void initGeom(const std::string& resourceDirectory)
	{
		// Deferred Stuff
		//init rectangle mesh (2 triangles) for the post processing
		glGenVertexArrays(1, &VertexArrayIDBox);
		glBindVertexArray(VertexArrayIDBox);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDBox);

		GLfloat *rectangle_vertices = new GLfloat[18];
		// front
		int verccount = 0;

		rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = -1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), rectangle_vertices, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferTex);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTex);

		float t = 1. / 100.;
		GLfloat *rectangle_texture_coords = new GLfloat[12];
		int texccount = 0;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 1;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 1;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 1;

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), rectangle_texture_coords, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // Initialize mesh.
        wall = make_shared<Shape>();
        wall->loadMesh(resourceDirectory + "/internet_square.obj");
        wall->resize();
        wall->init();
        
        // Initialize mesh.
        mouse = make_shared<Shape>();
        mouse->loadMesh(resourceDirectory + "/internet_square.obj");
        mouse->resize();
        mouse->init();
        
		int width, height, channels;
		char filepath[1000];

		change_texture();
        
        std::string str = resourceDirectory + "/lvl1normalscombined.jpg";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &wall_normal_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wall_normal_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/ghost_color_gradient.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &ghost_texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ghost_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/starburst.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &starburst_texture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, starburst_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
        
        //[TWOTEXTURES]
        //set the 2 textures to the correct samplers in the fragment shader:
        GLuint Tex1Location = glGetUniformLocation(prog_wall->pid, "tex");
		GLuint Tex2Location = glGetUniformLocation(prog_wall->pid, "tex2");
		GLuint Tex3Location = glGetUniformLocation(prog_wall->pid, "tex3");
		GLuint Tex4Location = glGetUniformLocation(prog_wall->pid, "tex4");

        // Then bind the uniform samplers to texture units:
        glUseProgram(prog_wall->pid);
        glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
		glUniform1i(Tex3Location, 2);
		glUniform1i(Tex4Location, 3);

		glUseProgram(prog_deferred->pid);
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glGenFramebuffers(1, &fb);
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);

		// Deffered Rendering stuff

		// Generate Color Texture
		glGenTextures(1, &FBOcol);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOcol);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);
		
		// Generate Position Texture
		glGenTextures(1, &FBOpos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		// Generate Normal Texture
		glGenTextures(1, &FBOnorm);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOnorm);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);

		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOcol, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBOpos, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, FBOnorm, 0);

		//-------------------------
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?

		int Tex1Loc = glGetUniformLocation(prog_deferred->pid, "col_tex");
		int Tex2Loc = glGetUniformLocation(prog_deferred->pid, "pos_tex");
		int Tex3Loc = glGetUniformLocation(prog_deferred->pid, "norm_tex");

		glUniform1i(Tex1Loc, 0);
		glUniform1i(Tex2Loc, 1);
		glUniform1i(Tex3Loc, 2);

		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good" << endl;
			break;
		default:
			cout << "status framebuffer: bad" << endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Create second FBO
		glUseProgram(prog_screen->pid);
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glGenFramebuffers(1, &fb2);
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, fb2);

		// Deffered Rendering stuff

		// Generate Flare Texture
		glGenTextures(1, &FBOghost);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOghost);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

		glGenTextures(1, &FBOhalo);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOhalo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

		glGenTextures(1, &FBOstarburst);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOstarburst);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);


		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOghost, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBOhalo, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, FBOstarburst, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, FBOcol, 0);

		//-------------------------
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?

		Tex1Loc = glGetUniformLocation(prog_screen->pid, "ghost_tex");
		Tex2Loc = glGetUniformLocation(prog_screen->pid, "halo_tex");
		Tex3Loc = glGetUniformLocation(prog_screen->pid, "starburst_tex");
		int Tex4Loc = glGetUniformLocation(prog_screen->pid, "col_tex");

		glUniform1i(Tex1Loc, 0);
		glUniform1i(Tex2Loc, 1);
		glUniform1i(Tex3Loc, 2);
		glUniform1i(Tex4Loc, 3);

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good" << endl;
			break;
		default:
			cout << "status framebuffer: bad" << endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    
	float map(float x, float in_min, float in_max, float out_min, float out_max)
	{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}

	void render_to_texture() // aka render to framebuffer
	{
		glfwGetCursorPos(windowManager->windowHandle, &mouse_posX, &mouse_posY);

		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, buffers);

		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		glm::mat4 M, T, S, R;
		float pi = 3.14159625;
		float pi_half = pi / 2.;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wall_normal_texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ghost_texture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, starburst_texture);

		prog_wall->bind();

		// WALLS
		T = glm::translate(glm::mat4(1), glm::vec3(texture_x_offset,0,0));
		S = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1));
		R = glm::rotate(glm::mat4(1), (float)pi, glm::vec3(0,0,1));
		M = T * R * S;
		glm::vec4 uScale = glm::vec4(.1);
		glm::vec4 uBias = glm::vec4(.1);
		glm::vec2 txSize = glm::vec2(1920, 1080);
		glUniform4fv(prog_wall->getUniform("uScale"), 1, &uScale.x);
		glUniform4fv(prog_wall->getUniform("uBias"), 1, &uBias.x);
		glUniform2fv(prog_wall->getUniform("txSize"), 1, &txSize.x);
		glUniform1i(prog_wall->getUniform("uSrcLevel"), 1);
		glUniformMatrix4fv(prog_wall->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		wall->draw(prog_wall);

		//done, unbind stuff
		prog_wall->unbind();

		// Save output to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOcol);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOnorm);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void render_deferred()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fb2);
		GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, buffers);
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 M, S, T;
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		prog_deferred->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOcol);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOnorm);
		glBindVertexArray(VertexArrayIDBox);

		glUniform1i(prog_deferred->getUniform("uGhostCount"), uGhostCount);
		glUniform1i(prog_deferred->getUniform("pass"), pass);
		glUniform1f(prog_deferred->getUniform("uGhostSpacing"), uGhostSpacing);
		glUniform1f(prog_deferred->getUniform("uGhostThreshold"), uGhostThreshold);
		glUniform1f(prog_deferred->getUniform("uHaloRadius"), uHaloRadius);
		glUniform1f(prog_deferred->getUniform("uHaloThickness"), uHaloThickness);
		glUniform1f(prog_deferred->getUniform("uHaloThreshold"), uHaloThreshold);
		glUniform1f(prog_deferred->getUniform("uHaloAspectRatio"), uHaloAspectRatio);
		glUniform1f(prog_deferred->getUniform("uChromaticAberration"), uChromaticAberration);
		glUniform1f(prog_deferred->getUniform("uDownsample"), uDownsample);
		glUniform1f(prog_deferred->getUniform("uGlobalBrightness"), uGlobalBrightness);
		glUniform1f(prog_deferred->getUniform("uStarburstOffset"), uStarburstOffset);
		glUniform1f(prog_deferred->getUniform("ghost_offset"), ghost_offset);
		glUniform1i(prog_deferred->getUniform("debug_on"), debug_on); 
		glDrawArrays(GL_TRIANGLES, 0, 6);
		prog_deferred->unbind();
		// Save output to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOghost);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOhalo);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOstarburst);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void render_to_screen()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 M, S, T;
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		prog_screen->bind();
		glm::vec2 txSize = glm::vec2(1920, 1080);
		glUniform2fv(prog_screen->getUniform("txSize"), 1, &txSize.x);
		glUniform1i(prog_screen->getUniform("uSrcLevel"), 1);
		glUniform1i(prog_screen->getUniform("starburst_on"), starburst_on);
		glUniform1i(prog_screen->getUniform("debug_on"), debug_on);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOghost);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOhalo);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOstarburst);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, wall_texture);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		prog_screen->unbind();
	}
};

//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1920, 1920);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->resourceDirectory = resourceDir;

	application->init_image_files(resourceDir);
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		application->render_to_texture();
		application->render_deferred();
		application->render_to_screen();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	windowManager->shutdown();
	cout << endl;
	return 0;
}
