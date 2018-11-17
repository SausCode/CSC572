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

// Change this if you want a higher/lower resolution shadow map (affects performance!).
#define SHADOW_DIM 4096
#define SSBOSIZE 4096*4

class ssbo_data
{
    public:
        ivec2 pixels[SSBOSIZE];
};

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
	std::shared_ptr<Program> prog, prog2, shadowProg;
	
	// Shape to be used (from obj file)
	shared_ptr<Shape> shape, sponza;
	//Mouse
	Mouse mouse;

	//camera
	camera mycam;

	//texture for sim
	GLuint TextureEarth, TextureWall;
	GLuint TextureMoon, FBOtex, FBOmask, fb, depth_rb;
	GLuint FBOtex_shadowMapDepth, fb_shadowMap;
	glm::mat4 M_Earth;
	glm::mat4 M_Moon;
	glm::mat4 M_Sponza;
	const vec3 earth_pos = glm::vec3(0, 0, -5);
	const vec3 origin = glm::vec3(0, 0, 0);
	const vec3 moon_pos_offset = glm::vec3(-0.7, 0, 0.7);
	int key_j = 0, key_i = 0, key_k = 0, key_l = 0;
	
	GLuint VertexArrayIDBox, VertexBufferIDBox, VertexBufferTex, VertexBufferIDNorm;
	
	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	Light primaryLight;

	bool show_shadowmap = false;

	ssbo_data blurpixels;
	GLuint CScollect, CSblur, SSBOid;

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
		if (key == GLFW_KEY_I && action == GLFW_PRESS)		key_i = 1;
		if (key == GLFW_KEY_I && action == GLFW_RELEASE)	key_i = 0;
		if (key == GLFW_KEY_J && action == GLFW_PRESS)		key_j = 1;
		if (key == GLFW_KEY_J && action == GLFW_RELEASE)	key_j = 0;
		if (key == GLFW_KEY_K && action == GLFW_PRESS)		key_k = 1;
		if (key == GLFW_KEY_K && action == GLFW_RELEASE)	key_k = 0;
		if (key == GLFW_KEY_L && action == GLFW_PRESS)		key_l = 1;
		if (key == GLFW_KEY_L && action == GLFW_RELEASE)	key_l = 0;

		if (key == GLFW_KEY_T ){
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOid);
	        GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	        int siz = sizeof(ssbo_data);
	        memcpy(&blurpixels, p, siz);
	        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	        for (int i = 0; i < SSBOSIZE; i++){
	        	cout << "Sphere x: " << blurpixels.pixels[i].x << "  sphere y: " << blurpixels.pixels[i].y << endl;
	        }
	        blurpixels.pixels[0] = ivec2(1,1);
	        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &blurpixels, GL_DYNAMIC_COPY);
		}
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
        if (!rc)    //error compiling the shader file
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

void compute()
	{
        glUseProgram(CScollect);
        GLuint block_index = 0;
        block_index = glGetProgramResourceIndex(CScollect, GL_SHADER_STORAGE_BLOCK, "shader_data");
        GLuint ssbo_binding_point_index = 0;
        glShaderStorageBlockBinding(CScollect, block_index, ssbo_binding_point_index);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOid);        
        glBindImageTexture(0, FBOmask, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
        glDispatchCompute((GLuint)640, (GLuint)480, 1);                //start compute shader
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        
	}

void compute_blur()
	{
        glUseProgram(CSblur);
        GLuint block_index = 0;
        block_index = glGetProgramResourceIndex(CSblur, GL_SHADER_STORAGE_BLOCK, "shader_data");
        GLuint ssbo_binding_point_index = 0;
        glShaderStorageBlockBinding(CSblur, block_index, ssbo_binding_point_index);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOid);
		glBindImageTexture(0, FBOmask, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		glBindImageTexture(1, FBOtex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
        glDispatchCompute(SSBOSIZE, 1, 1);//start compute shader
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	}

	void init_screen_texture_fbo()
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		glGenFramebuffers(1, &fb);
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);

		// Deffered Rendering stuff
		// Generate Color Texture
		glGenTextures(1, &FBOtex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		// Generate Position Texture
		glGenTextures(1, &FBOmask);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOmask);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);

		//NULL means reserve texture memory, but texels are undefined
		//**** Tell OpenGL to reserve level 0
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//You must reserve memory for other mipmaps levels as well either by making a series of calls to
		//glTexImage2D or use glGenerateMipmapEXT(GL_TEXTURE_2D).
		//Here, we'll use :
		glGenerateMipmap(GL_TEXTURE_2D);
		//-------------------------
		glGenFramebuffers(1, &fb);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//Attach 2D texture to this FBO

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBOmask, 0);
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

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);

		// Super hacky resize support (changing aspect ratio doesn't scale right), but at least stuff doesn't vanish.
		init_screen_texture_fbo();
	}

	void init(const std::string& resourceDirectory)
	{
		// Flip image to make tex coords line up with image correctly (due to OpenGL vs DirectX differences).
		stbi_set_flip_vertically_on_load(true);

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
		prog->addUniform("lightSpace");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addUniform("lightpos");
		prog->addUniform("lightdir");
		prog->addUniform("sphereon");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");


		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/frag_nolight.glsl");
		if (!prog2->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertTex");

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
	}

	void initGeom(const std::string& resourceDirectory)
	{

		sponza = make_shared<Shape>();
		string sponzamtl = resourceDirectory + "/sponza/";
		sponza->loadMesh(resourceDirectory + "/sponza/sponza.obj", &sponzamtl, stbi_load);
		sponza->resize();
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
				cout << "status framebuffer: very bad!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
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

		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(TexLocation, 0);
		glUniform1i(ShadowTexLocation, 1);

		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &FBOtex);
		init_screen_texture_fbo();

		CSblur = init_computeshader(resourceDirectory + "/compute_blur.glsl");
		CScollect = init_computeshader(resourceDirectory + "/compute.glsl");

		//make an SSBO
        for (int ii = 0; ii < SSBOSIZE; ii++)
            blurpixels.pixels[ii] = ivec2(0,0);                
        //blurpixels.pixels[0] = ivec2(1, 1);
        glGenBuffers(1, &SSBOid);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOid);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &blurpixels, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOid);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

		
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
	//*************************************
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
		glm::mat4 M,V,S,T;		
	
		V = glm::mat4(1);
		
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		prog2->bind();
		glActiveTexture(GL_TEXTURE0);

		// Debug, shows shadow map when 'y' is pressed
		//show_shadowmap ? glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth) : glBindTexture(GL_TEXTURE_2D, FBOtex);
		show_shadowmap ? glBindTexture(GL_TEXTURE_2D, FBOmask) : glBindTexture(GL_TEXTURE_2D, FBOtex);
		M = glm::scale(glm::mat4(1),glm::vec3(1.2,1,1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		prog2->unbind();
		
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

	void render_to_texture() // aka render to framebuffer
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, buffers);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 V, P, lightP, lightV, lightSpace;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		V = mycam.get_viewmatrix();

		// Orthographic frustum in light space; encloses the scene, adjust if larger or smaller scene used.
		get_light_proj_matrix(lightP);

		// "Camera" for rendering shadow map is at light source, looking at the scene.
		get_light_view_matrix(lightV);

		lightSpace = lightP * lightV;

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind shader and copy matrices
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos.x);
		glUniform3fv(prog->getUniform("lightpos"), 1, &primaryLight.position.x);
		glUniform3fv(prog->getUniform("lightdir"), 1, &primaryLight.direction.x);
		glUniform1f(prog->getUniform("sphereon"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOtex_shadowMapDepth);

		//	******		earth		******
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_Earth[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureEarth);
		glUniform1f(prog->getUniform("sphereon"), 1);
		shape->draw(prog, true);	//draw earth
		glUniform1f(prog->getUniform("sphereon"), 0);
							
		//	******		moon		******
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_Moon[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMoon);
		shape->draw(prog, true);	//draw moon

		//	******		sponza		******
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_Sponza[0][0]);
		sponza->draw(prog, false);	//draw sponza

		//done, unbind stuff
		prog->unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOmask);
		glGenerateMipmap(GL_TEXTURE_2D);
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
		application->compute();
		application->compute_blur();
		application->render_to_screen();
		
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
