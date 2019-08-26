#include "Renderer.h"
#include <iostream>

btVector3 Cube::setBulletV3(glm::vec3 gV3)
{	
	return  { gV3.x , gV3.y , gV3.z };
}

Cube::Cube()
{
}

Cube::Cube(btDiscreteDynamicsWorld* world,float length,glm::vec3 pos,float mass):m_Pos(pos),m_Mass(mass)
{
	m_BtPos = setBulletV3(m_Pos);

	HFlength = length * 0.5;

	float positions[] = {
		-HFlength, -HFlength, HFlength, 0.0, 0.0, //0
		 HFlength, -HFlength, HFlength, 1.0, 0.0, //1
		 HFlength,  HFlength, HFlength, 1.0, 1.0, //2
		-HFlength,  HFlength, HFlength, 0.0, 1.0, //3
		-HFlength, -HFlength,-HFlength, 1.0, 0.0, //4
		 HFlength, -HFlength,-HFlength, 0.0, 0.0, //5
		 HFlength,  HFlength,-HFlength, 0.0, 1.0, //6
		-HFlength,  HFlength,-HFlength, 1.0, 1.0, //7
	    -HFlength,  HFlength,-HFlength, 0.0, 0.0, //8
		 HFlength,  HFlength,-HFlength, 1.0, 0.0, //9
		-HFlength, -HFlength,-HFlength, 0.0, 1.0, //10
		 HFlength, -HFlength,-HFlength, 1.0, 1.0  //11
	};

	unsigned int indices[] = {
		0, 3, 2,   //1
		0, 1, 2,

		1, 2, 6,   //2
		1, 6, 5,

		7, 6, 4,   //3
		4, 6, 5,

		0, 3, 7,   //4
		0, 4, 7,

		3, 8, 9,   //5
		3, 9, 2,

		0, 10, 11,  //6
		0, 1, 11

	};

	vb.InitVertexBuffer(positions,sizeof(positions));
	ib.InitIndexBuffer(indices, 36);
	auto box = new btBoxShape({ HFlength,HFlength,HFlength });
	btTransform startTransform;
	startTransform.setIdentity();
	btVector3 localInertia(0, 0, 0);

	if (m_Mass > 0)
		box->calculateLocalInertia(m_Mass, localInertia);

	startTransform.setOrigin(m_BtPos);
	auto motionstate = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionstate, box, localInertia);
	m_rigid = new btRigidBody(rbInfo);
	world->addRigidBody(m_rigid);

}


void Cube::Bind()
{
	vb.Bind();
	ib.Bind();
}

void Cube::Draw(Shader& shader, glm::mat4& view, glm::mat4& projection,int tex)
{
	Bind();
	shader.Bind();
	VertixAttrib va;
	BufferLayout layout(5);
	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);
	posAttrib = shader.getAttribLocation("texCord");
	layout.Push<float>(posAttrib, 2, false);
	va.AddBuffer(vb, layout);
	glm::mat4 model;
	double btMat4[16] = {};
	float a[16];
	auto trans = m_rigid->getWorldTransform();
	trans.getOpenGLMatrix((btScalar*)&btMat4);
	for (int i = 0; i < 16; i++) {
		a[i] = (float)btMat4[i];
	}
	model = glm::make_mat4(a);
	glm::mat4 mvp = projection * view * model;
	shader.setUniform4fv("mvp", mvp);
	shader.setUniform1i("texSampler",tex);
	m_BtPos = m_rigid->getCenterOfMassPosition();
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

}

btVector3 Cube::getBulletV3()
{
	return m_BtPos;
}

glm::vec3 Cube::getPositionOfCube()
{
	return glm::vec3((float)m_BtPos[0] , (float)m_BtPos[1] , (float)m_BtPos[2]);
}

btVector3 GLRectangle::setBulletV3(glm::vec3 gV3)
{
	return  { gV3.x , gV3.y , gV3.z };
}

GLRectangle::GLRectangle(btDiscreteDynamicsWorld* world, float length,float breadth, glm::vec3 pos, float mass, Orientation orientation):m_Pos(pos), m_Mass(mass),m_Orientation(orientation)
{
	m_BtPos = setBulletV3(m_Pos);
	lightPos = glm::vec3(0.0, 50.0, 0.0);
	HFlength = length * 0.5;
	HFbreadth = breadth * 0.5;
	float Nz = 0.0, Ny = 0.0, Nx = 0.0;
	float y = 0.0, z = 0.0, x = 0.0;
	float cy = 0.0, cz = 0.0, cx = 0.0,z1 = 0.0;
	if (Orientation::ZX == m_Orientation) {
		Ny = 1.0;
		cy = 0.1;
		cx = HFlength;
		cz = HFbreadth;
		x = HFlength;
		y = 0.0;
		z = HFbreadth;
		z1 = z;
	}
	if (Orientation::ZY == m_Orientation) {
		Nx = 1.0;
		cx = 0.1;
		cz = HFlength;
		cy = HFbreadth;
		z = HFlength;
		x = 0.0;
		y = HFbreadth;
		z1 = -z;
	}
	if (Orientation::XY == m_Orientation) {
		Nz = 1.0;
		cz = 0.1;
		cx = HFlength;
		cy = HFbreadth;
		x = HFlength;
		z = 0.0;
		y = HFbreadth;
		z1 = z;
	}
	float positions[] = {
		//Coordinates //TexCords  //Normal
	   -x, -y,  -z1,  0.0, 0.0,   Nx, Ny, Nz, //0
		x, -y,  -z,   1.0, 0.0,   Nx, Ny, Nz, //1
		x,  y,   z1,  1.0, 1.0,   Nx, Ny, Nz, //2
	   -x,  y,   z,   0.0, 1.0,   Nx, Ny, Nz  //3
	};

	
	unsigned int indices[] = {
		0, 3, 2,   //1
		0, 1, 2
	};

	vb.InitVertexBuffer(positions, sizeof(positions));
	ib.InitIndexBuffer(indices, 6);
	auto box = new btBoxShape({ cx, cy, cz });
	btTransform startTransform;
	startTransform.setIdentity();
	btVector3 localInertia(0, 0, 0);

	if (m_Mass > 0)
		box->calculateLocalInertia(m_Mass, localInertia);

	startTransform.setOrigin(m_BtPos);
	auto motionstate = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionstate, box, localInertia);
	m_rigid = new btRigidBody(rbInfo);
	world->addRigidBody(m_rigid);
}

void GLRectangle::Bind()
{
	vb.Bind();
	ib.Bind();
}

void GLRectangle::Draw(Shader& shader, glm::mat4& view, glm::mat4& projection, int tex,Camera & camera, glm::vec3& lc, float& as)
{
	Bind();
	shader.Bind();
	VertixAttrib va;
	BufferLayout layout(8);
	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);
	posAttrib = shader.getAttribLocation("texCord");
	layout.Push<float>(posAttrib, 2, false);
	posAttrib = shader.getAttribLocation("aNormal");
	layout.Push<float>(posAttrib, 3, false);
	va.AddBuffer(vb, layout);
	glm::mat4 model;
	double btMat4[16] = {};
	float a[16];
	auto trans = m_rigid->getWorldTransform();
	trans.getOpenGLMatrix((btScalar*)& btMat4);
	for (int i = 0; i < 16; i++) {
		a[i] = (float)btMat4[i];
	}
	
	model = glm::make_mat4(a);
	glm::mat4 mvp = projection * view * model;
	shader.setUniform4fv("mvp", mvp);
	shader.setUniform4fv("model", model);
	shader.setUniform1i("texSampler_1", tex);
	shader.setUniform3fv("lightColor", lc);
	shader.setUniform3fv("lightPos", lightPos);
	shader.setUniform3fv("viewPos", camera.Position);
	shader.setUniform1f("ambientStrength", as);
	glDrawElements(GL_TRIANGLES, 6 , GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
}

btVector3 GLRectangle::getBulletV3()
{
	return m_BtPos;
}

btVector3 Projectile::setBulletV3(glm::vec3 gV3)
{
	return  { gV3.x , gV3.y , gV3.z };
}

Projectile::Projectile()
{
	
}

Projectile::Projectile(btDiscreteDynamicsWorld* world, glm::vec3 pos, float mass,float speed,glm::vec3 velocity):m_Pos(pos),m_Speed(speed)
{
	m_BtPos = setBulletV3(pos);
	bHlength = 0.2 * 0.5;
	

	float positions[] = {
		-bHlength, -bHlength, bHlength, //0
		 bHlength, -bHlength, bHlength, //1
		 bHlength,  bHlength, bHlength, //2
		-bHlength,  bHlength, bHlength, //3
		-bHlength, -bHlength,-bHlength, //4
		 bHlength, -bHlength,-bHlength, //5
		 bHlength,  bHlength,-bHlength, //6
		-bHlength,  bHlength,-bHlength, //7
		-bHlength,  bHlength,-bHlength, //8
		 bHlength,  bHlength,-bHlength, //9
		-bHlength, -bHlength,-bHlength, //10
		 bHlength, -bHlength,-bHlength  //11
	};

	unsigned int indices[] = {
		1, 3, 2,   //1
		1, 3, 0,

		1, 2, 6,   //2
		1, 6, 5,

		7, 6, 4,   //3
		4, 6, 5,

		0, 3, 7,   //4
		0, 4, 7,

		3, 8, 9,   //5
		3, 9, 2,

		0, 10, 11,  //6
		0, 1, 11

	};

	vb.InitVertexBuffer(positions, sizeof(positions));
	ib.InitIndexBuffer(indices, 36);
	auto box = new btBoxShape({ bHlength,bHlength,bHlength });
	btTransform startTransform;
	startTransform.setIdentity();
	btVector3 localInertia(0, 0, 0);

	if (m_Mass > 0)
		box->calculateLocalInertia(m_Mass, localInertia);

	startTransform.setOrigin(m_BtPos);
	auto motionstate = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionstate, box, localInertia);
	m_rigid = new btRigidBody(rbInfo);
	m_rigid->setLinearVelocity({ m_Speed * velocity.x, m_Speed * velocity.y, m_Speed * velocity.z });
	world->addRigidBody(m_rigid);
}

void Projectile::RemoveRigidBody(btDiscreteDynamicsWorld* world)
{
	world->removeRigidBody(m_rigid);
	delete m_rigid->getMotionState();
	delete m_rigid;
	m_rigid = NULL;
}



void Projectile::Bind()
{
	vb.Bind();
	ib.Bind();
}

void Projectile::Draw(Shader& shader, glm::mat4& view, glm::mat4& projection)
{
	Bind();
	shader.Bind();
	VertixAttrib va;
	BufferLayout layout(3);
	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);
	va.AddBuffer(vb, layout);
	glm::mat4 model;
	double btMat4[16] = {};
	float a[16];
	auto trans = m_rigid->getWorldTransform();
	trans.getOpenGLMatrix((btScalar*)& btMat4);
	for (int i = 0; i < 16; i++) {
		a[i] = (float)btMat4[i];
	}
	model = glm::make_mat4(a);
    glm::mat4 mvp = projection * view * model;
	shader.setUniform4fv("mvp", mvp);
	m_BtPos = m_rigid->getCenterOfMassPosition();
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
}

btVector3 Projectile::getBulletV3()
{
	return m_BtPos;
}

CrossHair::CrossHair()
{

	float bHlength = .01;
	float positions[] = {
		-bHlength/2., -bHlength, 0.0, //0
		 bHlength/2., -bHlength, 0.0, //1
		 bHlength/2.,  bHlength, 0.0, //2
		-bHlength/2.,  bHlength, 0.0 //3
	};

	unsigned int indices[] = {
		1, 3, 2,   //1
		1, 3, 0,
	};
	vb.InitVertexBuffer(positions, sizeof(positions));
	ib.InitIndexBuffer(indices, 6);

}

void CrossHair::Bind()
{
	vb.Bind();
	ib.Bind();
}

void CrossHair::Draw(Shader& shader)
{
	Bind();
	shader.Bind();
	VertixAttrib va;
	BufferLayout layout(3);
	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);
	va.AddBuffer(vb, layout);
	glm::mat4 mvp = glm::mat4(1.0);
	shader.setUniform4fv("mvp", mvp);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
}

btVector3 EnemyProjectile::setBulletV3(glm::vec3 gV3)
{
	return  { gV3.x , gV3.y , gV3.z };
}

EnemyProjectile::EnemyProjectile()
{
}

EnemyProjectile::EnemyProjectile(btDiscreteDynamicsWorld* world, glm::vec3 pos, float mass, float speed, glm::vec3 velocity):m_Pos(pos), m_Speed(speed)
{
	m_BtPos = setBulletV3(pos);
	bHlength = 1. * 0.5;


	float positions[] = {
		-bHlength, -bHlength, bHlength, 0.0, 0.0, //0
		 bHlength, -bHlength, bHlength, 1.0, 0.0,//1
		 bHlength,  bHlength, bHlength, 1.0, 1.0,//2
		-bHlength,  bHlength, bHlength, 0.0, 1.0,//3
		-bHlength, -bHlength,-bHlength, 1.0, 0.0,//4
		 bHlength, -bHlength,-bHlength, 0.0, 0.0,//5
		 bHlength,  bHlength,-bHlength, 0.0, 1.0,//6
		-bHlength,  bHlength,-bHlength, 1.0, 1.0,//7
		-bHlength,  bHlength,-bHlength, 0.0, 0.0,//8
		 bHlength,  bHlength,-bHlength, 1.0, 0.0,//9
		-bHlength, -bHlength,-bHlength, 0.0, 1.0,//10
		 bHlength, -bHlength,-bHlength, 1.0, 1.0 //11
	};

	unsigned int indices[] = {
		1, 3, 2,   //1
		1, 3, 0,

		1, 2, 6,   //2
		1, 6, 5,

		7, 6, 4,   //3
		4, 6, 5,

		0, 3, 7,   //4
		0, 4, 7,

		3, 8, 9,   //5
		3, 9, 2,

		0, 10, 11,  //6
		0, 1, 11

	};

	vb.InitVertexBuffer(positions, sizeof(positions));
	ib.InitIndexBuffer(indices, 36);
	auto box = new btBoxShape({ bHlength,bHlength,bHlength });
	btTransform startTransform;
	startTransform.setIdentity();
	btVector3 localInertia(0, 0, 0);

	if (m_Mass > 0)
		box->calculateLocalInertia(m_Mass, localInertia);

	startTransform.setOrigin(m_BtPos);
	auto motionstate = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionstate, box, localInertia);
	m_rigid = new btRigidBody(rbInfo);
	
	m_rigid->setLinearVelocity({ m_Speed * velocity.x, m_Speed * velocity.y, m_Speed * velocity.z });
	world->addRigidBody(m_rigid);
}

void EnemyProjectile::RemoveRigidBody(btDiscreteDynamicsWorld* world)
{
	world->removeRigidBody(m_rigid);
	delete m_rigid->getMotionState();
	delete m_rigid;
	m_rigid = NULL;
}

void EnemyProjectile::Bind()
{
	vb.Bind();
	ib.Bind();
}

void EnemyProjectile::Draw(Shader& shader, glm::mat4& view, glm::mat4& projection,int tex)
{
	Bind();
	shader.Bind();
	VertixAttrib va;
	BufferLayout layout(5);
	GLint posAttrib = shader.getAttribLocation("position");
	layout.Push<float>(posAttrib, 3, false);
	posAttrib = shader.getAttribLocation("texCord");
	layout.Push<float>(posAttrib, 2, false);
	va.AddBuffer(vb, layout);
	glm::mat4 model;
	double btMat4[16] = {};
	float a[16];
	auto trans = m_rigid->getWorldTransform();
	trans.getOpenGLMatrix((btScalar*)& btMat4);
	for (int i = 0; i < 16; i++) {
		a[i] = (float)btMat4[i];
	}
	model = glm::make_mat4(a);
	glm::mat4 mvp = projection * view * model;
	shader.setUniform4fv("mvp", mvp);
	shader.setUniform1i("texSampler", tex);
	m_BtPos = m_rigid->getCenterOfMassPosition();

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
}

btVector3 EnemyProjectile::getBulletV3()
{
	return m_BtPos;
}

glm::vec3 EnemyProjectile::getPositionOfCube()
{
	return glm::vec3((float)m_BtPos[0], (float)m_BtPos[1], (float)m_BtPos[2]);
}

