/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt
*/
#include "Shape.h"
#include <iostream>
#include <glad/glad.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"

#include "MatrixStack.h"
#include "WindowManager.h"

#include "camera.h"

#define GLM_ENABLE_EXPERIMENTAL


// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#include "double_algebra.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "OpenVRclass.h"
OpenVRApplication *vrapp = NULL;
float eyeconvergence = 0.1;
float eyedistance = 0.08;

//#define LEFTEYE 0
//#define RIGHTEYE 1
//class camera
//{
//private:
//	vec3 accelangle;
//	float accela = 0.003;
//	float maxaccela = 1.0;
//public:
//	long double velocity = 0;
//	long double lorentz = 1;
//	long double maxvel = 0.02;
//	double_vec_ pos, rot, viewrot;
//	double_vec_ camdir;
//	double_mat_ viewM;
//	double_mat_ trackingM;
//	long double unitsmul_lightyears;
//	long double unitsmul_km;
//	long double unitsmul_lightsecond;
//	int w, a, s, d;
//	int key_i, key_j, key_k, key_l;
//	camera()
//	{
//		//unitsmul_lightyears = GALAXYRADIUS / 50000.0;
//		unitsmul_km = unitsmul_lightyears * 9.461e+12;
//		unitsmul_lightsecond = unitsmul_lightyears / 3.156e+7;
//		//maxvel = unitsmul_lightsecond * MAGIC;
//		key_i = key_j = key_k = key_l = 0;
//		w = a = s = d = 0;
//		pos = rot = viewrot = double_vec_(0, 0, 0);
//		camdir = double_vec_(0, 0, 1);
//		viewM.set_identity();
//		accelangle = vec3(0, 0, 0);
//
//	}
//	void offsetOrientation(float upAngle, float rightAngle) {
//		rot.y += rightAngle;
//		rot.x += upAngle;
//		normalizeAngles();
//	}
//
//	void normalizeAngles() {
//		static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock
//		rot.y = fmodf(rot.y, 360.0f);
//		//fmodf can return negative values, but this will make them all positive
//		if (rot.y < 0.0f)
//			rot.y += 360.0f;
//
//		if (rot.x > MaxVerticalAngle)
//			rot.x = MaxVerticalAngle;
//		else if (rot.x < -MaxVerticalAngle)
//			rot.x = -MaxVerticalAngle;
//	}
//
//	glm::mat4 getR()
//	{
//		glm::mat4 R = glm::rotate(glm::mat4(1), (float)rot.y, glm::vec3(0, 1, 0));
//		return R;
//	}
//
//	glm::mat4 process(double ftime, int eye)
//	{
//
//		long double vdiff = maxvel - velocity;
//		if (w == 1)
//		{
//			velocity += vdiff * 0.9 * ftime;
//		}
//		else if (s == 1)
//		{
//			velocity /= 1.1;
//		}
//		else
//			velocity -= velocity * 0.2 * ftime;
//
//		lorentz = sqrt(1. - pow(velocity, 2) / pow(maxvel, 2));
//
//
//
//
//		float yangle = 0;
//		if (a == 1)
//		{
//			if (accelangle.y < 0)accelangle.y = 0;
//			accelangle.y = min(accelangle.y, maxaccela);
//			accelangle.y += accela * ftime;
//			yangle += accelangle.y;
//		}
//		else if (d == 1)
//		{
//			if (accelangle.y > 0)accelangle.y = 0;
//			accelangle.y = max(accelangle.y, -maxaccela);
//			accelangle.y -= accela * ftime;
//			yangle += accelangle.y;
//		}
//		else
//			accelangle.y /= 2;
//		rot.y += yangle;
//
//
//
//		yangle = 0;
//		if (key_j == 1)
//			yangle = 3 * ftime;
//		else if (key_l == 1)
//			yangle = -3 * ftime;
//		viewrot.y += yangle;
//		float xangle = 0;
//		if (key_i == 1)
//			xangle = 3 * ftime;
//		else if (key_k == 1)
//			xangle = -3 * ftime;
//		viewrot.x += xangle;
//
//		//glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
//		double_mat_ dR;
//		dR.set_rotation_matrix_y(rot.y);
//		double_vec_ dir = double_vec_(0, 0, velocity / lorentz);
//		dir = dR * dir;
//		camdir = dR * double_vec_(0, 0, 1);
//		dir.x *= -1;
//		pos += dir;
//		double_mat_ T;
//		T.set_transform_matrix(pos);
//		double_mat_ M = T * dR;
//		mat4 glmM = M.convert_glm();
//
//
//		return glmM;
//	}
//};
//
//camera mycam;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog,prog2,prog3;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<Shape> skysphere;
	shared_ptr<Shape> laser;
	
	//camera
	camera mycam;

	//XWing positions

	float XWingZ, left, right = 0.0;
	float XWingRoll = 0.0;
	float XWingYaw = 0.0;

	bool XWingHasShot = false;
	bool shotFirstFrame = false;
	float distanceFromXWING = 0.0;
	glm::vec3 translationCopy;

	//texture for sim
	GLuint Texture, TextureDeathStar;
	GLuint Texture2;
	GLuint TexturePlane;

	GLuint VertexArrayIDBox, VertexBufferIDBox, VertexBufferTex;
	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	float xWingZ = 0.f;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		float zAmount = .05;
		update();
		std::cout << "key is : " << key << std::endl;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
			xWingZ += .1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
		{
			//mycam.q = 0;
			XWingRoll -= .05;
		}
		if (key == GLFW_KEY_E && action != GLFW_RELEASE)
		{
			//mycam.e = 0;
			XWingRoll += .05;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		{
			// UNCOMMENT
			//mycam.z = 1;
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
		{
			// UNCOMMENT
			//mycam.z = 0;
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS)
		{
			// UNCOMMENT
			//mycam.x = 1;
		}
		if (key == GLFW_KEY_X && action == GLFW_RELEASE)
		{
			// UNCOMMENT
			//mycam.x = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
			xWingZ -= .1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE)
		{
			//mycam.space = 0;
			XWingZ += zAmount;
		}
		if (key == GLFW_KEY_C && action != GLFW_RELEASE)
		{
			//mycam.c = 0;
			XWingZ -= zAmount;
		}
		if (key == GLFW_KEY_A && action != GLFW_RELEASE)
		{
			//mycam.a = 0;
			XWingYaw -= .05;
		}
		if (key == GLFW_KEY_D && action != GLFW_RELEASE)
		{
			//mycam.d = 0;
			XWingYaw += .05;
		}

		if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS)
		{
			XWingHasShot = true;
			shotFirstFrame = true;
		}

		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		{
			// UNCOMMENT
			//mycam.reset = 1;

			XWingZ = 0;
			XWingRoll = 0;
		}
		
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
		}
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
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vertsky.glsl", resourceDirectory + "/simple_fragsky.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		
		
		
		//second prog for skysphere

		// Initialize the GLSL program.
		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (!prog2->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addUniform("offset");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertNor");
		prog2->addAttribute("vertTex");

		//Third prog for lasers

		// Initialize the GLSL program.
		prog3 = make_shared<Program>();
		prog3->setVerbose(true);
		prog3->setShaderNames(resourceDirectory + "/laser_vert.glsl", resourceDirectory + "/laser_frag.glsl");
		if (!prog3->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog3->init();
		prog3->addUniform("P");
		prog3->addUniform("V");
		prog3->addUniform("M");
		prog3->addUniform("offset");
		prog3->addAttribute("vertPos");
		prog3->addAttribute("vertNor");
		prog3->addAttribute("vertTex");


	}

	void initGeom(const std::string& resourceDirectory)
	{

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayIDBox);
		glBindVertexArray(VertexArrayIDBox);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDBox);

		GLfloat *ver= new GLfloat[10000 * 18];
			// front
		int verc = 0;
			for (int i = 0; i < 100; i++) {
				for (int j = 0; j < 100; j++) {
					ver[verc++] = 0.0 + j, ver[verc++] = 0.0 + i, ver[verc++] = 0.0;
					ver[verc++] = 1.0 + j, ver[verc++] = 0.0 + i, ver[verc++] = 0.0;
					ver[verc++] = 0.0 + j, ver[verc++] = 1.0 + i, ver[verc++] = 0.0;
					ver[verc++] = 1.0 + j, ver[verc++] = 0.0 + i, ver[verc++] = 0.0;
					ver[verc++] = 1.0 + j, ver[verc++] = 1.0 + i, ver[verc++] = 0.0;
					ver[verc++] = 0.0 + j, ver[verc++] = 1.0 + i, ver[verc++] = 0.0;
				}

		}
			
			
		
		
		
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 10000*18*sizeof(float), ver, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferTex);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTex);

		float t = 1. / 100.;
		GLfloat *cube_tex=new GLfloat[10000 * 12];
		int texc = 0;
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++) {
				int u = 0;
				cube_tex[texc++] = (GLfloat)j*t, cube_tex[texc++] = (GLfloat)i*t;
				cube_tex[texc++] = (GLfloat)(j + 1)*t, cube_tex[texc++] = (GLfloat)i*t;
				cube_tex[texc++] = (GLfloat)j*t, cube_tex[texc++] = (GLfloat)(i + 1)*t;
				cube_tex[texc++] = (GLfloat)(j + 1)*t, cube_tex[texc++] = 0.0 + (GLfloat)i*t;
				cube_tex[texc++] = (GLfloat)(j + 1)*t, cube_tex[texc++] = (GLfloat)(i + 1)*t;
				cube_tex[texc++] = (GLfloat)j*t, cube_tex[texc++] = (GLfloat)(i + 1)*t;
			}
		}

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 10000*12*sizeof(float), cube_tex, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		// Initialize mesh.
		shape = make_shared<Shape>();
		string path = resourceDirectory + '/';
		shape->loadMesh(resourceDirectory + "/x-wing.obj", &path, stbi_load);
		shape->resize();
		shape->init();

		// Initialize mesh.
		skysphere = make_shared<Shape>();
		skysphere->loadMesh(resourceDirectory + "/sphere.obj");
		skysphere->resize();
		skysphere->init();

		// Initialize mesh.
		laser = make_shared<Shape>();
		laser->loadMesh(resourceDirectory + "/sphere.obj");
		laser->resize();
		laser->init();
		
		int width, height, channels;
		char filepath[1000];

		//texture sky
		string str = resourceDirectory + "/deathStarSurface.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureDeathStar);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureDeathStar);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 1
		str = resourceDirectory + "/testHeightmap2.png";
		strcpy(filepath, str.c_str());		
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/space.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 3
		str = resourceDirectory + "/red.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TexturePlane);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TexturePlane);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(prog2->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(prog2->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog2->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(prog3->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(prog3->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog3->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		

	}

	void update() {
		//rotate camera based on mouse movement
		const float mouseSensitivity = 0.01f;
		double mouseX, mouseY;
		glfwGetCursorPos(windowManager->getHandle(), &mouseX, &mouseY);
		mycam.offsetOrientation(mouseSensitivity * (float)mouseY, mouseSensitivity * (float)mouseX);
		glfwSetCursorPos(windowManager->getHandle(), 0, 0); //reset the mouse, so it doesn't go out of the window
	}

	void render(int width, int height, glm::mat4 VRheadmatrix)
	{
		//mycam.cameraPos = VRheadmatrix;
		update();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Get current frame buffer size.
		int width1, height1;
		glfwGetFramebufferSize(windowManager->getHandle(), &width1, &height1);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();
		P->pushMatrix();
		P->perspective(70., width, height, 0.1, 1000.0f);
		glm::mat4 M, T, R, S;

		//glm:mat4 V = mycam.process();

		glm:mat4 V = VRheadmatrix;

		//V[3] = glm::vec4(mycam.pos,1);



		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw mesh using GLSL
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		glDisable(GL_DEPTH_TEST);
		glFrontFace(GL_CW);
		float pih = 3.1415926 / 2.0;
		R = glm::rotate(glm::mat4(1), pih, glm::vec3(1, 0, 0));
		T = glm::translate(glm::mat4(1), glm::vec3(0, -mycam.pos.y, -mycam.pos.z));
		M = T * R;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		skysphere->draw(prog, false);
		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CCW);


		//Draw XWING
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexturePlane);
		R = glm::rotate(glm::mat4(1), 110.f, glm::vec3(1, 0, 0));
		R = glm::rotate(R, -3.145f, glm::vec3(0, 0, 1));
		R = glm::rotate(R, XWingRoll, glm::vec3(0, 0, 1));
		glm::vec3 translation = glm::vec3(-XWingYaw, -XWingZ + 1, xWingZ + 5);
		T = glm::translate(glm::mat4(1), -translation);
		M = T * R;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		shape->draw(prog, false);
		prog->unbind();

		prog2->bind();
		glBindTexture(GL_TEXTURE_2D, Texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureDeathStar);
		glm::mat4 RV = mycam.getR();
		T = glm::translate(glm::mat4(1), glm::vec3(-50, -2, 50));
		RV = glm::transpose(RV);
		pih = -3.1415926 / 2.0;
		R = glm::rotate(glm::mat4(1), pih, glm::vec3(1, 0, 0));
		M = T * R;
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDBox);
		glm::vec2 offset(0, 0);
		offset.x = floor(-mycam.pos.x);
		offset.y = floor(mycam.pos.z);

		glUniform2fv(prog2->getUniform("offset"), 1, &offset[0]);
		glDrawArrays(GL_TRIANGLES, 0, 10000 * 6);


		prog2->unbind();

		prog3->bind();
		if (XWingHasShot) {
			if (shotFirstFrame) {
				translationCopy = translation;
				shotFirstFrame = false;
			}
			glUniformMatrix4fv(prog3->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog3->getUniform("V"), 1, GL_FALSE, &V[0][0]);

			float pih = 3.1415926 / 2.0;
			R = glm::rotate(glm::mat4(1), pih, glm::vec3(1, 0, 0));
			if (distanceFromXWING < 20) {
				distanceFromXWING += 0.5;
				T = glm::translate(glm::mat4(1), glm::vec3(-translationCopy.x, -translationCopy.y, -translationCopy.z - distanceFromXWING - 1));
				S = glm::scale(glm::mat4(1), vec3(.1, .1, .1));
				M = T * S * R;
				glUniformMatrix4fv(prog3->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Texture2);
				laser->draw(prog3, false);
			}
			else {
				XWingHasShot = false;
				distanceFromXWING = 0.0;
			}
		}



		prog3->unbind();
	}


	//void render_p(int width, int height, glm::mat4 VRheadmatrix)
	//{
	//	mycam.trackingM = VRheadmatrix;
	//	static int framecount = 0;
	//	framecount++;
	//	double frametime = 0;
	//	static double totaltime = 0;

	//	if (framecount % 2 == 1)
	//		frametime = get_last_elapsed_time();

	//	totaltime += frametime;
	//	// Get current frame buffer size.
	//	// Create the matrix stacks - please leave these alone for now

	//	glm::mat4 V, M, P, Vdir; //View, Model and Perspective matrix
	//	V = mycam.process(frametime, LEFTEYE);

	//	Vdir = V;
	//	//Vdir = mycam.viewM.convert_glm();
	//	Vdir = VRheadmatrix;
	//	mat4 Vi = glm::transpose(V);
	//	Vi[0][3] = 0;
	//	Vi[1][3] = 0;
	//	Vi[2][3] = 0;
	//	//Vdir = transpose(Vdir);
	//	M = glm::mat4(1);
	//	P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 100000.0f); //so much type casting... GLM metods are quite funny ones
	//	float sangle = 3.1415926 / 2.;

	//	vec3 camdir = mycam.camdir.convertGLM();
	//	vec3 campos = mycam.pos.convertGLM();


	//	//animation with the model matrix:
	//	static float w = 0.0;
	//	w += 1.0 * frametime;//rotation angle
	//	float trans = 0;// sin(t) * 2;
	//	glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
	//	float angle = -3.1415926 / 2.0;
	//	glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
	//	glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -STARTTRANSZ));
	//	glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

	//	M = TransZ * RotateY * RotateX * S;

	//	// Draw the box using GLSL.
	//	prog->bind();
	//	//send the matrices to the shaders
	//	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
	//	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
	//	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
	//	glBindVertexArray(VertexArrayID);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, Texture);
	//	M = TransZ;
	//	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
	//	glUniform1f(prog->getUniform("lorentz"), mycam.lorentz);
	//	glUniform3fv(prog->getUniform("campos"), 1, &campos.x);
	//	glUniform3fv(prog->getUniform("camdir"), 1, &camdir.x);
	//	glUniformMatrix4fv(prog->getUniform("Vdir"), 1, GL_FALSE, &Vdir[0][0]);
	//	glDisable(GL_DEPTH_TEST);
	//	glDrawArrays(GL_POINTS, 0, STARSCOUNT);
	//	glEnable(GL_DEPTH_TEST);
	//	glBindVertexArray(0);
	//	prog->unbind();
	//	// Draw the circle using GLSL.
	//	pcircle->bind();
	//	float scaldir = 100.0;
	//	vec3 pos = vec3(-mycam.pos.x, -mycam.pos.y, -mycam.pos.z) - scaldir * vec3(-mycam.camdir.x, mycam.camdir.y, mycam.camdir.z);
	//	mat4 Tcstart = translate(mat4(1), pos);
	//	M = Tcstart;
	//	glUniformMatrix4fv(pcircle->getUniform("P"), 1, GL_FALSE, &P[0][0]);
	//	glUniformMatrix4fv(pcircle->getUniform("V"), 1, GL_FALSE, &V[0][0]);
	//	glUniformMatrix4fv(pcircle->getUniform("Vdir"), 1, GL_FALSE, &Vdir[0][0]);
	//	glBindVertexArray(vaocircle);
	//	glDisable(GL_DEPTH_TEST);
	//	for (int ii = 0; ii < 4; ii++)
	//	{
	//		glm::mat4 Sc = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f*ii, 0.2f*ii, 0.2f*ii));
	//		mat4 Res = M * Sc*Vi;
	//		glUniformMatrix4fv(pcircle->getUniform("M"), 1, GL_FALSE, &Res[0][0]);
	//		glDrawArrays(GL_LINE_STRIP, 0, 100);
	//	}

	//	glEnable(GL_DEPTH_TEST);
	//	glBindVertexArray(0);
	//	pcircle->unbind();

	//	pobj->bind();
	//	//send the matrices to the shaders
	//	TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -STARTTRANSZ));
	//	S = glm::scale(glm::mat4(1.0f), glm::vec3(12.0f, 13.0f, 4 * 10.0f));
	//	M = TransZ * S;
	//	//M = mat4(1);
	//	glUniformMatrix4fv(pobj->getUniform("P"), 1, GL_FALSE, &P[0][0]);
	//	glUniformMatrix4fv(pobj->getUniform("V"), 1, GL_FALSE, &V[0][0]);
	//	glUniformMatrix4fv(pobj->getUniform("M"), 1, GL_FALSE, &M[0][0]);
	//	glUniform1f(pobj->getUniform("lorentz"), mycam.lorentz);
	//	glUniform3fv(pobj->getUniform("campos"), 1, &campos.x);
	//	glUniform3fv(pobj->getUniform("camdir"), 1, &camdir.x);
	//	glUniformMatrix4fv(pobj->getUniform("Vdir"), 1, GL_FALSE, &Vdir[0][0]);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, Texture2);
	//	glDisable(GL_DEPTH_TEST);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//	//shape->draw(pobj, FALSE);
	//	vec3 addcolor;
	//	addcolor = vec3(1, 1, 0);
	//	glUniform3fv(pobj->getUniform("addcolor"), 1, &addcolor.x);
	//	glBindVertexArray(vaobox);
	//	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (const void *)0);


	//	S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	//	mat4 S2 = glm::scale(glm::mat4(1.0f), glm::vec3(10.5f, 10.5f, 1.0f));
	//	mat4 Tl;
	//	addcolor = vec3(1, 1, 1);
	//	glUniform3fv(pobj->getUniform("addcolor"), 1, &addcolor.x);
	//	for (int ii = 0; ii < 40; ii++)
	//	{

	//		Tl = translate(mat4(1), vec3(0, 0, 100 - 5 * ii));
	//		if (ii % 2 == 0)
	//			M = Tl * TransZ*S;
	//		//	else
	//		//	M = Tl*TransZ*S2;
	//		glUniformMatrix4fv(pobj->getUniform("M"), 1, GL_FALSE, &M[0][0]);
	//		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (const void *)0);
	//	}
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//	glEnable(GL_DEPTH_TEST);
	//	pobj->unbind();
	//}
};


Application *application = new Application();
void renderfct(int w, int h, glm::mat4 VRheadmatrix)
{
	//application->render_p(w, h, VRheadmatrix);
	application->render(w, h, VRheadmatrix);
}


//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}


	//Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	//WindowManager *windowManager = new WindowManager();
	//windowManager->init(1920, 1080);
	//windowManager->setEventCallbacks(application);
	//application->windowManager = windowManager;


	WindowManager * windowManager = new WindowManager();


	vrapp = new OpenVRApplication();

	windowManager->init(vrapp->get_render_width(), vrapp->get_render_height());

	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
	may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);
	vrapp->init_buffers(resourceDir);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	//application->init(resourceDir);
	//application->initGeom(resourceDir);

	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		//application->render();

		vrapp->render_to_VR(renderfct);
		vrapp->render_to_screen(1);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
