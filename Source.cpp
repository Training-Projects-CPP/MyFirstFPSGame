#pragma comment(lib, "winmm.lib")
#include <GL/glew.h>     // use this before GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <Mmsystem.h>
#include "Buffer.h"
#include "shader.h"
#include "Camera.h"
#include "Renderer.h"
#include <memory>
#include <btBulletDynamicsCommon.h>
#include <vector>
#include<cmath>

#define MAX_INFO_CARDS 3

using namespace std;


enum Texture { TEST = 0, FLOOR = 1, WALL = 2, ENEMY = 3, SHOOTER = 4, GUN = 5, TESTIMAGE = 6, IMAGE_1 = 7, IMAGE_2 = 8, IMAGE_EXIT = 9, IMAGE_3 = 10 };
bool isExit = false;
int kolor = 0;
GLFWwindow* window;
GLFWwindow* WelcomeWindow;
float screenWidth = 500, screenHeight = 500;
float lastX = 400, lastY = 300;
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection;
glm::mat4 mvp;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentFrame;
bool firstMouse = true;

Camera camera(glm::vec3(0.0f, 3.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
void ProcessInput();

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool createNewProjectile = false, isTap = false;
void DrawGun(Shader& shader, VertexBuffer& vb, int tex);
void DrawEntrance(Shader& shader, VertexBuffer& vb, int tex);


bool createNewRAYaFFECT = false, isTapRAY = false;
glm::vec3 lightColor = glm::vec3(1.0, 0.84, 0.667);
glm::vec3 temp = glm::vec3(1.0, 1.0, 0.0);
float ambientStrength = .4;
#include<tuple>

tuple< GLuint, GLuint, GLuint> CreateFrameBuffer()
{
	GLuint color_tex = 0, fb = 0, depth_tex = 0;
	//RGBA8 2D texture, 24 bit depth texture, 256x256
	glGenTextures(1, &color_tex);
	glBindTexture(GL_TEXTURE_2D, color_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//NULL means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//NULL means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	//-------------------------
	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	//Attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0/*mipmap level*/);
	//-------------------------
	//Attach depth texture to FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_tex, 0/*mipmap level*/);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	return tuple< GLuint, GLuint, GLuint>(fb, color_tex, depth_tex);
}

GLuint loadBMP_STB(const char* imagepath)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(imagepath, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return texture;
}



int main(void)
{



	auto collisionConfiguration = new btDefaultCollisionConfiguration();
	auto dispatcher = new btCollisionDispatcher(collisionConfiguration);
	auto overlappingPairCache = new btDbvtBroadphase();
	auto solver = new btSequentialImpulseConstraintSolver;
	auto  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	//WelcomeWindow = glfwCreateWindow(mode->width, mode->height, "WELCOME", glfwGetPrimaryMonitor(), NULL);
	window = glfwCreateWindow(mode->width, mode->height, "LET'S GO", glfwGetPrimaryMonitor(), NULL);
	screenHeight = (float)mode->height; //Resolution
	screenWidth = (float)mode->width; //Resolution
	cout << screenHeight << " " << screenWidth;
	//window = glfwCreateWindow(500, 500, "Hello World", NULL, NULL);



	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (GLEW_OK != glewInit())
	{
		cout << "Glew Init Error";
		return -1;
	}

	lastX = screenHeight / 2.;
	lastY = screenWidth / 2.;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);




	float x = 1.f, y = 1.f, z = -1.f;

	float positions[] = {
		//Cords     //TexCords  //Normal Plane
		-x,  y,  z, 0.0f, 0.0f, 0.0, 0.0, 1.0,//0
		 x,  y,  z, 1.0f, 0.0f, 0.0, 0.0, 1.0,//1
		 x, -y,  z, 1.0f, 1.0f, 0.0, 0.0, 1.0,//2
		-x, -y,  z, 0.0f, 1.0f, 0.0, 0.0, 1.0 //3

	};

	unsigned int indices[] = {
		0, 3, 2,
		0, 1, 2
	};



	VertexBuffer vb(positions, sizeof(positions));

	IndexBuffer ib(indices, 6);




	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);  	//Texture::TEST = 0
	GLuint tid_0 = loadBMP_STB("C:\\a\\floor_1.png");
	glBindTexture(GL_TEXTURE_2D, tid_0);

	glActiveTexture(GL_TEXTURE1);  	//Texture::FLOOR = 1
	GLuint tid_1 = loadBMP_STB("C:\\a\\wallaman.bmp");
	glBindTexture(GL_TEXTURE_2D, tid_1);

	glActiveTexture(GL_TEXTURE2);
	GLuint tid_2 = loadBMP_STB("C:\\a\\wall_6.bmp");
	glBindTexture(GL_TEXTURE_2D, tid_2);

	glActiveTexture(GL_TEXTURE3);
	GLuint tid_3 = loadBMP_STB("C:\\a\\enem.bmp");
	glBindTexture(GL_TEXTURE_2D, tid_3);

	glActiveTexture(GL_TEXTURE4);
	GLuint tid_4 = loadBMP_STB("C:\\a\\UFO.bmp");
	glBindTexture(GL_TEXTURE_2D, tid_4);

	glActiveTexture(GL_TEXTURE5);
	GLuint tid_5 = loadBMP_STB("C:\\a\\amangun.jpg");
	glBindTexture(GL_TEXTURE_2D, tid_5);

	glActiveTexture(GL_TEXTURE6);
	GLuint tid_6 = loadBMP_STB("C:\\a\\s1.jpg");
	glBindTexture(GL_TEXTURE_2D, tid_6);

	glActiveTexture(GL_TEXTURE7);
	GLuint tid_7 = loadBMP_STB("C:\\a\\ff.jpg");
	glBindTexture(GL_TEXTURE_2D, tid_7);

	glActiveTexture(GL_TEXTURE8);
	GLuint tid_8 = loadBMP_STB("C:\\a\\F1.jpg");
	glBindTexture(GL_TEXTURE_2D, tid_8);

	glActiveTexture(GL_TEXTURE9);
	GLuint tid_9 = loadBMP_STB("C:\\a\\ff2.jpg");
	glBindTexture(GL_TEXTURE_2D, tid_9);

	glActiveTexture(GL_TEXTURE10);
	GLuint tid_10 = loadBMP_STB("C:\\a\\ff0.jpg");
	glBindTexture(GL_TEXTURE_2D, tid_10);

	Shader shader("res\\shader\\sh0.shader");
	Shader Bulletshader("res\\shader\\shaderBullet.shader");
	Shader LightShader("res\\shader\\sh.shader");
	Shader EnemyShader("res\\shader\\shaderEnemy.shader");
	Shader GunShader("res\\shader\\sh2.shader");
	Shader EntranceShader("res\\shader\\EntranceShader.shader");


	//Initilization
	{
		model = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
	}
	/* Loop until the user closes the window */
	dynamicsWorld->setGravity({ 0, -6, 0 });
	vector<Cube*> cubeObjects;
	Cube* cptr = new Cube(dynamicsWorld, 2.0, glm::vec3(0.0, 3.0, 0.0), 1.0);
	cubeObjects.push_back(cptr);
	cubeObjects.push_back(cptr);
	cptr = new Cube(dynamicsWorld, 1.0, glm::vec3(9.0, 1., -5.0), 1.0);
	cubeObjects.push_back(cptr);
	cptr = new Cube(dynamicsWorld, 1.0, glm::vec3(10.0, 1., -5.0), 1.0);
	cubeObjects.push_back(cptr);
	cptr = new Cube(dynamicsWorld, 1.0, glm::vec3(11.0, 1., -5.0), 1.0);
	cubeObjects.push_back(cptr);
	cptr = new Cube(dynamicsWorld, 1.0, glm::vec3(9.5, 2., -5.0), 1.0);
	cubeObjects.push_back(cptr);
	cptr = new Cube(dynamicsWorld, 1.0, glm::vec3(10.5, 2., -5.0), 1.0);
	cubeObjects.push_back(cptr);
	cptr = new Cube(dynamicsWorld, 1.0, glm::vec3(10, 3., -5.0), 1.0);
	cubeObjects.push_back(cptr);


	CrossHair Cr;
	GLRectangle floor_1(dynamicsWorld, 20.0, 20.0, glm::vec3(10.0, 0.0, 10.0), 0.0, Orientation::ZX);
	GLRectangle floor_2(dynamicsWorld, 20.0, 20.0, glm::vec3(10.0, 0.0, -10.0), 0.0, Orientation::ZX);
	GLRectangle floor_3(dynamicsWorld, 20.0, 20.0, glm::vec3(-10.0, 0.0, 10.0), 0.0, Orientation::ZX);
	GLRectangle floor_4(dynamicsWorld, 20.0, 20.0, glm::vec3(-10.0, 0.0, -10.0), 0.0, Orientation::ZX);
	GLRectangle wall_1(dynamicsWorld, 40.0, 10.0, glm::vec3(0.0, 5, -20.0), 0.0, Orientation::XY);
	GLRectangle wall_2(dynamicsWorld, 40.0, 10.0, glm::vec3(0.0, 5, 20.0), 0.0, Orientation::XY);
	GLRectangle wall_3(dynamicsWorld, 40.0, 10.0, glm::vec3(-20.0, 5, 0.0), 0.0, Orientation::ZY);
	GLRectangle wall_4(dynamicsWorld, 40.0, 10.0, glm::vec3(20.0, 5, 0.0), 0.0, Orientation::ZY);
	GLRectangle wall_5(dynamicsWorld, 40.0, 10.0, glm::vec3(0.0, 15, -20.0), 0.0, Orientation::XY);
	GLRectangle wall_6(dynamicsWorld, 40.0, 10.0, glm::vec3(0.0, 15, 20.0), 0.0, Orientation::XY);
	GLRectangle wall_7(dynamicsWorld, 40.0, 10.0, glm::vec3(-20.0, 15, 0.0), 0.0, Orientation::ZY);
	GLRectangle wall_8(dynamicsWorld, 40.0, 10.0, glm::vec3(20.0, 15, 0.0), 0.0, Orientation::ZY);
	Cube Shooter(dynamicsWorld, 1.0, glm::vec3(0.0, 6.5, 0.0), 0.0);

	vector<EnemyProjectile*> ebullet;
	vector<Projectile*> bullet;
	Projectile* p = 0;
	EnemyProjectile* ep = 0;
	int flag = 0, flag2 = 0;
	long int shooterFlag = 0;


	int currentIMage = 0;
	PlaySound(TEXT("C:\\a\\song.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	float lightAngle = 0.0;
	float lightpC = 0.5;
	glm::vec3 lightPo ;

	while (!glfwWindowShouldClose(window))
	{

		lightAngle = lightAngle + lightpC;
		cout << lightAngle << endl;
		if (lightAngle >= 90.) {
			lightpC = -.5;

		}
		if (lightAngle <= -90.) {
			lightpC = 0.5;
		}
		lightPo = glm::vec3((float)19.*sin(glm::radians(lightAngle)), (float)19.*cos(glm::radians(lightAngle)), 0.0);
		/**/

		if (isExit) {
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
				exit(0);
			}
			DrawEntrance(EntranceShader, vb, Texture::IMAGE_EXIT);
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
			continue;
		}

		if (currentIMage < MAX_INFO_CARDS) {
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
				currentIMage++;
				if (currentIMage == 3) {
					//PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC);
				    //PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

				}
				Sleep(500);
			}
		if (currentIMage == 0) {
				DrawEntrance(EntranceShader, vb, Texture::IMAGE_1);
			}
			if (currentIMage == 1) {
				DrawEntrance(EntranceShader, vb, Texture::IMAGE_2);
		}
			if (currentIMage == 2) {
				DrawEntrance(EntranceShader, vb, Texture::IMAGE_3);
			}
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
			continue;
		}




		shooterFlag++;

		Cr.Draw(Bulletshader);
		if (kolor == 0) {
			lightColor = glm::vec3(1.0, 1.0, 1.0); //Wl
		}
		else if (kolor == 1) {
			lightColor = glm::vec3(1.0, 0.5764, 0.1608); //Tungsten
		}
		else if (kolor == 2) {
			lightColor = glm::vec3(0.251, 0.612, 0.5607); //Can
		}
		else {
			lightColor = glm::vec3(1.0, 0.84, 0.667);  //B
		}


		/* Render here */
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		ProcessInput();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Cr.Draw(Bulletshader);
		Shooter.Draw(shader, view, projection, Texture::SHOOTER);
		//Matrix Update
		projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
		view = camera.GetViewMatrix();


		floor_1.setLightPos(lightPo);
		floor_2.setLightPos(lightPo);
		floor_3.setLightPos(lightPo);
		floor_4.setLightPos(lightPo);
		wall_1.setLightPos(lightPo);
		wall_2.setLightPos(lightPo);
		wall_3.setLightPos(lightPo);
		wall_4.setLightPos(lightPo);
		wall_5.setLightPos(lightPo);
		wall_6.setLightPos(lightPo);
		wall_7.setLightPos(lightPo);
		wall_8.setLightPos(lightPo);

		//makeworld
		floor_1.Draw(LightShader, view, projection, Texture::FLOOR, camera, lightColor, ambientStrength);
		floor_2.Draw(LightShader, view, projection, Texture::FLOOR, camera, lightColor, ambientStrength);
		floor_3.Draw(LightShader, view, projection, Texture::FLOOR, camera, lightColor, ambientStrength);
		floor_4.Draw(LightShader, view, projection, Texture::FLOOR, camera, lightColor, ambientStrength);
		wall_1.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_2.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_3.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_4.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_5.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_6.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_7.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);
		wall_8.Draw(LightShader, view, projection, Texture::WALL, camera, lightColor, ambientStrength);


		//EndWORLDmAKING


		//rIGIDoBJ
		for (int i = 0; i < cubeObjects.size(); i++) {
			cubeObjects[i]->Draw(shader, view, projection, Texture::TEST);
		}
		//eND rIGIDoBJ
		if (bullet.size() > 5) {
			Projectile* temp = bullet[0];
			bullet.erase(bullet.begin());
			temp->RemoveRigidBody(dynamicsWorld);
			delete temp;
		}

		if (ebullet.size() > 4) {
			EnemyProjectile* temp = ebullet[0];
			ebullet.erase(ebullet.begin());
			temp->RemoveRigidBody(dynamicsWorld);
			delete temp;
		}

		//RAY CASTING
		flag = 0;
		flag2 = 0;
		glm::vec3 newPos = camera.Position - (float)1. * camera.Front;
		for (int i = 0; i < 50; i++) {
			newPos = newPos + camera.Front * ((float)0.1 * i);
			for (int k = 0; k < ebullet.size(); k++) {
				if (glm::distance(ebullet[k]->getPositionOfCube(), newPos) < 2.0) {
					//	cout << newPos.x << " " << newPos.y << " "<<newPos.z << " "<<i<<endl;
					if (createNewProjectile) {
						p = new Projectile(dynamicsWorld, ((ebullet[k]->getPositionOfCube()) - (float)(.1 * sqrt(2)) * camera.Front), 10000.0, 25.0, camera.Front);
						bullet.push_back(p);
						flag = 1;
						createNewProjectile = false;
						break;
					}
				}
				if (flag == 1) {
					break;
				}
			}
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isTap) {
				isTap = true;
				createNewProjectile = true;
			}
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && isTap) {
				isTap = false;
				createNewProjectile = false;
			}

			for (int k = 0; k < cubeObjects.size(); k++) {
				if (glm::distance(cubeObjects[k]->getPositionOfCube(), newPos) < 1.0) {
					//	cout << newPos.x << " " << newPos.y << " "<<newPos.z << " "<<i<<endl;
					if (createNewProjectile) {
						p = new Projectile(dynamicsWorld, ((cubeObjects[k]->getPositionOfCube()) - (float)(.1 * sqrt(2)) * camera.Front), 500.0, 15.0, camera.Front);
						bullet.push_back(p);
						flag2 = 1;
						createNewProjectile = false;
						break;
					}
				}
				if (flag2 == 1) {
					break;
				}
			}

		}
		//RAY CASTING



		//for (int i = 0; i < bullet.size(); i++) {
		//	bullet[i]->Draw(Bulletshader, view, projection);
		//}


		if (shooterFlag == 500) {

			shooterFlag = 0;
			ep = new EnemyProjectile(dynamicsWorld, glm::vec3(0.0, 7.0, 0.0), 0.1, 1.2, camera.Position - glm::vec3(0.0, 7.0, 0.0));
			ebullet.push_back(ep);
		}


		for (int i = 0; i < ebullet.size(); i++) {
			ebullet[i]->Draw(EnemyShader, view, projection, Texture::ENEMY);
		}

		//EndProjectile

		DrawGun(GunShader, vb, Texture::GUN);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
		dynamicsWorld->stepSimulation(1.0f / 100.f, 100);
	}

	glfwTerminate();
	return 0;
}

void DrawEntrance(Shader& shader, VertexBuffer& vb, int tex) {
	VertixAttrib va;
	BufferLayout layout(8);



	shader.Bind();



	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);


	posAttrib = shader.getAttribLocation("texCord");
	layout.Push<float>(posAttrib, 2, false);



	va.AddBuffer(vb, layout);
	glm::mat4 modelx = glm::mat4(1.0f);


	shader.setUniform4fv("model", modelx);
	shader.setUniform1i("texSampler_1", tex);


	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
}


void DrawGun(Shader& shader, VertexBuffer& vb, int tex)
{
	VertixAttrib va;
	BufferLayout layout(8);



	shader.Bind();



	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);


	posAttrib = shader.getAttribLocation("texCord");
	layout.Push<float>(posAttrib, 2, false);



	va.AddBuffer(vb, layout);
	glm::mat4 modelx = glm::mat4(1.0f);
	modelx = glm::scale(modelx, glm::vec3(.5, .5, .5));
	modelx = glm::translate(modelx, glm::vec3(1., -1., 0));

	shader.setUniform4fv("model", modelx);
	shader.setUniform1i("texSampler_1", tex);


	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

}


void ProcessInput()
{
	float cameraSpeed = 4.f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		isExit = true;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isTap) {
		isTap = true;
		createNewProjectile = true;
		PlaySound(TEXT("gunshot.wav"), NULL, SND_FILENAME | SND_ASYNC);
		temp = lightColor;
		lightColor = glm::vec3(1.0, 1.0, 0.0);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && isTap) {
		isTap = false;
		createNewProjectile = false;
		lightColor = temp;
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		kolor = (kolor + 1) % 4;
		Sleep(1000);
	}

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);

}

//  whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

	camera.ProcessMouseScroll(yoffset);

}