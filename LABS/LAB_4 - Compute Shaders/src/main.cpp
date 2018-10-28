/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/
//		showcase:
///		uniforms in CS
///		atomic counters
///		atomic operations
///		texture handling
///		workgroups

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <time.h>
#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;

#define BUFFCOUNT 4097
class ssbo_data
	{
	public:
		vec4 dataA[BUFFCOUNT];
		ivec4 dataB[BUFFCOUNT];
	};
float frand()
	{
	return (float)rand() / (float)RAND_MAX;
	}

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}



class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
	//texture data
	GLuint Texture;
	
	ssbo_data ssbo_CPUMEM;
	GLuint ssbo_GPU_id;
	GLuint computeProgram;
	GLuint atomicsBuffer;

	int array_size = BUFFCOUNT - 1;

	int odd_even = 0;

	int sizeofbuffer = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {}
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {}
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) {}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void init_atomic()
	{
		glGenBuffers(1, &atomicsBuffer);
		// bind the buffer and define its initial storage capacity
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 1, NULL, GL_DYNAMIC_DRAW);
		// unbind the buffer 
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	}
	void reset_atomic()
	{
		GLuint *userCounters;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
		// map the buffer, userCounters will point to the buffers data
		userCounters = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
			0,
			sizeof(GLuint),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
		);
		// set the memory to zeros, resetting the values in the buffer
		memset(userCounters, 0, sizeof(GLuint));
		// unmap the buffer
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	}
	void read_atomic()
	{
		GLuint *userCounters;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
		// again we map the buffer to userCounters, but this time for read-only access
		userCounters = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
			0,
			sizeof(GLuint),
			GL_MAP_READ_BIT
		);
		// copy the values to other variables because...
		//cout << endl << *userCounters << endl;
		// ... as soon as we unmap the buffer
		// the pointer userCounters becomes invalid.
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	}
	void initGeom()
	{
		
		string resourceDirectory = "../resources";			
		int width, height, channels;
		char filepath[1000];
		//texture 1
		string str = resourceDirectory + "/Blue_Giant.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindImageTexture(0, Texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		glGenerateMipmap(GL_TEXTURE_2D);		
		
		//make an SSBO
		for (int ii = 0; ii < BUFFCOUNT; ii++)
			{
			//ssbo_CPUMEM.dataA[ii] = vec4(ii, 0.0, 0.0, 0.0);
			//ssbo_CPUMEM.dataA[ii] = vec4(ii, rand(), rand(), rand());
			ssbo_CPUMEM.dataA[ii] = vec4(ii, rand(), 0.0, 0.0);
			ssbo_CPUMEM.dataB[ii] = vec4(0.0, 0.0, 0.0, 0.0);
			}
		glGenBuffers(1, &ssbo_GPU_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_GPU_id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &ssbo_CPUMEM, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_GPU_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind


	}

	//General OGL initialization - set OGL state here
	void init()
	{
		GLSL::checkVersion();
		//load the compute shader
		std::string ShaderString = readFileAsString("../resources/compute.glsl");
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
			//exit(1);
			}


		computeProgram = glCreateProgram();
		glAttachShader(computeProgram, computeShader);
		glLinkProgram(computeProgram);
		glUseProgram(computeProgram);

		sizeofbuffer = glGetUniformLocation(computeProgram, "sizeofbuffer");
		
		GLuint block_index = 0;
		block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "shader_data");
		GLuint ssbo_binding_point_index = 2;
		glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);

	}
	void compute()
		{

		ofstream fout("output.txt");

		////print data before compute shader
		cout << endl << endl << "BUFFER BEFORE COMPUTE SHADER" << endl << endl;
		fout << endl << endl << "BUFFER BEFORE COMPUTE SHADER" << endl << endl;
		for (int i = 0; i < BUFFCOUNT; i++) {
			cout << "dataA: " << "Index: " << ssbo_CPUMEM.dataA[i].x << ", Value: " << ssbo_CPUMEM.dataA[i].y << endl;
			fout << "dataA: " << "Index: " << ssbo_CPUMEM.dataA[i].x << ", Value: " << ssbo_CPUMEM.dataA[i].y << endl;
		}

		int buffersize = BUFFCOUNT;
		int groupsize = 1024;
		int workload_per_shader = buffersize / groupsize;
		int remainder = buffersize % groupsize;
		if (remainder > 0) {
			workload_per_shader++;
		}

		cout << "Buffersize: " << buffersize << endl;
		cout << "Groupsize: " << groupsize << endl;
		cout << "Workload per shader: " << workload_per_shader << endl;
		cout << "Remainder: " << remainder << endl;

		for (int i = 0; i < BUFFCOUNT; i++) {
			GLuint block_index = 0;
			block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "shader_data");
			GLuint ssbo_binding_point_index = 0;
			glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_GPU_id);
			glUseProgram(computeProgram);

			GLint buff_loc = glGetUniformLocation(computeProgram, "sizeofbuffer");
			if (buff_loc != -1)
			{
				//glUniform1i(buff_loc, array_size);
				glUniform1i(buff_loc, BUFFCOUNT);
			}

			GLint odd_even_loc = glGetUniformLocation(computeProgram, "odd");
			if (odd_even_loc != -1)
			{
				if (odd_even == 0) {
					odd_even = 1;
				}
				else {
					odd_even = 0;
				}
				glUniform1i(odd_even_loc, odd_even);
			}

			//activate atomic counter
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicsBuffer);

			glDispatchCompute((GLuint)1, (GLuint)1, 1);				//start compute shader
																	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

			//copy data back to CPU MEM

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_GPU_id);
			GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
			int siz = sizeof(ssbo_data);
			memcpy(&ssbo_CPUMEM, p, siz);
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		
		cout << endl << endl;
		fout << endl << endl;
		cout << "BUFFER AFTER COMPUTE SHADER" << endl;
		fout << "BUFFER AFTER COMPUTE SHADER" << endl;
		for (int i = 0; i < BUFFCOUNT; i++) {
			cout << "dataA: " << "Index: " << ssbo_CPUMEM.dataA[i].x << ", Value: " << ssbo_CPUMEM.dataA[i].y << endl;
			fout << "dataA: " << "Index: " << ssbo_CPUMEM.dataA[i].x << ", Value: " << ssbo_CPUMEM.dataA[i].y << endl;
		}
		cout << "Array is sorted: " << std::boolalpha << check_if_array_sorted(ssbo_CPUMEM.dataA, BUFFCOUNT - 1) << endl;
		fout << "Array is sorted: " << std::boolalpha << check_if_array_sorted(ssbo_CPUMEM.dataA, BUFFCOUNT - 1) << endl;
		fout.close();
	}

	bool check_if_array_sorted(vec4 data[], int length) {
		for (int i = 0; i < length-1; i++) {
			if (data[i].y > data[i + 1].y) {
				return false;
			}
		}
		return true;
	}
};
//******************************************************************************************
int main(int argc, char **argv)
{
	Application *application = new Application();
	srand(time(0));

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(32, 32, "Dummy", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGL();

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group size x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);


	application->init();
	application->initGeom();

	application->init_atomic();
	
	application->compute();

	application->read_atomic();
	
	system("pause");
	return 0;
}
