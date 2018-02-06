// cpsc453 assignment 2 qiyue zhang 10131658


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <math.h>

#define _USE_MATH_DEFINES
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

unsigned char* pixels;
vector<float> colors = {1, 0, 0, 0, 1, 0, 0, 0, 1};
vector<float> vertices;
vector<float> textures;
int windowWidth, windowHeight;

bool closeWindow = false;
float scroll = 1.f;
bool mouseButtonPressed = false;
double prevCursorX, prevCursorY, offsetX, offsetY;
int grayScale = 0;
int twoBit = 0;
int dots = 0;
double dotX, dotY;
vector<vector<float>> controlPoints;
int closeCurve = 0;
int numCurves = 0;
bool showPoints = true;

class Program {
	GLuint vertex_shader;
	GLuint fragment_shader;
public:
	GLuint id;
	Program() {
		vertex_shader = 0;
		fragment_shader = 0;
		id = 0;
	}
	Program(string vertex_path, string fragment_path) {
		init(vertex_path, fragment_path);
	}
	void init(string vertex_path, string fragment_path) {
		id = glCreateProgram();
		vertex_shader = addShader(vertex_path, GL_VERTEX_SHADER);
		fragment_shader = addShader(fragment_path, GL_FRAGMENT_SHADER);
		if (vertex_shader)
			glAttachShader(id, vertex_shader);
		if (fragment_shader)
			glAttachShader(id, fragment_shader);

		glLinkProgram(id);
	}
	GLuint addShader(string path, GLuint type) {
		std::ifstream in(path);
		string buffer = [&in] {
			std::ostringstream ss {};
			ss << in.rdbuf();
			return ss.str();
		}();
		const char *buffer_array[] = { buffer.c_str() };

		GLuint shader = glCreateShader(type);

		glShaderSource(shader, 1, buffer_array, 0);
		glCompileShader(shader);


		// Compile results
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			string info(length, ' ');
			glGetShaderInfoLog(shader, info.length(), &length, &info[0]);
			cerr << "ERROR compiling shader:" << endl << endl;
			cerr << info << endl;
		}
		return shader;
	}
	~Program() {
		glUseProgram(0);
		glDeleteProgram(id);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}
};

class VertexArray {
	std::map<string, GLuint> buffers;
	std::map<string, int> indices;
public:
	GLuint id;
	unsigned int count;
	VertexArray(int c) {
		glGenVertexArrays(1, &id);
		count = c;
	}

	VertexArray(const VertexArray &v) {
		glGenVertexArrays(1, &id);

		// Copy data from the old object
		this->indices = std::map<string, int>(v.indices);
		count = v.count;

		vector<GLuint> temp_buffers(v.buffers.size());

		// Allocate some temporary buffer object handles
		glGenBuffers(v.buffers.size(), &temp_buffers[0]);

		// Copy each old VBO into a new VBO
		int i = 0;
		for (auto &ent : v.buffers) {
			int size = 0;
			glBindBuffer(GL_ARRAY_BUFFER, ent.second);
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

			glBindBuffer(GL_COPY_READ_BUFFER, temp_buffers[i]);
			glBufferData(GL_COPY_READ_BUFFER, size, NULL, GL_STATIC_COPY);

			glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_READ_BUFFER, 0, 0,
					size);
			i++;
		}

		// Copy those temporary buffer objects into our VBOs

		i = 0;
		for (auto &ent : v.buffers) {
			GLuint buffer_id;
			int size = 0;
			int index = indices[ent.first];

			glGenBuffers(1, &buffer_id);

			glBindVertexArray(this->id);
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
			glBindBuffer(GL_COPY_READ_BUFFER, temp_buffers[i]);
			glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

			// Allocate VBO memory and copy
			glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0,
					size);
			string indexs = ent.first;

			buffers[ent.first] = buffer_id;
			indices[ent.first] = index;

			// Setup the attributes
			size = size / (sizeof(float) * this->count);
			glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(index);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			i++;
		}

		// Delete temporary buffers
		glDeleteBuffers(v.buffers.size(), &temp_buffers[0]);
	}

	void addBuffer(string name, int index, vector<float> buffer) {
		GLuint buffer_id;
		glBindVertexArray(id);

		glGenBuffers(1, &buffer_id);
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float),
				buffer.data(), GL_STATIC_DRAW);
		buffers[name] = buffer_id;
		indices[name] = index;

		int components = buffer.size() == 9 ? 3 : 2;
		glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(index);

		// unset states
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void updateBuffer(string name, vector<float> buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[name]);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float),
				buffer.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	~VertexArray() {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		for (auto &ent : buffers)
			glDeleteBuffers(1, &ent.second);
	}
};

struct Texture {
	GLuint id;
	GLuint type;
	int w;
	int h;
	int channels;

	Texture() : id(0), type(0), w(0), h(0), channels(0){}

}tex1;

void initTexture(){

	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load("data/sintel.png", &tex1.w, &tex1.h, &tex1.channels, 0);

	int width = tex1.w; int height = tex1.h; int channels = tex1.channels;
	cout<<"width\t"<<width<<"\theight\t"<<height<<"\tchannels\t"<<channels<<endl;

	tex1.type = GL_TEXTURE_RECTANGLE;

	GLuint f1 = tex1.channels == 3 ? GL_RGB8 : GL_RGBA8;
	GLuint f2 = tex1.channels == 3 ? GL_RGB : GL_RGBA;

	glGenTextures(1, &tex1.id);
	glBindTexture(GL_TEXTURE_RECTANGLE, tex1.id);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, f1, tex1.w, tex1.h, 0, f2, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	stbi_image_free(pixels);

}

void render(VertexArray &va, GLuint pid, GLuint qid) {

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(pid);
	glBindVertexArray(va.id);
	glBindTexture(tex1.type, tex1.id);
	glDrawArrays(GL_TRIANGLES, 0, va.count);
	glBindTexture(tex1.type, 0);
	glBindVertexArray(0); //unbind
	glUseProgram(0);


	for(auto& curves : controlPoints) {


		VertexArray vac(curves.size()/2);
		vac.addBuffer("vac", 0, curves);
		if(showPoints) {
			glPointSize(5);
			glUseProgram(pid);

			glBindVertexArray(vac.id);
			glDrawArrays(GL_POINTS, 0, vac.count);
			glBindVertexArray(0);
			glUseProgram(0);
		}
		if(vac.count>=4){

			glUseProgram(qid);

			GLint curveScaleLoc = glGetUniformLocation(qid, "scroll");
			glUniform1f(curveScaleLoc, scroll);

			GLint curveOffsetLoc = glGetUniformLocation(qid, "offset");
			glUniform2f(curveOffsetLoc, offsetX/400, offsetY/400);

			glBindVertexArray(vac.id);
			for(uint i = 0; i < vac.count-3; i++) {
				glPatchParameteri(GL_PATCH_VERTICES, 4);
				glDrawArrays(GL_PATCHES, i, 4);
			}

			glBindVertexArray(0);
			glUseProgram(0);
			if(showPoints) {
				glPointSize(5);
				glUseProgram(pid);
				glBindVertexArray(vac.id);
				glDrawArrays(GL_POINTS, 0, vac.count);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		}
	}
}

void display(GLuint pid, GLuint qid) {
	float wr = (float)tex1.w/(float)max((float)tex1.w, (float)tex1.h);
	float hr = (float)tex1.h/(float)max((float)tex1.w, (float)tex1.h);

	float w = (float)tex1.w;
	float h = (float)tex1.h;

	float temp[12] = {-wr, -hr,
		wr, -hr,
		-wr, hr,
		-wr, hr,
		wr, -hr,
		wr, hr
	};

	for(uint i = 0; i < 12; i++) {
		vertices.push_back(temp[i]);
	}

	textures.push_back(0.0);
	textures.push_back(0.0);

	textures.push_back(w);
	textures.push_back(0.0);

	textures.push_back(0.0);
	textures.push_back(h);

	textures.push_back(0.0);
	textures.push_back(h);

	textures.push_back(w);
	textures.push_back(0.0);

	textures.push_back(w);
	textures.push_back(h);

	VertexArray va(vertices.size()/2);
	va.addBuffer("va", 0, vertices);
	va.addBuffer("tex", 1, textures);


	render(va, pid, qid);

}


bool isEven(int num) {
	return (num%2 == 0);
}

void toggleCurve() {
	if(closeCurve > 0 && isEven(closeCurve)) {
		controlPoints[numCurves].pop_back();
		controlPoints[numCurves].pop_back();

		controlPoints[numCurves].pop_back();
		controlPoints[numCurves].pop_back();

		controlPoints[numCurves].pop_back();
		controlPoints[numCurves].pop_back();

		dots--;
		dots--;
		dots--;
	}
	if(closeCurve > 0 && !isEven(closeCurve)){

		controlPoints[numCurves].push_back(controlPoints[numCurves][0]);
		controlPoints[numCurves].push_back(controlPoints[numCurves][1]);

		controlPoints[numCurves].push_back(controlPoints[numCurves][2]);
		controlPoints[numCurves].push_back(controlPoints[numCurves][3]);

		controlPoints[numCurves].push_back(controlPoints[numCurves][4]);
		controlPoints[numCurves].push_back(controlPoints[numCurves][5]);
		dots++;
		dots++;
		dots++;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) closeWindow = closeWindow?false:true;
     if (key == GLFW_KEY_Q && action == GLFW_PRESS) grayScale = (grayScale == 0)?1:0;
     if (key == GLFW_KEY_W && action == GLFW_PRESS) twoBit = (twoBit == 0)?1:0;
	if (key == GLFW_KEY_E && action == GLFW_PRESS) { twoBit = (twoBit == 0)?1:0; grayScale = (grayScale == 0)?1:0; }
     if (key == GLFW_KEY_A && action == GLFW_PRESS && controlPoints[numCurves].size() >= 2) {
		controlPoints[numCurves].pop_back();
		controlPoints[numCurves].pop_back();
		dots--;
		cout<<"Erasing last control point..."<<endl;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		if(numCurves > 0 ){
			controlPoints.erase(controlPoints.begin()+numCurves - 1, controlPoints.begin()+numCurves);
			numCurves--;
		}
		else{
			controlPoints.erase(controlPoints.begin());
			dots = 0;
		}
		cout<<"Erasing last curve..."<<endl;
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		int numRemoved = numCurves+1;
		for(uint i = 0; i < (uint)numRemoved; i++){
			controlPoints.erase(controlPoints.begin(), controlPoints.begin()+numCurves+1);
		}
		showPoints = true;
		closeCurve = 0;
		numCurves = 0;
		dots = 0;
		grayScale = false;
		twoBit = false;
		offsetX = 0.0;
		offsetY = 0.0;
		scroll = 1.0;
		cout<<"Restoring image to original state..."<<endl;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS && controlPoints[numCurves].size()/2 >= 4) {
		closeCurve++;
		toggleCurve();
		cout<<"Closing current curve...Now beginning next curve"<<endl;
		closeCurve++;
		numCurves++;
		controlPoints.emplace_back();
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS && controlPoints[numCurves].size()/2 >= 4) {
		closeCurve++;
		toggleCurve();
		cout<<"Closing current curve..."<<endl;
	}
	if (key == GLFW_KEY_G && action == GLFW_PRESS && controlPoints[numCurves].size()/2 >= 4) {
		if(!isEven(closeCurve)){closeCurve++;}
		cout<<"Now beginning next curve..."<<endl;
		numCurves++;

		controlPoints.emplace_back();
	}
     if (key == GLFW_KEY_Z && action == GLFW_PRESS) showPoints = showPoints?false:true;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if(scroll+yoffset<4.f&&scroll+yoffset>-.5f){
		scroll += (float)yoffset*0.1;
	}
}

bool dotWithinBounds(double dotx, double doty){

	float wr = ((float)tex1.w/(float)max((float)tex1.w, (float)tex1.h));
	float hr = ((float)tex1.h/(float)max((float)tex1.w, (float)tex1.h));

	dotx = (1.0/scroll)*((-1.0+(float)(dotx/400))-(offsetX/400));
	doty= (1.0/scroll)*((1.0 - (float)(doty/400))-(offsetY/400));

	return (abs(dotx)<=wr)&&(abs(doty)<=hr);

}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
 		mouseButtonPressed = true;
 	} else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
 		mouseButtonPressed = false;
     } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		glfwGetCursorPos(window, &dotX, &dotY);
		if(dotWithinBounds(dotX, dotY)){
			dots ++;
			controlPoints[numCurves].push_back((1.0/scroll)*((-1.0+(float)(dotX/400))-(offsetX/400)));
			controlPoints[numCurves].push_back((1.0/scroll)*((1.0 - (float)(dotY/400))-(offsetY/400)));
			cout<<"placing dot =  dotNum: "<<dots<<" at window coordinates: (" <<
			(1.0/scroll)*((-1.0+(float)(dotX/400))-(offsetX/400))<<", "<<
			(1.0/scroll)*((1.0 - (float)(dotY/400))-(offsetY/400))<<" )"<<endl;
		}
	}

}

void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	if(mouseButtonPressed) {
        	offsetX = offsetX + xpos - prevCursorX;
        	offsetY = offsetY - ypos + prevCursorY;

   	}
  	prevCursorX = xpos;
  	prevCursorY = ypos;
	//cout<<xpos<<"\t"<<ypos<<endl;
}


int main(int argc, char *argv[]) {

		if (!glfwInit()) {
                    cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
                    return -1;
          }
          GLFWwindow *window = 0;
          glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
          glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
          glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
          glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
          glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
          window = glfwCreateWindow(800, 800, "Assignment 2", 0, 0);
          glfwSetWindowAspectRatio(window, 1, 1);
          if (!window) {
                    cout << "failed to create window, TERMINATING" << endl;
                    glfwTerminate();
                    return -1;
          }
          glfwMakeContextCurrent(window);
          glfwSetKeyCallback(window, key_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetMouseButtonCallback(window, mouse_callback);
		glfwSetCursorPosCallback(window, cursor_callback);

	     initTexture();
	 	Program p("data/vertex.glsl", "data/fragment.glsl");

		Program q("data/vertex2.glsl", "data/fragment2.glsl");

		GLuint tc = q.addShader("data/tc.glsl", GL_TESS_CONTROL_SHADER);
		GLuint te = q.addShader("data/te.glsl", GL_TESS_EVALUATION_SHADER);

		if(tc){
			glAttachShader(q.id, tc);
		}
		else{cout<<"tesControl not attached"<<endl;}
		if(te){
			glAttachShader(q.id, te);
		}
		else{cout<<"tesEval not attached"<<endl;}

		glLinkProgram(q.id);

	     while(!glfwWindowShouldClose(window)) {
               glfwGetWindowSize(window, &windowWidth, &windowHeight); //detects current window size
               glViewport(0, 0, windowWidth, windowHeight);
			glUseProgram(p.id);
			GLint scaleLoc = glGetUniformLocation(p.id, "scroll");
			glUniform1f(scaleLoc, scroll);

			GLint offsetLoc = glGetUniformLocation(p.id, "offset");
			glUniform2f(offsetLoc, offsetX/400, offsetY/400);

			GLint grayScaleLoc = glGetUniformLocation(p.id, "grayScale");
			glUniform1i(grayScaleLoc, grayScale);

			GLint twoBitLoc = glGetUniformLocation(p.id, "twoBit");
			glUniform1i(twoBitLoc, twoBit);

			controlPoints.emplace_back();
			display(p.id, q.id);

			if(closeWindow){break;}
               glfwSwapBuffers(window);
               glfwPollEvents();
          }
          glfwDestroyWindow(window);
          glfwTerminate();

          cout << "bui bui" << endl;
          return 0;

}
