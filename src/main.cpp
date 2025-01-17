#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include <cstdio>

#include "Skybox.h"

#include <fstream>
#include <iterator>
#include <vector>
#include "stb_image.h"

#include "Shader_Loader.h"
#include "Render_Utils.h"

#include "Box.cpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Physics.h"
#include "Texture.h"
#include "CustomCamera.h"
#include "CustomMouseController.h"


GLuint program, programSun, programSkybox, programColor, programTexture;
GLuint textureEarth, textureEarthNormal;
GLuint textureAsteroid, textureAsteroidNormal;
GLuint textureSun, textureSunNormal;
GLuint textureMoon, textureMoonNormal;
GLuint textureMars, textureMarsNormal;
GLuint textureVenus, textureVenusNormal;
GLuint shipTexture, shipTextureNormal;

float frustumScale = 1.f;
Physics pxScene(9.8);
const double physicsStepTime = 1.f / 90.0f;
double physicsTimeToProcess = 0;
PxRigidDynamic *shipBody = nullptr, *sunBody = nullptr, *earthBody = nullptr, *moonBody = nullptr, *marsBody = nullptr, *venusBody = nullptr;
PxMaterial* shipMaterial = nullptr, *sunMaterial = nullptr, *earthMaterial = nullptr, *moonMaterial = nullptr, *marsMaterial = nullptr, *venusMaterial = nullptr;
obj::Model sphereModel, shipModel;
Core::Shader_Loader shaderLoader;
Core::RenderContext sphereContext, shipContext, sunContext;
float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-6, 0, 0), cameraDir, lightPos = glm::vec3(0.0f, 0.0f, 0.0f), cameraSide;
glm::mat4 cameraMatrix, perspectiveMatrix;
struct Renderable {
	Core::RenderContext* context;
    glm::mat4 modelMatrix;
    GLuint textureId;
	GLuint textureNormal;
};
Renderable *ship, *sun, *earth, *moon, *asteroids, *mars, *venus;

float horizontalDistance = 6.0f; // CustomCamera variable
float verticalDistance = 0.8f; // CustomCamera variable
float angleAroundPlayer; //
CustomCamera customCamera(cameraPos);
CustomMouseController customMouse(horizontalDistance, angleAroundPlayer);
float fov = 70.0f;

void mouse(int x, int y) {
	customMouse.mouseController(x, y);
	angleAroundPlayer = customMouse.getAngleAroundPlayer();
}

void mouseKeyController(int button, int state, int x, int y) {
	customMouse.mouseKeyController(button, state, x, y);
	angleAroundPlayer = customMouse.getAngleAroundPlayer();
	horizontalDistance = customMouse.getHorizontalDistance();
}
void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 1.0f;
	float moveSpeed = 1.0f;
	switch (key)
	{
	case 'z': angleAroundPlayer -= angleSpeed; break;
	case 'x': angleAroundPlayer += angleSpeed; break;
	case 'w': shipBody->addForce(PxVec3(cameraDir.x * -50.0f, 0, cameraDir.z * 50.0f), PxForceMode::eFORCE, true); break;
	case 's': shipBody->addForce(PxVec3(cameraDir.x * 50.0f, 0, cameraDir.z * -50.0f), PxForceMode::eFORCE, true); break;
	case 'd': shipBody->addForce(PxVec3(cameraSide.x * 25.0f, 0, cameraSide.z * -25.0f), PxForceMode::eFORCE, true); break;
	case 'a': shipBody->addForce(PxVec3(cameraSide.x * -25.0f, 0, cameraSide.z * 25.0f), PxForceMode::eFORCE, true); break;
	case 'e': shipBody->addTorque(PxVec3(0, -100.f, 0), PxForceMode::eFORCE, true); break;
	case 'q': shipBody->addTorque(PxVec3(0, 100.f, 0), PxForceMode::eFORCE, true); break;
	case ' ': shipBody->setAngularVelocity(PxVec3(0, 0, 0)); break;
	}
}

void initPhysicsScene(){
	sunMaterial = pxScene.physics->createMaterial(0.9f, 0.8f, 0.7f);
	sunBody = pxScene.physics->createRigidDynamic(PxTransform(0,0,0));
	PxShape *sunShape = pxScene.physics->createShape(PxSphereGeometry(20), *sunMaterial);
	sunBody->attachShape(*sunShape);
	sunShape->release();
	sunBody->setMass(10000);
	sunBody->setMassSpaceInertiaTensor(PxVec3(2000.0f, 2000.0f, 2000.0f));
	sunBody->userData = sun;
	sunBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	pxScene.scene->addActor(*sunBody);

	earthMaterial = pxScene.physics->createMaterial(0.9f, 0.8f, 0.7f);
	earthBody = pxScene.physics->createRigidDynamic(PxTransform(0,0,50));
	PxShape *earthShape = pxScene.physics->createShape(PxSphereGeometry(10), *earthMaterial);
	earthBody->attachShape(*earthShape);
	earthShape->release();
	earthBody->setMass(1000);
	earthBody->setAngularVelocity(PxVec3(0 ,1, 0));
	earthBody->setMassSpaceInertiaTensor(PxVec3(2000.0f, 2000.0f, 2000.0f));
	earthBody->userData = earth;
	earthBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	pxScene.scene->addActor(*earthBody);

	moonMaterial = pxScene.physics->createMaterial(0.9f, 0.8f, 0.7f);
	moonBody = pxScene.physics->createRigidDynamic(PxTransform(50,0,50));
	PxShape *moonShape = pxScene.physics->createShape(PxSphereGeometry(1), *moonMaterial);
	moonBody->attachShape(*moonShape);
	moonShape->release();
	moonBody->setMass(1000);
	moonBody->setAngularVelocity(PxVec3(0 ,1, 0));
	moonBody->setMassSpaceInertiaTensor(PxVec3(2000.0f, 2000.0f, 2000.0f));
	moonBody->userData = moon;
	moonBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	pxScene.scene->addActor(*moonBody);

	marsMaterial = pxScene.physics->createMaterial(0.9f, 0.8f, 0.7f);
	marsBody = pxScene.physics->createRigidDynamic(PxTransform(0, 0, 100));
	PxShape* marsShape = pxScene.physics->createShape(PxSphereGeometry(15), *marsMaterial);
	marsBody->attachShape(*marsShape);
	marsShape->release();
	marsBody->setMass(1000);
	marsBody->setAngularVelocity(PxVec3(0, 1, 0));
	marsBody->setMassSpaceInertiaTensor(PxVec3(2000.0f, 2000.0f, 2000.0f));
	marsBody->userData = mars;
	marsBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	pxScene.scene->addActor(*marsBody);

	venusMaterial = pxScene.physics->createMaterial(0.9f, 0.8f, 0.7f);
	venusBody = pxScene.physics->createRigidDynamic(PxTransform(0, 0, 100));
	PxShape* venusShape = pxScene.physics->createShape(PxSphereGeometry(7), *venusMaterial);
	venusBody->attachShape(*venusShape);
	venusShape->release();
	venusBody->setAngularVelocity(PxVec3(0, 1, 0));
	venusBody->setMass(1000);
	venusBody->setMassSpaceInertiaTensor(PxVec3(2000.0f, 2000.0f, 2000.0f));
	venusBody->userData = venus;
	venusBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	pxScene.scene->addActor(*venusBody);

	shipBody = pxScene.physics->createRigidDynamic(PxTransform(0, 0, 80));
    shipMaterial = pxScene.physics->createMaterial(0.82f, 0.8f, 0.f);
    PxShape* shipShape = pxScene.physics->createShape(PxBoxGeometry(1.25f, 0.3f, 1.25f), *shipMaterial);
    shipBody->attachShape(*shipShape);
    shipShape->release();
    shipBody->setMass(5);
    shipBody->setMassSpaceInertiaTensor(PxVec3(0.f, 250.0f, 0.f));
    //spaceshipBody->setRigidDynamicLockFlags(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z | PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
    shipBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    shipBody->userData = ship;

    pxScene.scene->addActor(*shipBody);
}

void initRenderables(){
	textureSun = Core::LoadTexture("textures/Sun/sunTex.png");
	textureEarth = Core::LoadTexture("textures/Earth/earth2.png");
	textureEarthNormal = Core::LoadTexture("textures/Earth/earth2_normals.png");
	textureMoon = Core::LoadTexture("textures/Moon/moon2.png");
	textureMoonNormal = Core::LoadTexture("textures/Moon/moon_normal.png");
	textureAsteroid = Core::LoadTexture("textures/Asteroid/asteroid.png");
	textureAsteroidNormal = Core::LoadTexture("textures/Asteroid/asteroid_normals.png");
	textureMars = Core::LoadTexture("textures/Planet/mars.png");
	textureMarsNormal = Core::LoadTexture("textures/Planet/mars_normal.png");;
	textureVenus = Core::LoadTexture("textures/Planet/venus.png");
	//textureVenusNormal = Core::LoadTexture("textures/Planet/venus_normal.png");

	shipTextureNormal = Core::LoadTexture("textures/StarSparrow_Normal.png");
	shipTexture = Core::LoadTexture("textures/StarSparrow_Blue.png");
	
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/StarSparrow02.obj");
	shipContext.initFromOBJ(shipModel);
	sunContext.initFromOBJ(sphereModel);

	sun = new Renderable();
	sun->textureId = textureSun;
	sun->textureNormal = textureEarthNormal;
	sun->context = &sunContext;
	
	earth = new Renderable();
	earth->textureId = textureEarth;
	earth->textureNormal = textureEarthNormal;
	earth->context = &sphereContext;
	
	moon = new Renderable();
	moon->textureId = textureMoon;
	moon->textureNormal = textureMoonNormal;
	moon->context = &sphereContext;
	
	mars = new Renderable();
	mars->textureId = textureMars;
	mars->textureNormal = textureMarsNormal;
	mars->context = &sphereContext;
	
	venus = new Renderable();
	venus->textureId = textureVenus;
	venus->textureNormal = textureMarsNormal;
	venus->context = &sphereContext;
	
	ship = new Renderable();
	ship->textureId = shipTexture;
	ship->textureNormal = shipTextureNormal;
	ship->context = &shipContext;
}

glm::mat4 createCameraMatrix(PxRigidActor* actor, float angleAroundPlayer) {

    glm::mat4 cameraMatrix = customCamera.createCustomCameraMatrix(actor, horizontalDistance, verticalDistance, angleAroundPlayer);
    cameraDir = customCamera.getCameraDir();
    cameraSide = customCamera.getCameraSide();
    cameraPos = customCamera.getCameraPos();
    return cameraMatrix;

}

void updateTransforms()
{
    // Here we retrieve the current transforms of the objects from the physical simulation.
    auto actorFlags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
    PxU32 nbActors = pxScene.scene->getNbActors(actorFlags);
    if (nbActors)
    {
        std::vector<PxRigidActor*> actors(nbActors);
        pxScene.scene->getActors(actorFlags, (PxActor**)&actors[0], nbActors);
        for (auto actor : actors)
        {
            // We use the userData of the objects to set up the model matrices
            // of proper renderables.
            if (!actor->userData) continue;
            Renderable *renderable = (Renderable*)actor->userData;

            // get world matrix of the object (actor)
            PxMat44 transform = actor->getGlobalPose();
            auto &c0 = transform.column0;
            auto &c1 = transform.column1;
            auto &c2 = transform.column2;
            auto &c3 = transform.column3;

            // set up the model matrix used for the rendering
            glm::mat4 modelMatrix = glm::mat4(
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w);
            
			if (actor->userData == earth) modelMatrix = modelMatrix * glm::scale(glm::vec3(10.0f));
			if (actor->userData == sun) modelMatrix = modelMatrix * glm::scale(glm::vec3(20.0f));
            if (actor->userData == ship) modelMatrix = modelMatrix * glm::scale(glm::vec3(0.1f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)); // IMPORTANT!
			if (actor->userData == mars) modelMatrix = modelMatrix * glm::scale(glm::vec3(15.f));
			if (actor->userData == venus) modelMatrix = modelMatrix * glm::scale(glm::vec3(7.f));

            renderable->modelMatrix = modelMatrix;
        }
    }
}

void renderScene()
{
	PxU32 nbActors = pxScene.scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
    if (nbActors) {
        // CAMERA WILL BE ATTACHED TO THE LAST ACTOR ADDED TO THE PHYSX SCENE!
        std::vector<PxRigidActor*> actors(nbActors);
        pxScene.scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, (PxActor**)&actors[0], nbActors);
        PxRigidActor* actor = actors.back();

        cameraMatrix = createCameraMatrix(actor, angleAroundPlayer);
    }
	perspectiveMatrix = customCamera.createPerspectiveMatrix(0.1f, 10000.f, fov, frustumScale);
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

	pxScene.step(physicsStepTime);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	earthBody->setAngularVelocity(PxVec3(0 ,1, 0));
	moonBody->setAngularVelocity(PxVec3(0 ,1, 0));
	marsBody->setAngularVelocity(PxVec3(0 ,1, 0));
	venusBody->setAngularVelocity(PxVec3(0 ,1, 0));

	glm::mat4 rotate1, rotate2, rotate3, rotate4, moonRotate;
	rotate1 = glm::rotate((time / 360.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate2 = glm::rotate((time / 300.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate3 = glm::rotate((time / 240.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	rotate4 = glm::rotate((time / 6.0f) * 2 * 3.14159f, glm::vec3(0.0f, 2.0f, 0.0f));
	moonRotate = glm::rotate((time / 15.0f) * 2 * 3.14159f, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 moonScale, sunScale, planetScale1, planetScale2, planetScale3;
	moonScale = glm::scale(glm::vec3(15, 15, 15));
	sunScale = glm::scale(glm::vec3(1.5, 1.5, 1.5));
	planetScale1 = glm::scale(glm::vec3(500, 500, 500));
	planetScale2 = glm::scale(glm::vec3(300, 300, 300));
	planetScale3 = glm::scale(glm::vec3(100, 100, 100));

	updateTransforms();

	glm::mat4 earthPos = rotate3 * planetScale2;
	glm::mat4 moonPos = moonRotate * glm::translate(glm::vec3(0.25, 0.5, 1.5)) * moonScale;
	glm::mat4 marsPos = rotate1 * planetScale1;
	glm::mat4 venusPos = rotate2 * planetScale3;

	std::cout << moonPos[0].x << " " << moonPos[0].y << " " << moonPos[0].z << '\n';
	earthBody->setGlobalPose(PxTransform(earthPos[0].x, earthPos[0].y, earthPos[0].z));
	moonBody->setGlobalPose(PxTransform(earthPos[0].x + moonPos[0].x, earthPos[0].y + moonPos[0].y, earthPos[0].z + moonPos[0].z));
	marsBody->setGlobalPose(PxTransform(marsPos[0].x, marsPos[0].y, marsPos[0].z));
	venusBody->setGlobalPose(PxTransform(venusPos[0].x, venusPos[0].y, venusPos[0].z));
	//earth->modelMatrix = rotate3 * glm::translate(glm::vec3(0, 0, 20)) * rotate4;
	//moon->modelMatrix =  rotate3 * glm::translate(glm::vec3(0, 0, 20)) * moonRotate * glm::translate(glm::vec3(0.25, 0.5, 1.5)) * moonScale;
	//mars->modelMatrix = rotate2 * glm::translate(glm::vec3(0, 0, 25)) * planetScale1 * rotate4;
	//venus->modelMatrix = rotate1 * glm::translate(glm::vec3(0, 0, -10)) * planetScale3 * rotate4;

	renderSkybox(programSkybox, cameraMatrix, perspectiveMatrix);
	
	Core::drawObjectTextureSun(programSun, &sphereModel, sun->modelMatrix, sun->textureId, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	Core::drawObjectTexture(programTexture, &sphereModel, earth->modelMatrix, earth->textureId, earth->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	Core::drawObjectTexture(programTexture, &sphereModel, moon->modelMatrix, moon->textureId, moon->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	Core::drawObjectTexture(programTexture, &sphereModel, mars->modelMatrix, mars->textureId, mars->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	Core::drawObjectTexture(programTexture, &sphereModel, venus->modelMatrix, venus->textureId, venus->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	//Core::drawObjectTexture(programTexture, &sphereModel, asteroids->modelMatrix, asteroids->textureId, asteroids->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	Core::drawObjectTexture(programTexture, &shipModel, ship->modelMatrix, ship->textureId, ship->textureNormal, cameraMatrix, perspectiveMatrix, cameraPos, lightPos);
	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programSun = shaderLoader.CreateProgram("shaders/shader_sun_tex.vert", "shaders/shader_sun_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_texture.vert", "shaders/shader_texture.frag");

	initRenderables();
	initPhysicsScene();
	initSkybox();
}

void shutdown()
{
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

void onReshape(int width, int height)
{
	frustumScale = (float)width / height;

	glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(2560, 1440);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);

	glutMouseFunc(mouseKeyController);
	glutMotionFunc(mouse);

	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);
	glutReshapeFunc(onReshape);

	glutMainLoop();

	shutdown();

	return 0;
}
