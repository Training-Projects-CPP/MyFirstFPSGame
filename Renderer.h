#pragma once
#include <GL/glew.h>
#include "Buffer.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <memory>
#include "Camera.h"


class Cube {
private:
	float HFlength;
	VertexBuffer vb;
	IndexBuffer ib;
	glm::vec3 m_Pos;
	btRigidBody* m_rigid = 0;
	float m_Mass;
	btVector3 m_BtPos;
	btVector3 setBulletV3(glm::vec3 gV3);
	

public:
	Cube();
	Cube(btDiscreteDynamicsWorld* world,float length,glm::vec3 pos,float mass);
	void Bind();
	void Draw(Shader& shader,glm::mat4& view, glm::mat4& projection,int tex);
	btVector3 getBulletV3();
	glm::vec3 getPositionOfCube();
};

enum class Orientation {
	XY = 0,
	ZX = 1,
	ZY = 2
};

class GLRectangle {
private:
	float HFlength;
	float HFbreadth;
	VertexBuffer vb;
	IndexBuffer ib;
	glm::vec3 m_Pos;
	btRigidBody* m_rigid = 0;
	float m_Mass;
	btVector3 m_BtPos;
	btVector3 setBulletV3(glm::vec3 gV3);
	Orientation m_Orientation;
	glm::vec3 lightPos;

public:
	GLRectangle(btDiscreteDynamicsWorld* world, float length, float breadth, glm::vec3 pos, float mass, Orientation orientation);
	void Bind();
	void Draw(Shader& shader, glm::mat4& view, glm::mat4& projection, int tex,Camera & camera,glm::vec3 & lc,float& as);
	btVector3 getBulletV3();
	void setLightPos(glm::vec3 lightPosition) {
		lightPos = lightPosition;
	}
};

class Projectile {
private:
	float bHlength;
	VertexBuffer vb;
	IndexBuffer ib;
	glm::vec3 m_Pos;
	btRigidBody* m_rigid = 0;
	float m_Mass;
	btVector3 m_BtPos;
	btVector3 setBulletV3(glm::vec3 gV3);
	float m_Speed;


public:
	Projectile();
	Projectile(btDiscreteDynamicsWorld* world, glm::vec3 pos, float mass,float speed,glm::vec3 velocity);
	void RemoveRigidBody(btDiscreteDynamicsWorld* world);
	void Bind();
	void Draw(Shader& shader, glm::mat4& view, glm::mat4& projection);
	btVector3 getBulletV3();
};


class CrossHair {
private:
	VertexBuffer vb;
	IndexBuffer ib;
	const glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
public:
	CrossHair();
	void Bind();
	void Draw(Shader &shader);

};


class EnemyProjectile {

private:
	float bHlength;
	VertexBuffer vb;
	IndexBuffer ib;
	glm::vec3 m_Pos;
	btRigidBody* m_rigid = 0;
	float m_Mass;
	btVector3 m_BtPos;
	btVector3 setBulletV3(glm::vec3 gV3);
	float m_Speed;


public:
	EnemyProjectile();
	EnemyProjectile(btDiscreteDynamicsWorld* world, glm::vec3 pos, float mass, float speed, glm::vec3 velocity);
	void RemoveRigidBody(btDiscreteDynamicsWorld* world);
	void Bind();
	void Draw(Shader& shader, glm::mat4& view, glm::mat4& projection,int tex);
	btVector3 getBulletV3();
	glm::vec3 getPositionOfCube();
};

