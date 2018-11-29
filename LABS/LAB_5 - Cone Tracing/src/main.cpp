/* 
shadow mapping example for CSC 572 - Cal Poly San Luis Obispo
written by Reed Garmsen and Christian Eckhardt
based on the CSC 471 templates by Ian Dunn and Zoe Wood.
*/
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

#include "sound.h"
music_ music;

// Change this if you want a higher/lower resolution shadow map (affects performance!).
#define SHADOW_DIM 4096
#define VOXELSIZE 256 //512*512*512*4*2 = (1073741824) = 1GB , 256*256*256*4*1 =67108864 (67MB)
// Simple structure to represent a light in the scene.
struct Light
{
	vec3 position;
	vec3 direction;
	vec3 color;
};
class Mouse
	{
	private:
		bool mousemove = false;
	public:
		bool is_mousemove() { return mousemove; }
		void swap_mousemove(GLFWwindow *window)
			{
			if (!mousemove)
				{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				double dcurrentx, dcurrenty;
				glfwGetCursorPos(window, &dcurrentx, &dcurrenty);
				holdx = dcurrentx;
				holdy = dcurrenty;
				}
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mousemove = !mousemove;
			}
		
		int holdx, holdy;
		int currentx, currenty;
		//void set_current(bool )
		Mouse()	{}
		void process(GLFWwindow *window, vec3 *camerarotation)
			{
			if (!mousemove) return;
			double dcurrentx, dcurrenty;
			glfwGetCursorPos(window, &dcurrentx, &dcurrenty);
			currentx = dcurrentx;
			currenty = dcurrenty;
			vec2 diff = vec2(holdx - currentx, holdy - currenty);
			glfwSetCursorPos(window, (double)holdx, (double)holdy);
			*camerarotation -= (float)0.005*vec3(diff.y, diff.x, 0);			
			}

	};

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, prog2, shadowProg, voxprog;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape, sponza;
	//Mouse
	Mouse mouse;

	//camera
	camera mycam;

	//texture for sim
	GLuint TextureEarth, TextureEarthNorm, TextureWall;
	GLuint TextureMoon, TextureMoonNorm, FBOtex, FBOnorm, FBOpos, FBOtan, FBOmain, depth_rb;
	GLuint FBOvoxelize, FBOvoxelizeTex, FBOvoxelizeDepth;
	//3D texture
	GLuint voxeltexture[7], voxeltexture_r, voxeltexture_g, voxeltexture_b, voxeltexture_c;//float texture mimaps and unsigned int 16 bit for reg, gree, blue, and pixel count
	GLuint CSmipmap, CSaver;

	GLuint FBOtex_shadowMapDepth, fb_shadowMap;
	glm::mat4 M_Earth;
	glm::mat4 M_Moon;
	glm::mat4 M_Sponza;
	const vec3 earth_pos = glm::vec3(0, -3.2, -5);
	const vec3 origin = glm::vec3(0, 0, 0);
	const vec3 moon_pos_offset = glm::vec3(-0.7, 0, 0.7);
	int key_j = 0, key_i = 0, key_k = 0, key_l = 0;
	int key_n = 0,key_m = 0;
	
	GLuint VertexArrayIDBox, VertexBufferIDBox, VertexBufferTex, VertexBufferIDNorm;
	
	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	Light primaryLight;


	//toggles
	bool show_shadowmap = false;
	bool use_integer_textures_against_flickering = true;

	float coneHalfAngle = 0.571239; //27 degree

	float steps_for_tracing = 1;
	float cut_off_alpha = 1;
	float reflective_cone_angle = coneHalfAngle / 8;
	float ambient_factor;
	float shadow_factor;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_Y && action == GLFW_RELEASE)
		{
			show_shadowmap = !show_shadowmap;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
			{
			use_integer_textures_against_flickering = !use_integer_textures_against_flickering;
			}
		
		if (key == GLFW_KEY_I && action == GLFW_PRESS)		key_i = 1;
		if (key == GLFW_KEY_I && action == GLFW_RELEASE)	key_i = 0;
		if (key == GLFW_KEY_J && action == GLFW_PRESS)		key_j = 1;
		if (key == GLFW_KEY_J && action == GLFW_RELEASE)	key_j = 0;
		if (key == GLFW_KEY_K && action == GLFW_PRESS)		key_k = 1;
		if (key == GLFW_KEY_K && action == GLFW_RELEASE)	key_k = 0;
		if (key == GLFW_KEY_L && action == GLFW_PRESS)		key_l = 1;
		if (key == GLFW_KEY_L && action == GLFW_RELEASE)	key_l = 0;
		if (key == GLFW_KEY_N && action == GLFW_PRESS)		key_n = 1;
		if (key == GLFW_KEY_N && action == GLFW_RELEASE)	key_n = 0;
		if (key == GLFW_KEY_M && action == GLFW_PRESS)		key_m = 1;
		if (key == GLFW_KEY_M && action == GLFW_RELEASE)	key_m = 0;

		if (key == GLFW_KEY_1 && action == GLFW_PRESS)		steps_for_tracing += 1;
		if (key == GLFW_KEY_2 && action == GLFW_PRESS)		steps_for_tracing -= 1;
		if (key == GLFW_KEY_3 && action == GLFW_PRESS)		cut_off_alpha += .1;
		if (key == GLFW_KEY_4 && action == GLFW_PRESS)		cut_off_alpha -= .1;
		if (key == GLFW_KEY_5 && action == GLFW_PRESS)		reflective_cone_angle += .1;
		if (key == GLFW_KEY_6 && action == GLFW_PRESS)		reflective_cone_angle -= .1;
		if (key == GLFW_KEY_7 && action == GLFW_PRESS)		ambient_factor += .1;
		if (key == GLFW_KEY_8 && action == GLFW_PRESS)		ambient_factor -= .1;
		if (key == GLFW_KEY_9 && action == GLFW_PRESS)		shadow_factor += .1;
		if (key == GLFW_KEY_0 && action == GLFW_PRESS)		shadow_factor -= .1;

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
			{
			mouse.swap_mousemove(windowManager->getHandle());
			}
		
	}
	GLuint init_computeshader(string file)
		{
		//load the compute shader
		std::string ShaderString = readFileAsString(file);
		const char *shader = ShaderString.c_str();
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(computeShader, 1, &shader, nullptr);

		GLint rc;
		CHECKED_GL_CALL(glCompileShader(computeShader));
		CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rc));
		if (!rc)	//error compiling the shader file
			{
			GLSL::printShaderInfoLog(computeShader);
			std::cout << "Error compiling fragment shader " << std::endl;
			return false;
			}

		GLuint computeProgram = glCreateProgram();
		glAttachShader(computeProgram, computeShader);
		glLinkProgram(computeProgram);
		glUseProgram(computeProgram);
		CHECKED_GL_CALL(glUseProgram(0));
		return computeProgram;
		}
	//-------------------------------------------
	//voxeltexture = make3DTexture(VOXELSIZE, VOXELSIZE, VOXELSIZE, 1, GL_RGBA16F, GL_NEAREST, GL_NEAREST);
		
	GLuint make3DTexture(GLsizei width, GLsizei height, GLsizei depth, GLsizei levels, GLenum internalFormat, GLint minFilter, GLint magFilter) 
		{
		GLuint handle;
		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_3D, handle);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexStorage3D(GL_TEXTURE_3D, levels, internalFormat, width, height, depth);
		if (internalFormat == GL_R32UI) {
			glClearTexImage(handle, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
			}
		else {
			glClearTexImage(handle, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
			}

		if (levels > 1) {
			glGenerateMipmap(GL_TEXTURE_3D);
			}

		glBindTexture(GL_TEXTURE_3D, 0);

		return handle;
		}
	//-------------------------------------------
	void init_screen_texture_fbo()
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		

		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, FBOnorm);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, FBOtan);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		//-------------------------
		glGenFramebuffers(1, &FBOmain);
		glBindFramebuffer(GL_FRAMEBUFFER, FBOmain);
		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBOnorm, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, FBOpos, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, FBOtan, 0);
		//-------------------------
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good" << std::endl;
			break;
		default:
			cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//-------------------------------------------
	void init_voxelize_texture_fbo()
		{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		glBindTexture(GL_TEXTURE_2D, FBOvoxelizeTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//NULL means reserve texture memory, but texels are undefined
		//**** Tell OpenGL to reserve level 0
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//You must reserve memory for other mipmaps levels as well either by making a series of calls to
		//glTexImage2D or use glGenerateMipmapEXT(GL_TEXTURE_2D).
		//Here, we'll use :
		glGenerateMipmap(GL_TEXTURE_2D);
		//-------------------------
		glGenFramebuffers(1, &FBOvoxelize);
		glBindFramebuffer(GL_FRAMEBUFFER, FBOvoxelize);
		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOvoxelizeTex, 0);
		//-------------------------
		glGenRenderbuffers(1, &FBOvoxelizeDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, FBOvoxelizeDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FBOvoxelizeDepth);
		//-------------------------
		//Does the GPU support current FBO configuration?
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
			{
			case GL_FRAMEBUFFER_COMPLETE:
				cout << "status framebuffer: good" << std::endl;
				break;
			default:
				cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
			}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	//-------------------------------------------
	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);

		// Super hacky resize support (changing aspect ratio doesn't scale right), but at least stuff doesn't vanish.
		init_screen_texture_fbo();
	}
	//-------------------------------------------
	void init(const std::string& resourceDirectory)
	{
		// Flip image to make tex coords line up with image correctly (due to OpenGL vs DirectX differences).
		stbi_set_flip_vertically_on_load(TRUE);

		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		/*glDepthFunc(GL_EQUAL);
		glDepthMask(GL_FALSE);*/

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
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
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
		prog->addAttribute("vertTan");
		prog->addAttribute("vertBinorm");

		// Initialize the GLSL program.
		voxprog = make_shared<Program>();
		voxprog->setVerbose(true);
		voxprog->setShaderNames(resourceDirectory + "/vox_vert.glsl", resourceDirectory + "/vox_frag.glsl");
		if (!voxprog->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
			}
		voxprog->init();
		voxprog->addUniform("P");
		voxprog->addUniform("V");
		voxprog->addUniform("lightSpace");
		voxprog->addUniform("M");
		voxprog->addUniform("campos");
		voxprog->addUniform("lightpos");
		voxprog->addUniform("lightdir");
		voxprog->addUniform("integer_textures_on");		
		voxprog->addAttribute("vertPos");
		voxprog->addAttribute("vertNor");
		voxprog->addAttribute("vertTex");
		voxprog->addAttribute("vertTan");
		voxprog->addAttribute("vertBinorm");


		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/frag_nolight.glsl");
		if (!prog2->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			//exit(1);
		}
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertTex");
		prog2->addUniform("manualmipmaplevel");
		prog2->addUniform("lightSpace");
		prog2->addUniform("campos");
		prog2->addUniform("lightpos");
		prog2->addUniform("lightdir");
		prog2->addUniform("steps_for_tracing");
		prog2->addUniform("cut_off_alpha");
		prog2->addUniform("reflective_cone_angle");
		prog2->addUniform("ambient_factor");
		prog2->addUniform("shadow_factor");
		
		// Initialize the Shadow Map shader program.
		shadowProg = make_shared<Program>();
		shadowProg->setVerbose(true);
		shadowProg->setShaderNames(resourceDirectory + "/shadow_vert.glsl", resourceDirectory + "/shadow_frag.glsl");
		if (!shadowProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		shadowProg->init();
		shadowProg->addUniform("P");
		shadowProg->addUniform("V");
		shadowProg->addUniform("M");
		shadowProg->addAttribute("vertPos");

		CSmipmap = init_computeshader(resourceDirectory + "/compute_mipmap.glsl");
		CSaver = init_computeshader(resourceDirectory + "/compute_average.glsl");
	}

	void initGeom(const std::string& resourceDirectory)
	{
		sponza = make_shared<Shape>();
		string sponzamtl = resourceDirectory + "/sponza/";
		sponza->loadMesh(resourceDirectory + "/sponza/sponza.obj", &sponzamtl, stbi_load);
		sponza->resize();
		sponza->calc_SxT();
		sponza->init();

		// Initialize light structures.
		primaryLight.position = vec3(2.0f, 20.0f, 2.0f);
		primaryLight.direction = normalize(earth_pos - primaryLight.position);
		primaryLight.color = vec3(1.0f, 1.0f, 1.0f);

		//init rectangle mesh (2 triangles) for the post processing
		glGenVertexArrays(1, &VertexArrayIDBox);
		glBindVertexArray(VertexArrayIDBox);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDBox);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDBox);
		GLfloat *rectangle_positions = new GLfloat[18];
		int verccount = 0;
		rectangle_positions[verccount++] = 0.0, rectangle_positions[verccount++] = 0.0, rectangle_positions[verccount++] = 0.0;
		rectangle_positions[verccount++] = 1.0, rectangle_positions[verccount++] = 0.0, rectangle_positions[verccount++] = 0.0;
		rectangle_positions[verccount++] = 0.0, rectangle_positions[verccount++] = 1.0, rectangle_positions[verccount++] = 0.0;
		rectangle_positions[verccount++] = 1.0, rectangle_positions[verccount++] = 0.0, rectangle_positions[verccount++] = 0.0;
		rectangle_positions[verccount++] = 1.0, rectangle_positions[verccount++] = 1.0, rectangle_positions[verccount++] = 0.0;
		rectangle_positions[verccount++] = 0.0, rectangle_positions[verccount++] = 1.0, rectangle_positions[verccount++] = 0.0;
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), rectangle_positions, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//normals
		glGenBuffers(1, &VertexBufferIDNorm);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDNorm);
		GLfloat *rectangle_normals = new GLfloat[18];
		verccount = 0;
		rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 1.0;
		rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 1.0;
		rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 1.0;
		rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 1.0;
		rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 1.0;
		rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 0.0, rectangle_normals[verccount++] = 1.0;
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), rectangle_normals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//texture coords
		glGenBuffers(1, &VertexBufferTex);
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
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), rectangle_texture_coords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize(); 
		shape->calc_SxT();
		shape->init();

		// Attach Shadow Map depth texture to Shadow Map FBO
		{
			glGenFramebuffers(1, &fb_shadowMap);
			glBindFramebuffer(GL_FRAMEBUFFER, fb_shadowMap);

			glGenTextures(1, &FBOtex_shadowMapDepth);
			//glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_DIM, SHADOW_DIM, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(vec3(1.0)));

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBOtex_shadowMapDepth, 0);

			// We don't want the draw result for a shadow map!
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			GLenum status;
			status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			switch (status)
			{
			case GL_FRAMEBUFFER_COMPLETE:
				cout << "status framebuffer: good" << std::endl;
				break;
			default:
				cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		int width, height, channels;
		char filepath[1000];

		//texture earth diffuse
		string str = resourceDirectory + "/earth.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureEarth);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureEarth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		str = resourceDirectory + "/earthn.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureEarthNorm);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureEarthNorm);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture moon
		str = resourceDirectory + "/moon.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureMoon);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMoon);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		str = resourceDirectory + "/moonn.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureMoonNorm);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMoonNorm);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture wall
		str = resourceDirectory + "/wall.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureWall);
		glBindTexture(GL_TEXTURE_2D, TextureWall);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint TexLocation = glGetUniformLocation(prog->pid, "tex"); //tex sampler in the fragment shader
		GLuint ShadowTexLocation = glGetUniformLocation(prog->pid, "shadowMapTex");
		GLuint NormalMapLoc = glGetUniformLocation(prog->pid, "normalMap");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(TexLocation, 0);
		glUniform1i(NormalMapLoc, 1);
		glUniform1i(ShadowTexLocation, 2);
		

		TexLocation = glGetUniformLocation(voxprog->pid, "tex"); //tex sampler in the fragment shader
		ShadowTexLocation = glGetUniformLocation(voxprog->pid, "shadowMapTex");
		glUseProgram(voxprog->pid);
		glUniform1i(TexLocation, 0);
		glUniform1i(ShadowTexLocation, 2);

		GLuint Tex0Location = glGetUniformLocation(prog2->pid, "tex"); //tex sampler in the fragment shader
		GLuint Tex1Location = glGetUniformLocation(prog2->pid, "texnorm");
		GLuint Tex2Location = glGetUniformLocation(prog2->pid, "texpos");
		GLuint Tex3Location = glGetUniformLocation(prog2->pid, "textan");
		GLuint Tex4Location = glGetUniformLocation(prog2->pid, "shadowMapTex");
		glUseProgram(prog2->pid);
		glUniform1i(Tex0Location, 0);
		glUniform1i(Tex1Location, 1);
		glUniform1i(Tex2Location, 2);
		glUniform1i(Tex3Location, 3);
		glUniform1i(Tex4Location, 4);

		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &FBOtex);
		glGenTextures(1, &FBOnorm);
		glGenTextures(1, &FBOpos);
		glGenTextures(1, &FBOtan);
		init_screen_texture_fbo();

		glGenTextures(1, &FBOvoxelizeTex);
		init_voxelize_texture_fbo();

		
		unsigned int size = pow(2, 8);
		size = pow(2, 8); voxeltexture[0] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		voxeltexture_r = make3DTexture(size, size, size, 1, GL_R32UI, GL_NEAREST, GL_NEAREST);
		voxeltexture_g = make3DTexture(size, size, size, 1, GL_R32UI, GL_NEAREST, GL_NEAREST);
		voxeltexture_b = make3DTexture(size, size, size, 1, GL_R32UI, GL_NEAREST, GL_NEAREST);
		voxeltexture_c = make3DTexture(size, size, size, 1, GL_R32UI, GL_NEAREST, GL_NEAREST);
	

		size = pow(2, 7); voxeltexture[1] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		size = pow(2, 6); voxeltexture[2] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		size = pow(2, 5); voxeltexture[3] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		size = pow(2, 4); voxeltexture[4] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		size = pow(2, 3); voxeltexture[5] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		size = pow(2, 2); voxeltexture[6] = make3DTexture(size, size, size, 1, GL_RGBA16F, GL_LINEAR, GL_LINEAR);
		int z;
		z = 0;
	}
	//*************************************
	double get_last_elapsed_time()
		{
		static double lasttime = glfwGetTime();
		double actualtime = glfwGetTime();
		double difference = actualtime - lasttime;
		lasttime = actualtime;
		return difference;
		}
	

	void update_scene()
	{	
		//update mouse
		mouse.process(windowManager->getHandle(), &mycam.rot);

		//update camera
		mycam.process();

		// update light
		if (key_i == 1)
			{
			primaryLight.position.y += 0.1f;
			primaryLight.direction = normalize(origin - primaryLight.position);
			}
		if (key_k == 1)
			{
			primaryLight.position.y -= 0.1f;
			primaryLight.direction = normalize(origin - primaryLight.position);
			}
		if (key_j == 1)
			{
			primaryLight.position.x -= 0.1f;
			primaryLight.direction = normalize(origin - primaryLight.position);
			}
		if (key_l == 1)
			{
			primaryLight.position.x += 0.1f;
			primaryLight.direction = normalize(origin - primaryLight.position);
			}

		//update models
		static float angle = 0;
		angle += 0.02;
		M_Earth = glm::translate(glm::mat4(1.f), earth_pos);
		glm::mat4 Ry = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0, 1, 0));
		float pih = -3.1415926 / 2.0;
		glm::mat4 Rx = glm::rotate(glm::mat4(1.f), pih, glm::vec3(1, 0, 0));
		glm::mat4 Se = glm::scale(glm::mat4(1.f), glm::vec3(0.65, 0.65, 0.65));
		M_Earth = M_Earth * Ry * Rx*Se;

		static float moonangle = 0;
		moonangle += 0.005;
		M_Moon = glm::translate(glm::mat4(1.f), moon_pos_offset);
		glm::mat4 Ryrad = glm::rotate(glm::mat4(1.f), moonangle, glm::vec3(0, 1, 0));
		glm::mat4 T = glm::translate(glm::mat4(1.f), earth_pos);
		glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(0.15, 0.15, 0.15));
		M_Moon = T * Ryrad * M_Moon * Rx * S;

		static float sponzaangle = pih;	
		M_Sponza = glm::rotate(glm::mat4(1.f), sponzaangle, glm::vec3(0, 1, 0))*glm::scale(glm::mat4(1), glm::vec3(10.0, 10.0, 10.0));
	}

	void get_light_proj_matrix(glm::mat4& lightP)
	{
		// If your scene goes outside these "bounds" (e.g. shadows stop working near boundary),
		// feel free to increase these numbers (or decrease if scene shrinks/light gets closer to
		// scene objects).
		const float left = -15.0f;
		const float right = 15.0f;
		const float bottom = -15.0f;
		const float top = 15.0f;
		const float zNear = 0.1f;
		const float zFar = 50.0f;

		lightP = glm::ortho(left, right, bottom, top, zNear, zFar);
	}

	void get_light_view_matrix(glm::mat4& lightV)
	{
		// Change earth_pos (or primaryLight.direction) to change where the light is pointing at.
		lightV = glm::lookAt(primaryLight.position, primaryLight.position + primaryLight.direction, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	
	void render_to_shadowmap()
		{
		glBindFramebuffer(GL_FRAMEBUFFER, fb_shadowMap);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, SHADOW_DIM, SHADOW_DIM);

		glDisable(GL_BLEND);

		glm::mat4 M, V, S, T, P;

		// Orthographic frustum in light space; encloses the scene, adjust if larger or smaller scene used.
		get_light_proj_matrix(P);

		// "Camera" for rendering shadow map is at light source, looking at the scene.
		get_light_view_matrix(V);

		// Bind shadow map shader program and matrix uniforms.
		shadowProg->bind();
		glUniformMatrix4fv(shadowProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(shadowProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		// set matrices
		//	******		earth		******
		glUniformMatrix4fv(shadowProg->getUniform("M"), 1, GL_FALSE, &M_Earth[0][0]);

		shape->drawBasic(shadowProg);	//draw earth

										//	******		moon		******
		glUniformMatrix4fv(shadowProg->getUniform("M"), 1, GL_FALSE, &M_Moon[0][0]);

		shape->drawBasic(shadowProg);	//draw moon

										//	******		sponza		******
		glUniformMatrix4fv(shadowProg->getUniform("M"), 1, GL_FALSE, &M_Sponza[0][0]);
		sponza->drawBasic(shadowProg);	//draw sponza

										//done, unbind stuff
		shadowProg->unbind();
		glEnable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth);
		glGenerateMipmap(GL_TEXTURE_2D);
		}
	void voxelize(int direction) // aka render to framebuffer
		{
		glDisable(GL_CULL_FACE);
		glBindFramebuffer(GL_FRAMEBUFFER, FBOvoxelize);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 V, P, lightP, lightV, lightSpace;
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);

		
		switch (direction)
			{
			default:
			case 0:			V = glm::lookAt(vec3(0, 0, 15), vec3(0), vec3(0, 1, 0)) * scale(mat4(1), vec3(0.15, 0.15, 0.15));	break;
			case 1:			V = glm::lookAt(vec3(15, 0, 0), vec3(0), vec3(0, 1, 0)) * scale(mat4(1), vec3(0.15, 0.15, 0.15));	break;
			case 2:			V = glm::lookAt(vec3(0, 15, 0), vec3(0), vec3(1, 0, 0)) * scale(mat4(1), vec3(0.15, 0.15, 0.15));	break;
			}
		// Orthographic frustum in light space; encloses the scene, adjust if larger or smaller scene used.
		get_light_proj_matrix(lightP);

		// "Camera" for rendering shadow map is at light source, looking at the scene.
		get_light_view_matrix(lightV);

		lightSpace = lightP * lightV;

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind shader and copy matrices

		voxprog->bind();
		glUniformMatrix4fv(voxprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(voxprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(voxprog->getUniform("lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);
		glUniform3fv(voxprog->getUniform("campos"), 1, &mycam.pos.x);
		glUniform3fv(voxprog->getUniform("lightpos"), 1, &primaryLight.position.x);
		glUniform3fv(voxprog->getUniform("lightdir"), 1, &primaryLight.direction.x);
		glUniform1i(voxprog->getUniform("integer_textures_on"), (int)use_integer_textures_against_flickering);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth);
		
		//bind shader and copy matrices
		

		//bind voxel texture
		glBindImageTexture(3, voxeltexture[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(4, voxeltexture_r, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
		glBindImageTexture(5, voxeltexture_g, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
		glBindImageTexture(6, voxeltexture_b, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
		glBindImageTexture(7, voxeltexture_c, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
		
		
		//	******		earth		******
		glUniformMatrix4fv(voxprog->getUniform("M"), 1, GL_FALSE, &M_Earth[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureEarth);
		shape->draw(voxprog, true);	//draw earth

									//	******		moon		******
		glUniformMatrix4fv(voxprog->getUniform("M"), 1, GL_FALSE, &M_Moon[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMoon);
		shape->draw(voxprog, true);	//draw moon

									//	******		sponza		******
		glUniformMatrix4fv(voxprog->getUniform("M"), 1, GL_FALSE, &M_Sponza[0][0]);
		sponza->draw(voxprog, false);	//draw sponza

									//done, unbind stuff
		voxprog->unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOvoxelizeTex);
		glGenerateMipmap(GL_TEXTURE_2D);
		}
	//******************************
	void compute()
		{
		if (use_integer_textures_against_flickering)
			{
			glUseProgram(CSaver);
			glBindImageTexture(0, voxeltexture[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glBindImageTexture(1, voxeltexture_r, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
			glBindImageTexture(2, voxeltexture_g, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
			glBindImageTexture(3, voxeltexture_b, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
			glBindImageTexture(4, voxeltexture_c, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
			glDispatchCompute(256, 256, 256);				//start compute shader
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}

		glUseProgram(CSmipmap);
		int asize = glGetUniformLocation(CSmipmap, "actualsize");
		int integer_textures_on = glGetUniformLocation(CSmipmap, "integer_textures_on");
		for (int i = 0; i < 6; i++)
			{
			glBindTextureUnit(0, voxeltexture[i]);
			//	glBindTextureUnit(1, voxeltexture[i]);
			//glBindImageTexture(0, voxeltexture[i], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
			glBindImageTexture(1, voxeltexture[i + 1], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glBindImageTexture(2, voxeltexture_c, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
			//	glBindTextureUnit(2, voxeltexture_c);
			GLuint size = pow(2, 7 - i);
			glUniform1i(asize, i + 1);
			glUniform1i(integer_textures_on, use_integer_textures_against_flickering);
			glDispatchCompute(size, size, size);				//start compute shader
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}

		}
	//*********************************************************
	void render_to_texture() // aka render to framebuffer
		{
		glEnable(GL_CULL_FACE);
		
		glBindFramebuffer(GL_FRAMEBUFFER, FBOmain);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3};
		glDrawBuffers(3, drawBuffers);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 V, P, lightP, lightV, lightSpace;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		V = mycam.get_viewmatrix();

		
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind shader and copy matrices
		prog->bind();

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
	
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth);

		//	******		earth		******
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_Earth[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureEarth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureEarthNorm);
		shape->draw(prog, true);	//draw earth

									//	******		moon		******
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_Moon[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMoon);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureMoonNorm);
		shape->draw(prog, true);	//draw moon

									//	******		sponza		******
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_Sponza[0][0]);
		sponza->draw(prog, false);	//draw sponza

									//done, unbind stuff
		prog->unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOnorm);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOtan);
		glGenerateMipmap(GL_TEXTURE_2D);
		}
	//**********************************************
	void render_to_screen()
		{
		// Get current frame buffer size.

		double frametime = get_last_elapsed_time();

		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		P->pushMatrix();
		P->perspective(70., width, height, 0.1, 100.0f);
		glm::mat4 M, V, S, T;

		V = glm::mat4(1);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		prog2->bind();

		mat4 lightP, lightV;
		get_light_proj_matrix(lightP);// "Camera" for rendering shadow map is at light source, looking at the scene.
		get_light_view_matrix(lightV);
		mat4 lightSpace = lightP * lightV;

		glUniformMatrix4fv(prog2->getUniform("lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);
		glUniform3fv(prog2->getUniform("campos"), 1, &mycam.pos.x);
		glUniform3fv(prog2->getUniform("lightpos"), 1, &primaryLight.position.x);
		glUniform3fv(prog2->getUniform("lightdir"), 1, &primaryLight.direction.x);

		glUniform1f(prog2->getUniform("steps_for_tracing"), steps_for_tracing);
		glUniform1f(prog2->getUniform("cut_off_alpha"), cut_off_alpha);
		glUniform1f(prog2->getUniform("reflective_cone_angle"), reflective_cone_angle);
		glUniform1f(prog2->getUniform("ambient_factor"), ambient_factor);
		glUniform1f(prog2->getUniform("shadow_factor"), shadow_factor);

		glActiveTexture(GL_TEXTURE0);
		// Debug, shows shadow map when 'y' is pressed
		show_shadowmap ? glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth) : glBindTexture(GL_TEXTURE_2D, FBOtex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOnorm);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, FBOtan);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth);
		

		glBindTextureUnit(5, voxeltexture[0]);
		glBindTextureUnit(6, voxeltexture[1]);
		glBindTextureUnit(7, voxeltexture[2]);
		glBindTextureUnit(8, voxeltexture[3]);
		glBindTextureUnit(9, voxeltexture[4]);
		glBindTextureUnit(10, voxeltexture[5]);
		glBindTextureUnit(11, voxeltexture[6]);
		//glBindImageTexture(2, voxeltexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);


		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		static float mml = 0;
		if (key_n == 1)	mml -= 0.5;
		if (key_m == 1)	mml += 0.5;
		if (mml < 0)mml = 0;
		if (mml > 8)mml = 8;
		glUniform1f(prog2->getUniform("manualmipmaplevel"), mml);

		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		prog2->unbind();
		}

	void reset_frame()
		{
		glClearTexImage(voxeltexture[0], 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		glClearTexImage(voxeltexture_r, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		glClearTexImage(voxeltexture_g, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		glClearTexImage(voxeltexture_b, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		glClearTexImage(voxeltexture_c, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		}
	void voxelmipmaps()
		{
		compute();
		//glBindTexture(GL_TEXTURE_3D, voxeltexture[0]);
	//	glGenerateTextureMipmap(voxeltexture[0]);
		}

	

};
//*********************************************************************************************************
int main(int argc, char **argv)
{

	vec3 tangent, binormal, normal;
	normal = vec3(0, 0, -1);
	binormal = vec3(0, -1, 0);
	tangent = vec3(1, 0, 0);
	mat3 TBN = mat3(tangent, binormal, normal);
	vec3 bumpNormal = TBN * normal;

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
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Update scene.
		application->update_scene();

		// Render scene.
		application->render_to_shadowmap();
		application->render_to_texture();
		application->voxelize(0);
		application->voxelize(1);
		application->voxelize(2);
		application->voxelmipmaps();
	
		application->render_to_screen();
		application->reset_frame();
		
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}




void GetTangent(
	glm::vec3 *posA, glm::vec3 *posB, glm::vec3 *posC,
	glm::vec2 *texA, glm::vec2 *texB, glm::vec2 *texC,
	glm::vec3 *nnA, glm::vec3 *nnB, glm::vec3 *nnC,
	glm::vec3 *tanA, glm::vec3 *tanB, glm::vec3 *tanC);
void GetTangent2(
	glm::vec3 *posA, glm::vec3 *posB, glm::vec3 *posC,
	glm::vec2 *texA, glm::vec2 *texB, glm::vec2 *texC,
	glm::vec3 *nnA, glm::vec3 *nnB, glm::vec3 *nnC,
	glm::vec3 *tanA, glm::vec3 *tanB, glm::vec3 *tanC);
//calculate tangents and binormals
void computeTangentSpace(Shape *shape)
	{
	for (int u = 0; u < shape->obj_count; u++)
		{
		GLfloat* tangents = new GLfloat[shape->posBuf[u].size()]();
		GLfloat* binormals = new GLfloat[shape->posBuf[u].size()]();

		std::vector<glm::vec3 > tangent;
		std::vector<glm::vec3 > binormal;
		int im = 0;

		for (unsigned int i = 0; i < shape->eleBuf[u].size(); i = i + 3) {

			if (shape->eleBuf[u].at(i + 0) > im)			im = shape->eleBuf[u].at(i + 0);
			if (shape->eleBuf[u].at(i + 1) > im)			im = shape->eleBuf[u].at(i + 1);
			if (shape->eleBuf[u].at(i + 2) > im)			im = shape->eleBuf[u].at(i + 2);



			glm::vec3 vertex0 = glm::vec3(shape->posBuf[u].at(shape->eleBuf[u].at(i + 0) * 3 + 0), shape->posBuf[u].at(shape->eleBuf[u].at(i + 0) * 3 + 1), shape->posBuf[u].at(shape->eleBuf[u].at(i + 0) * 3 + 2));
			glm::vec3 vertex1 = glm::vec3(shape->posBuf[u].at(shape->eleBuf[u].at(i + 1) * 3 + 0), shape->posBuf[u].at(shape->eleBuf[u].at(i + 1) * 3 + 1), shape->posBuf[u].at(shape->eleBuf[u].at(i + 1) * 3 + 2));
			glm::vec3 vertex2 = glm::vec3(shape->posBuf[u].at(shape->eleBuf[u].at(i + 2) * 3 + 0), shape->posBuf[u].at(shape->eleBuf[u].at(i + 2) * 3 + 1), shape->posBuf[u].at(shape->eleBuf[u].at(i + 2) * 3 + 2));

			glm::vec3 normal0 = glm::vec3(shape->norBuf[u].at(shape->eleBuf[u].at(i + 0) * 3 + 0), shape->norBuf[u].at(shape->eleBuf[u].at(i + 0) * 3 + 1), shape->norBuf[u].at(shape->eleBuf[u].at(i + 0) * 3 + 2));
			glm::vec3 normal1 = glm::vec3(shape->norBuf[u].at(shape->eleBuf[u].at(i + 1) * 3 + 0), shape->norBuf[u].at(shape->eleBuf[u].at(i + 1) * 3 + 1), shape->norBuf[u].at(shape->eleBuf[u].at(i + 1) * 3 + 2));
			glm::vec3 normal2 = glm::vec3(shape->norBuf[u].at(shape->eleBuf[u].at(i + 2) * 3 + 0), shape->norBuf[u].at(shape->eleBuf[u].at(i + 2) * 3 + 1), shape->norBuf[u].at(shape->eleBuf[u].at(i + 2) * 3 + 2));

			glm::vec2 tex0 = glm::vec2(shape->texBuf[u].at(shape->eleBuf[u].at(i + 0) * 2 + 0), shape->texBuf[u].at(shape->eleBuf[u].at(i + 0) * 2 + 1));
			glm::vec2 tex1 = glm::vec2(shape->texBuf[u].at(shape->eleBuf[u].at(i + 1) * 2 + 0), shape->texBuf[u].at(shape->eleBuf[u].at(i + 1) * 2 + 1));
			glm::vec2 tex2 = glm::vec2(shape->texBuf[u].at(shape->eleBuf[u].at(i + 2) * 2 + 0), shape->texBuf[u].at(shape->eleBuf[u].at(i + 2) * 2 + 1));

			glm::vec3 tan0, tan1, tan2; // tangents
			glm::vec3 bin0, bin1, bin2; // binormal


			GetTangent2(&vertex0, &vertex1, &vertex2, &tex0, &tex1, &tex2, &normal0, &normal1, &normal2, &tan0, &tan1, &tan2);


			bin0 = glm::normalize(glm::cross(tan0, normal0));
			bin1 = glm::normalize(glm::cross(tan1, normal1));
			bin2 = glm::normalize(glm::cross(tan2, normal2));

			// write into array - for each vertex of the face the same value
			tangents[shape->eleBuf[u].at(i + 0) * 3 + 0] = tan0.x;
			tangents[shape->eleBuf[u].at(i + 0) * 3 + 1] = tan0.y;
			tangents[shape->eleBuf[u].at(i + 0) * 3 + 2] = tan0.z;

			tangents[shape->eleBuf[u].at(i + 1) * 3 + 0] = tan1.x;
			tangents[shape->eleBuf[u].at(i + 1) * 3 + 1] = tan1.y;
			tangents[shape->eleBuf[u].at(i + 1) * 3 + 2] = tan1.z;

			tangents[shape->eleBuf[u].at(i + 2) * 3 + 0] = tan2.x;
			tangents[shape->eleBuf[u].at(i + 2) * 3 + 1] = tan2.y;
			tangents[shape->eleBuf[u].at(i + 2) * 3 + 2] = tan2.z;

			binormals[shape->eleBuf[u].at(i + 0) * 3 + 0] = bin0.x;
			binormals[shape->eleBuf[u].at(i + 0) * 3 + 1] = bin0.y;
			binormals[shape->eleBuf[u].at(i + 0) * 3 + 2] = bin0.z;

			binormals[shape->eleBuf[u].at(i + 1) * 3 + 0] = bin1.x;
			binormals[shape->eleBuf[u].at(i + 1) * 3 + 1] = bin1.y;
			binormals[shape->eleBuf[u].at(i + 1) * 3 + 2] = bin1.z;

			binormals[shape->eleBuf[u].at(i + 2) * 3 + 0] = bin2.x;
			binormals[shape->eleBuf[u].at(i + 2) * 3 + 1] = bin2.y;
			binormals[shape->eleBuf[u].at(i + 2) * 3 + 2] = bin2.z;
			}
		// Copy the tangent and binormal to meshData
		for (unsigned int i = 0; i < shape->posBuf[u].size(); i++) {
			shape->tanBuf[u].push_back(tangents[i]);
			shape->binormBuf[u].push_back(binormals[i]);
			}
		}
	}


void GetTangent2(
	glm::vec3 *posA, glm::vec3 *posB, glm::vec3 *posC,
	glm::vec2 *texA, glm::vec2 *texB, glm::vec2 *texC,
	glm::vec3 *nnA, glm::vec3 *nnB, glm::vec3 *nnC,
	glm::vec3 *tanA, glm::vec3 *tanB, glm::vec3 *tanC)
	{
	vec3 v1 = *posA;
	vec3 v2 = *posB;
	vec3 v3 = *posC;

	vec2 vu1 = *texA;
	vec2 vu2 = *texB;
	vec2 vu3 = *texC;

	vec3 v2v1 = v2 - v1;
	vec3 v3v1 = v3 - v1;

float c2c1t = vu2.x - vu1.x;
float c2c1b = vu2.y - vu1.y;

float c3c1t = vu3.x - vu1.x;
float c3c1b = vu3.y - vu1.y;

vec3 vecNormal =*nnA;
vec3 vecTangent, vecSmoothBitangent, vecSmoothTangent;

vecNormal = *nnA;
vecTangent = vec3(c3c1b * v2v1.x - c2c1b * v3v1.x, c3c1b * v2v1.y - c2c1b * v3v1.y, c3c1b * v2v1.z - c2c1b * v3v1.z);
vecSmoothBitangent = cross(vecNormal, normalize(vecTangent));
vecSmoothTangent = cross(vecSmoothBitangent, normalize(vecNormal)); 
*tanA = vecSmoothTangent;

vecNormal = *nnB;
//vecTangent = vec3(c3c1b * v2v1.x - c2c1b * v3v1.x, c3c1b * v2v1.y - c2c1b * v3v1.y, c3c1b * v2v1.z - c2c1b * v3v1.z);
vecSmoothBitangent = cross(vecNormal, normalize(vecTangent));
vecSmoothTangent = cross(vecSmoothBitangent, normalize(vecNormal));
*tanB = vecSmoothTangent;

vecNormal = *nnC;
//vecTangent = vec3(c3c1b * v2v1.x - c2c1b * v3v1.x, c3c1b * v2v1.y - c2c1b * v3v1.y, c3c1b * v2v1.z - c2c1b * v3v1.z);
vecSmoothBitangent = cross(vecNormal, normalize(vecTangent));
vecSmoothTangent = cross(vecSmoothBitangent, normalize(vecNormal));
*tanC = vecSmoothTangent;

}
void GetTangent(
	glm::vec3 *posA, glm::vec3 *posB, glm::vec3 *posC,
	glm::vec2 *texA, glm::vec2 *texB, glm::vec2 *texC,
	glm::vec3 *nnA, glm::vec3 *nnB, glm::vec3 *nnC,
	glm::vec3 *tanA, glm::vec3 *tanB, glm::vec3 *tanC)
	{

	if (!posA || !posB || !posC) return;
	if (!texA || !texB || !texC) return;
	if (!nnA || !nnB || !nnC) return;

	glm::vec3 v1 = *posA;
	glm::vec3 v2 = *posB;
	glm::vec3 v3 = *posC;

	glm::vec2 w1 = *texA;
	glm::vec2 w2 = *texB;
	glm::vec2 w3 = *texC;





	float x1 = v2.x - v1.x;
	float x2 = v3.x - v1.x;
	float y1 = v2.y - v1.y;
	float y2 = v3.y - v1.y;
	float z1 = v2.z - v1.z;
	float z2 = v3.z - v1.z;

	float s1 = w2.x - w1.x;
	float s2 = w3.x - w1.x;
	float t1 = w2.y - w1.y;
	float t2 = w3.y - w1.y;
	float inf = (s1 * t2 - s2 * t1);
	if (inf == 0)	inf = 0.0001;
	float r = 1.0F / inf;
	glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
		(t2 * z1 - t1 * z2) * r);
	glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
		(s1 * z2 - s2 * z1) * r);


	glm::vec3 erg;

	erg = sdir - (*nnA) * glm::dot(*nnA, sdir);
	erg = glm::normalize(erg);

	*tanA = erg;
	if (tanB)
		{
		erg = sdir - (*nnB) * glm::dot(*nnB, sdir);
		erg = glm::normalize(erg);
		*tanB = erg;
		}
	if (tanC)
		{
		erg = sdir - (*nnC) * glm::dot(*nnC, sdir);
		erg = glm::normalize(erg);
		*tanC = erg;
		}

	}