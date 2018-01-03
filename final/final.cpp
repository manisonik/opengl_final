// CS 465
// Jan 24, 2011
// Demonstration of Shadow Maps
// Author:

#define EARTH_RADIUS 5
#define SUN_RADIUS 10
#define SUN_ORBIT_RADIUS 1000
#define MOON_RADIUS 1.25
#define MOON_ORBIT_RADIUS 10
#define MOON_ANGLE_INCLINATION 5.15
#define MOON_PERIOD 0.748
#define PI 3.14159265

// Textures
#define NUM_TEXTURES 8
#define TILE_FLOOR_TEX 0
#define TERRAIN_TEX 1
#define SHADOW_TEX 2
#define BUILDING_TEX 3
#define GREENHOUSE_TEX 4
#define TILE_FLOOR_TEX 5
#define SOLAR_TEX 6
#define METAL_TEX 7

#define PI 3.14159
#define DELTA 1.5f;
#include <iostream>
#include "../shared/gltools.h"	// OpenGL toolkit
#include "../shared/math3d.h"
#include "Mesh.h"

using namespace std;

float tx = 0.0f;
float ty = 0.0f;
float tz = 0.0f;
float ta = 0.1;

// Camera variables
int mouseLastx;
int mouseLasty;
float cam_x;
float cam_y;
float cam_z;
float cam_rot_x;
float cam_rot_y;
float cam_rot_z;
float strafe;
float walk;
bool mouseDown = false;
bool lookUp;
bool lookDown;
bool lookLeft;
bool lookRight;

// Window turbine rotation
float bladesRot = 0;

// Test values
GLfloat bdist = 64.5;
GLfloat offsetFillX = 1.1f;
GLfloat offsetFillY = 1024.0f;
GLfloat nearPlane = 1000.0f;
GLfloat farPlane = 2000.0f;

GLfloat sunCenter[] = { 0.0, 0.0f, 0.0f };
GLfloat	 sunPos[] = { 0.0f, 200.0f, 180.0f, 1.0f };
GLfloat  sunSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f};
GLfloat  sunDiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f};
GLfloat  earthSpecRef[] =  { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  moonSpecRef[] =  { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat  ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat  specref[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  full[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat  fogColor[] = { 0.59, 0.2, 0.0, 0.01 };

// Light values
GLfloat  dim[] = { 0.2f, 0.2f, 0.2f, 0.2f };
GLfloat  dark[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat  bright[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLUquadric *orbitQuad;
GLUquadric *planetQuad;
GLfloat sunOrbitAngle = 0;
GLfloat moonOrbitAngle = 0;
GLint maxTexSize;                       // maximum allowed size for 1D/2D texture
GLuint tex[NUM_TEXTURES];

// Six sides of a cube map
const char *szCubeFaces[6] = {
	"pos_x.tga",
	"neg_x.tga",
	"pos_y.tga",
	"neg_y.tga",
	"pos_z.tga",
	"neg_z.tga"
};

GLenum  cube[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

M3DMatrix44f textureMatrix;
GLint windowWidth = 1024;               // window size
GLint windowHeight = 512;
GLint shadowWidth = 4096;               // set based on window size
GLint shadowHeight = 4096;
GLint shadowMapSize = 2048;

// Meshes
Mesh* meshTerrain;
Mesh* meshCube;
Mesh* meshBuilding;
Mesh* meshGreenHouseStructure;
Mesh* meshGreenHouseGlass;
Mesh* meshGreenHouseFloor;
Mesh* meshSolarPanel;
Mesh* meshWindTurbineBlades;
Mesh* meshWindTurbineTower;

// Matrices
GLfloat lightViewMatrix[16], lightProjectionMatrix[16], cameraProjectionMatrix[16], cameraViewMatrix[16], cameraPosition[3];

void Reflect() {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	/* Don't update color or depth. */
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	/* Draw 1 into the stencil buffer. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	/* Now drawing the floor just tags the floor pixels
	as stencil value 1. */
	glPushMatrix();
	glScalef(10.0f, 10.0f, 10.0f);
	glTranslatef(0.0f, 0.1f, 0.0f);
	glRotatef(90.0f, 0.0, 1.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glMateriali(GL_FRONT, GL_SHININESS, 256);
	glMaterialfv(GL_FRONT, GL_SPECULAR, full);
	glBindTexture(GL_TEXTURE_2D, tex[TILE_FLOOR_TEX]);
	meshGreenHouseFloor->Draw();
	glPopMatrix();

	/* Re-enable update of color and depth. */
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	/* Now, only render where stencil is set to 1. */
	glStencilFunc(GL_EQUAL, 1, 0xffffffff);  /* draw if stencil ==1 */
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glPushMatrix();
		glScalef(10, -10, 10);
		glTranslatef(0.0f, -0.1f, 0.0f);
		glRotatef(90.0f, 0.0, 1.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[GREENHOUSE_TEX]);
		glDisable(GL_CULL_FACE);
		meshGreenHouseStructure->Draw();
		glEnable(GL_CULL_FACE);
	glPopMatrix();

	glDisable(GL_STENCIL_TEST);

	glActiveTexture(GL_TEXTURE0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();
		glScalef(10.0f, 10.0f, 10.0f);
		glTranslatef(0.0f, 0.1f, 0.0f);
		glRotatef(90.0f, 0.0, 1.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[TILE_FLOOR_TEX]);
		meshGreenHouseFloor->Draw();
		glPopMatrix();
	glDisable(GL_BLEND);

	glPushMatrix();
		glDisable(GL_CULL_FACE);
		glScalef(10.0f, 10.0f, 10.0f);
		glTranslatef(0.0f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0, 1.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[GREENHOUSE_TEX]);
		meshGreenHouseStructure->Draw();
		glEnable(GL_CULL_FACE);
	glPopMatrix();

	// Activate texture 1
	glActiveTexture(GL_TEXTURE1);
}

void drawModels(bool inShadow) {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	////////////////////////////////////////////////////
	// Draw Solar Panel
	glPushMatrix();
		glScalef(5.0f, 5.0f, 5.0f);
		glTranslatef(-10.0, 0.0f, -15.0f);
		glColor4f(1, 1, 1, 0.5f);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[SOLAR_TEX]);
		meshSolarPanel->Draw();
		glTranslatef(0.0, 0.0f, 5.0f);
		meshSolarPanel->Draw();
		glTranslatef(0.0, 0.0f, 5.0f);
		meshSolarPanel->Draw();
		glTranslatef(0.0, 0.0f, 5.0f);
		meshSolarPanel->Draw();
		glTranslatef(0.0, 0.0f, 5.0f);
		meshSolarPanel->Draw();
		glTranslatef(0.0, 0.0f, 5.0f);
		meshSolarPanel->Draw();
	glPopMatrix();

	////////////////////////////////////////////////////
	// Draw windturbine
	glPushMatrix();
		glScalef(5.0f, 5.0f, 5.0f);
		glTranslatef(-4.0f, 0.0f, 4.8f);
		glRotatef(-48, 0.0f, 1.0f, 0.0f);
		glPushMatrix();	
			glColor3f(1, 1, 1);
			glMateriali(GL_FRONT, GL_SHININESS, 128);
			glMaterialfv(GL_FRONT, GL_SPECULAR, full);
			glBindTexture(GL_TEXTURE_2D, tex[METAL_TEX]);
			meshWindTurbineTower->Draw();
		glPopMatrix();

		glPushMatrix();
			bladesRot = fmodf(bladesRot + 1, 360.0);
			glTranslatef(1.09, 6.62, 0.007f);
			glRotatef(bladesRot, 1.0f, 0.0f, 0.0f);
			glColor3f(1, 1, 1);
			glMateriali(GL_FRONT, GL_SHININESS, 128);
			glMaterialfv(GL_FRONT, GL_SPECULAR, full);
			glBindTexture(GL_TEXTURE_2D, tex[METAL_TEX]);
			meshWindTurbineBlades->Draw();
		glPopMatrix();
	glPopMatrix();

	////////////////////////////////////////////////////
	// Draw building 1
	glPushMatrix();
		glScalef(20.0f, 20.0f, 20.0f);
		glTranslatef(0.0f, 0.0f, 2.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1, 1, 1);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[BUILDING_TEX]);
		meshBuilding->Draw();
	glPopMatrix();

	////////////////////////////////////////////////////
	// Draw building 2
	glPushMatrix();
		glScalef(20.0f, 20.0f, 20.0f);
		glTranslatef(0.0f, 0.0f, -2.0f);
		glColor3f(1, 1, 1);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[BUILDING_TEX]);
		meshBuilding->Draw();
	glPopMatrix();

	glPushMatrix();
		glDisable(GL_CULL_FACE);
		glScalef(10.0f, 10.0f, 10.0f);
		glTranslatef(0.0f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0, 1.0f, 0.0f);
		glColor3f(1, 1, 1);
		glMateriali(GL_FRONT, GL_SHININESS, 256);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[GREENHOUSE_TEX]);
		meshGreenHouseStructure->Draw();
		glEnable(GL_CULL_FACE);
	glPopMatrix();

	// Activate texture 1
	glActiveTexture(GL_TEXTURE1);
}

void UpdateCamera() {
	GLfloat hMov = DELTA;
	GLfloat vMov = 0;

	if (lookDown) {
		cam_rot_x -= DELTA;
		if (cam_rot_x <= -90)
			cam_rot_x = -90;
	}

	if (lookUp) {
		cam_rot_x += DELTA;
		if (cam_rot_x >= 90)
			cam_rot_x = 90;
	}

	if (lookLeft) {
		cam_rot_y -= DELTA;
		if (cam_rot_y <= 0)
			cam_rot_y = 360;
	}

	if (lookRight) {
		cam_rot_y += DELTA;
		if (cam_rot_y >= 360)
			cam_rot_y = 0;
	}

	hMov = cos(cam_rot_x * PI / 180) * DELTA;
	vMov = sin(cam_rot_x * PI / 180) * DELTA;
	cam_x += walk * sin(cam_rot_y * PI / 180) * hMov;
	cam_z += -walk * cos(cam_rot_y * PI / 180) * hMov;
	cam_y += walk * vMov;

	cam_x += strafe * sin((cam_rot_y + 90) * PI / 180) * DELTA;
	cam_z += -strafe * cos((cam_rot_y + 90) * PI / 180) * DELTA;
}

void mouseMovement(int x, int y) {
	if (mouseDown) {
		int mouseDiffx = x - mouseLastx;
		int mouseDiffy = y - mouseLasty;
		mouseLastx = x, mouseLasty = y;
		cam_rot_x -= ((GLfloat)mouseDiffy) * DELTA;
		cam_rot_y += ((GLfloat)mouseDiffx) * DELTA;

		if (cam_rot_x >= 90)
			cam_rot_x = 90;
		if (cam_rot_x <= -90)
			cam_rot_x = -90;
	}
}

void LoadMatrices() {
	glPushMatrix();
		glLoadIdentity();
		gluPerspective(45.0f, (float)windowWidth / windowHeight, 1.0f, 2000.0f);
		glGetFloatv(GL_PROJECTION_MATRIX, cameraProjectionMatrix);

		glLoadIdentity();
		gluLookAt(cam_x, cam_y, cam_z,
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f);
		glGetFloatv(GL_MODELVIEW_MATRIX, cameraViewMatrix);

		glLoadIdentity();
		gluPerspective(45.0f, 1.0f, 2.0f, 8.0f);
		glGetFloatv(GL_MODELVIEW_MATRIX, lightProjectionMatrix);

		glLoadIdentity();
		gluLookAt(sunPos[0], sunPos[1], sunPos[2],
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f);
		glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMatrix);
	glPopMatrix();
}

// Called to regenerate the shadow map
void RegenerateShadowMap(void)
{
	GLfloat lightToSceneDistance, fieldOfView, nearPlane;
    GLfloat lightModelview[16], lightProjection[16];
    GLfloat sceneBoundingRadius = bdist;

    // Save the depth precision for where it's useful
    lightToSceneDistance = sqrt(sunPos[0] * sunPos[0] + 
                                sunPos[1] * sunPos[1] + 
                                sunPos[2] * sunPos[2]);
    nearPlane = lightToSceneDistance - sceneBoundingRadius;

    // Keep the scene filling the depth texture
    fieldOfView = (GLfloat) m3dRadToDeg(2.0f * atan(sceneBoundingRadius / lightToSceneDistance));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fieldOfView, 1.0f, nearPlane, nearPlane + (2.0f * sceneBoundingRadius));
    glGetFloatv(GL_PROJECTION_MATRIX, lightProjection);

    // Switch to light's point of view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(sunPos[0], sunPos[1], sunPos[2], 
		0, 0, 0, 0.0f, 1.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightModelview);

    glViewport(0, 0, windowWidth, windowHeight);

	// Clear the depth buffer only
	glClear(GL_DEPTH_BUFFER_BIT);

    // All we care about here is resulting depth values
	glCullFace(GL_FRONT);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);
    glColorMask(0, 0, 0, 0);

    // Overcome imprecision
    glEnable(GL_POLYGON_OFFSET_FILL);

    // Draw objects in the scene except base plane
    // which never shadows anything
	drawModels(false);

    // Copy depth values into depth texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                     0, 0, windowWidth, windowHeight, 0);

    // Restore normal drawing state
    glShadeModel(GL_FLAT);
	glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glDisable(GL_POLYGON_OFFSET_FILL);
	glColorMask(1, 1, 1, 1);

    // Set up texture matrix for shadow map projection,
    // which will be rolled into the eye linear
    // texture coordinate generation plane equations
    M3DMatrix44f tempMatrix;
    m3dLoadIdentity44(tempMatrix);
    m3dTranslateMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
    m3dScaleMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
    m3dMatrixMultiply44(textureMatrix, tempMatrix, lightProjection);
    m3dMatrixMultiply44(tempMatrix, textureMatrix, lightModelview);

    // transpose to get the s, t, r, and q rows for plane equations
    m3dTransposeMatrix44(textureMatrix, tempMatrix);
}

void DrawRotatingLight() {
	glPushMatrix();
	glTranslatef(sunPos[0], sunPos[1], sunPos[2]);
	glColor3f(1, 0.8, 0);
	glDisable(GL_LIGHTING);
		glutSolidSphere(10, 20, 20);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void DrawSkyBox(void) {
	GLfloat fExtent = 512;
	glEnable(GL_TEXTURE_CUBE_MAP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBegin(GL_QUADS);
	//////////////////////////////////////////////
	// Negative X
	glTexCoord3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-fExtent, -fExtent, fExtent);

	glTexCoord3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-fExtent, -fExtent, -fExtent);

	glTexCoord3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-fExtent, fExtent, -fExtent);

	glTexCoord3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-fExtent, fExtent, fExtent);


	///////////////////////////////////////////////
	//  Postive X
	glTexCoord3f(1.0f, -1.0f, -1.0f);
	glVertex3f(fExtent, -fExtent, -fExtent);

	glTexCoord3f(1.0f, -1.0f, 1.0f);
	glVertex3f(fExtent, -fExtent, fExtent);

	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(fExtent, fExtent, fExtent);

	glTexCoord3f(1.0f, 1.0f, -1.0f);
	glVertex3f(fExtent, fExtent, -fExtent);


	////////////////////////////////////////////////
	// Negative Z 
	glTexCoord3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-fExtent, -fExtent, -fExtent);

	glTexCoord3f(1.0f, -1.0f, -1.0f);
	glVertex3f(fExtent, -fExtent, -fExtent);

	glTexCoord3f(1.0f, 1.0f, -1.0f);
	glVertex3f(fExtent, fExtent, -fExtent);

	glTexCoord3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-fExtent, fExtent, -fExtent);


	////////////////////////////////////////////////
	// Positive Z 
	glTexCoord3f(1.0f, -1.0f, 1.0f);
	glVertex3f(fExtent, -fExtent, fExtent);

	glTexCoord3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-fExtent, -fExtent, fExtent);

	glTexCoord3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-fExtent, fExtent, fExtent);

	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(fExtent, fExtent, fExtent);


	//////////////////////////////////////////////////
	// Positive Y
	glTexCoord3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-fExtent, fExtent, fExtent);

	glTexCoord3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-fExtent, fExtent, -fExtent);

	glTexCoord3f(1.0f, 1.0f, -1.0f);
	glVertex3f(fExtent, fExtent, -fExtent);

	glTexCoord3f(1.0f, 1.0f, 1.0f);
	glVertex3f(fExtent, fExtent, fExtent);


	///////////////////////////////////////////////////
	// Negative Y
	glTexCoord3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-fExtent, -fExtent, -fExtent);

	glTexCoord3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-fExtent, -fExtent, fExtent);

	glTexCoord3f(1.0f, -1.0f, 1.0f);
	glVertex3f(fExtent, -fExtent, fExtent);

	glTexCoord3f(1.0f, -1.0f, -1.0f);
	glVertex3f(fExtent, -fExtent, -fExtent);
	glEnd();
	glDisable(GL_TEXTURE_CUBE_MAP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Display() {
	// Update the camera
	UpdateCamera();

	///////////////////////////////////////////////////////////////////////////////////
	//First pass - from light's point of view
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(lightProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(lightViewMatrix);

	//Use viewport the same size as the shadow map
	glViewport(0, 0, windowWidth, windowHeight);

	// Draw back faces into the shadow map
	glCullFace(GL_FRONT);

	//Disable color writes, and use flat shading for speed
	glShadeModel(GL_FLAT);
	glColorMask(0, 0, 0, 0);

	// Draw Scene
	drawModels(false);

	// Read the depth buffer into the shadow map texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[SHADOW_TEX]);
	//glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, windowWidth, windowHeight);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		0, 0, shadowMapSize, shadowMapSize, 0);

	// Restore states
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	glColorMask(1, 1, 1, 1);

	///////////////////////////////////////////////////////////////////////////////////
	// 2nd pass - Draw from camera's point of view
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset projection matrix stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Reset the perpective
	gluPerspective(45, (float)windowWidth / (float)windowHeight, 1, 2000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Rotate Camera
	glRotatef(-cam_rot_x, 1, 0, 0);
	glRotatef(cam_rot_y, 0, 1, 0);

	// Enable texture 0 to show skybox
	glActiveTexture(GL_TEXTURE0);

	// Draw the skybox
	DrawSkyBox();

	// Enable Texture 1
	glActiveTexture(GL_TEXTURE1);

	// Translate Camera
	glTranslatef(-cam_x, -cam_y, -cam_z);
	glTranslatef(-50, -6, 0);

	glViewport(0, 0, windowWidth, windowHeight);

	// Show the light at sun position
	glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, dim);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dim);
	glLightfv(GL_LIGHT0, GL_SPECULAR, dark);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	// Draw objects
	drawModels(false);

	///////////////////////////////////////////////////////////////////////////////////
	// 3rd pass
	//Draw with bright light
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dim);
	glLightfv(GL_LIGHT0, GL_SPECULAR, dim);

	M3DMatrix44f tempMatrix;
	m3dLoadIdentity44(tempMatrix);
	m3dTranslateMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
	m3dScaleMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
	m3dMatrixMultiply44(textureMatrix, tempMatrix, lightProjectionMatrix);
	m3dMatrixMultiply44(tempMatrix, textureMatrix, lightViewMatrix);

	// transpose to get the s, t, r, and q rows for plane equations
	m3dTransposeMatrix44(textureMatrix, tempMatrix);

	// Set up texture coordinate generation.
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_S, GL_EYE_PLANE, &textureMatrix[0]);
	glEnable(GL_TEXTURE_GEN_S);

	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_T, GL_EYE_PLANE, &textureMatrix[4]);
	glEnable(GL_TEXTURE_GEN_T);

	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_R, GL_EYE_PLANE, &textureMatrix[8]);
	glEnable(GL_TEXTURE_GEN_R);

	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_Q, GL_EYE_PLANE, &textureMatrix[12]);
	glEnable(GL_TEXTURE_GEN_Q);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[SHADOW_TEX]);
	glEnable(GL_TEXTURE_2D);

	// Set alpha test to discard false comparisons
	glAlphaFunc(GL_GEQUAL, 0.99f);
	glEnable(GL_ALPHA_TEST);

	glActiveTexture(GL_TEXTURE0);
	glPushMatrix();
		//glScalef(0.5f, 0.5f, 0.5f);
		glTranslatef(0.0f, 0.0f, 0.0f);
		glColor4f(1, 1, 1, 1.0f);
		glMateriali(GL_FRONT, GL_SHININESS, 80);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[TERRAIN_TEX]);
		meshTerrain->Draw();
		glPopMatrix();
	glActiveTexture(GL_TEXTURE1);

	drawModels(false);

	//Disable textures and texgen
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);

	//Restore other states
	glDisable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);

	glutSwapBuffers();
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
{
	// Update the camera
	UpdateCamera();

	// Regenerate shadow map
	RegenerateShadowMap();

	// Reset the viewport
	glViewport(0, 0, windowWidth, windowHeight);
	
	// Reset projection matrix stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
   
	// Reset the perpective
	gluPerspective(45, (float)windowWidth/(float)windowHeight, 1, 2000);

	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Show the light at sun position
	glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, dim);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dim);
	glLightfv(GL_LIGHT0, GL_SPECULAR, full);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set up shadow comparison
    glEnable(GL_TEXTURE_2D);

	// Reset Model view matrix stack
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Rotate Camera
	glRotatef(-cam_rot_x, 1, 0, 0);
	glRotatef(cam_rot_y, 0, 1, 0);

	// Enable texture 0 to show skybox
	glActiveTexture(GL_TEXTURE0);

	// Draw the skybox
	DrawSkyBox();

	// Enable Texture 1
	glActiveTexture(GL_TEXTURE1);

	// Translate Camera
	glTranslatef(-cam_x, -cam_y, -cam_z);

	glTranslatef(-50, -6, 0);

	//Set up texture coordinate generation.
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_S, GL_EYE_PLANE, &textureMatrix[0]);
	glEnable(GL_TEXTURE_GEN_S);

	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_T, GL_EYE_PLANE, &textureMatrix[4]);
	glEnable(GL_TEXTURE_GEN_T);

	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_R, GL_EYE_PLANE, &textureMatrix[8]);
	glEnable(GL_TEXTURE_GEN_R);

	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_Q, GL_EYE_PLANE, &textureMatrix[12]);
	glEnable(GL_TEXTURE_GEN_Q);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	#ifdef DEBUG_BUILD
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	DrawRotatingLight();
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	#endif

	Reflect();

	glActiveTexture(GL_TEXTURE0);
	glPushMatrix();
		glTranslatef(0.0f, 0.0f, 0.0f);
		glColor4f(1, 1, 1, 1.0f);
		glMateriali(GL_FRONT, GL_SHININESS, 80);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[TERRAIN_TEX]);
		meshTerrain->Draw();
	glPopMatrix();
	glActiveTexture(GL_TEXTURE1);

    // Draw objects in the scene, including base plane
    drawModels(true);
  
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
	glEnable(GL_NORMALIZE);

	glActiveTexture(GL_TEXTURE0);
		glPushMatrix();
		glScalef(10.0f, 10.0f, 10.0f);
		glTranslatef(0.0f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0, 1.0f, 0.0f);
		glColor4f(1, 1, 1, 0.5f);
		glMateriali(GL_FRONT, GL_SHININESS, 80);
		glMaterialfv(GL_FRONT, GL_SPECULAR, full);
		glBindTexture(GL_TEXTURE_2D, tex[GREENHOUSE_TEX]);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		meshGreenHouseGlass->Draw();
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glPopMatrix();
	glActiveTexture(GL_TEXTURE1);

	// Flush drawing commands
	glutSwapBuffers();

	glutPostRedisplay();
}

void TimerFunction(int value){
	// Redraw the scene with new coordinates
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}

void SetupFog() {
	glFogi(GL_FOG_MODE, GL_LINEAR);     // Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor);    // Set Fog Color
	glFogf(GL_FOG_DENSITY, 0.0f);       // How Dense Will The Fog Be
	glHint(GL_FOG_HINT, GL_DONT_CARE);  // Fog Hint Value
	glFogf(GL_FOG_START, -500.0f);      // Fog Start Depth
	glFogf(GL_FOG_END, 800.0f);         // Fog End Depth
	glEnable(GL_FOG);                   // Enables GL_FOG
}

///////////////////////////////////////////////////////////
// Setup the rendering context
void SetupRC(void)
{
	GLbyte *pBytes;
	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;

	//Check for necessary extensions
	if (!GLEE_ARB_depth_texture || !GLEE_ARB_shadow) {
		printf("I require ARB_depth_texture and ARB_shadow extensionsn\n");
	}

	// Set color shading model to flat
	glShadeModel(GL_SMOOTH);

	// White background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Set drawing color to green
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Depth states
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	// Counter clock-wise polygons face out
	glFrontFace(GL_CCW);

	// Do not try to display the back sides
	glEnable(GL_CULL_FACE);

	// Polygon offset
    glPolygonOffset(offsetFillX, offsetFillY);

    // Setup and enable light 0
    /*glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR, sunSpecular);
    glLightfv(GL_LIGHT0,GL_POSITION, sunPos);
	glEnable(GL_LIGHT0);*/

	// Enable lighting
	glEnable(GL_LIGHTING);
	
    // Set Material properties to follow glColor values and
	// Enable color tracking
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, full);
    glMateriali(GL_FRONT, GL_SHININESS, 16.0f);
	glEnable(GL_COLOR_MATERIAL);

	// We use glScale when drawing the scene
    glEnable(GL_NORMALIZE);

    // Set up some texture state that never changes
    glGenTextures(NUM_TEXTURES, tex);

	// Load solar panel texture
	glBindTexture(GL_TEXTURE_2D, tex[SOLAR_TEX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("solar.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Load terrain texture
	glBindTexture(GL_TEXTURE_2D, tex[TERRAIN_TEX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("terrain.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Load greenhouse texture
	glBindTexture(GL_TEXTURE_2D, tex[GREENHOUSE_TEX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("greenhouse.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Load building texture
	glBindTexture(GL_TEXTURE_2D, tex[BUILDING_TEX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("building.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Load metal texture
	glBindTexture(GL_TEXTURE_2D, tex[METAL_TEX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("metal07.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Load terrain texture
	glBindTexture(GL_TEXTURE_2D, tex[TILE_FLOOR_TEX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pBytes = gltLoadTGA("tile02.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Load Skybox
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
	for (int i = 0; i < 6; i++) {
		// Load this texture map
		pBytes = gltLoadTGA(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);

		glTexImage2D(cube[i], 0, GL_RGBA, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
		free(pBytes);
	}

	// Load Cube Map images
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Shadow map
	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex[SHADOW_TEX]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE); //LUMINANCE: R=G=B=I, A=1.0
																		 //INTENSITY: R=G=B=A=L

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	//M3DVector4f eyePlane;
	//m3dLoadVector4(eyePlane, 1.0f, 0.0f, 0.0f, 0.0f);
	//glTexGenfv(GL_S, GL_EYE_PLANE, eyePlane);

	SetupFog();
	//RegenerateShadowMap();
}

void ChangeSize(int w, int h)
{
    GLint i;
    
	windowWidth = w; //= shadowWidth = w;
	windowHeight = h; // = shadowHeight = h;

	// Set Viewport to window dimensions
    glViewport(0, 0, w, h);

	// Reset projection matrix stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Establish clipping volume (left, right, bottom, top, near, far)
	gluPerspective(45,(float)w/(float)h, 1, 1000);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraProjectionMatrix);

	// Reset Model view matrix stack
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboardFunc(UCHAR key, int x, int y) {
	switch (key)
	{
	case 'W':
		walk = 1;
		break;
	case 'S':
		walk = -1;
		break;
	case 'A':
		strafe = -1;
		break;
	case 'D':
		strafe = 1;
		break;
	case 'w':
		walk = 1;
		break;
	case 's':
		walk = -1;
		break;
	case 'a':
		strafe = -1;
		break;
	case 'd':
		strafe = 1;
		break;
	case 'f':
		//bdist += 0.5f;
		//std::cout << "Distance: " << bdist << std::endl;
		sunCenter[2] += 1;
		break;
	case 'g':
		//bdist -= 0.5f;
		//std::cout << "Distance: " << bdist << std::endl;
		sunCenter[2] -= 1;
		break;
	case 'z':
		tx += ta;
		break;
	case 'v':
		tx -= ta;
		break;
	case 'x':
		ty += ta;
		break;
	case 'b':
		ty -= ta;
	case 'c':
		tz += ta;
		break;
	case 'n':
		tz -= ta;
	default:
		break;
	}
}

void keyboardUpFunc(UCHAR key, int x, int y)
{
	switch (key)
	{
	case 'W':
		walk = 0;
		break;
	case 'S':
		walk = 0;
		break;
	case 'A':
		strafe = 0;
		break;
	case 'D':
		strafe = 0;
		break;
	case 'w':
		walk = 0;
		break;
	case 's':
		walk = 0;
		break;
	case 'a':
		strafe = 0;
		break;
	case 'd':
		strafe = 0;
		break;
	case 'r':
		bdist += 0.5f;
		std::cout << "Distance: " << bdist << std::endl;
		break;
	case 't':
		bdist -= 0.5f;
		std::cout << "Distance: " << bdist << std::endl;
		break;
	case 'f':
		break;
	case 'g':
		break;
	case 'y':
		offsetFillX += 0.1f;
		glPolygonOffset(offsetFillX, offsetFillY);
		std::cout << "Polygon Offset X: " << offsetFillX << std::endl;
		std::cout << "Polygon Offset Y: " << offsetFillY << std::endl;
		break;
	case 'u':
		offsetFillX -= 0.1f;
		glPolygonOffset(offsetFillX, offsetFillY);
		std::cout << "Polygon Offset X: " << offsetFillX << std::endl;
		std::cout << "Polygon Offset Y: " << offsetFillY << std::endl;
	case 'i':
		offsetFillY += 0.1f;
		glPolygonOffset(offsetFillX, offsetFillY);
		std::cout << "Polygon Offset X: " << offsetFillX << std::endl;
		std::cout << "Polygon Offset Y: " << offsetFillY << std::endl;
		break;
	case 'o':
		offsetFillY -= 0.1f;
		glPolygonOffset(offsetFillX, offsetFillY);
		std::cout << "Polygon Offset X: " << offsetFillX << std::endl;
		std::cout << "Polygon Offset Y: " << offsetFillY << std::endl;
		break;
	default:
		break;
	}
}

void mouseClicks(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (mouseDown) {
			mouseDown = false;
		}
		else {
			mouseDown = true;
		}
	}
}

void LoadMesh() {
	meshTerrain = new Mesh();
	meshTerrain->loadMesh("terrain.obj");

	meshCube = new Mesh();
	meshCube->loadMesh("cube.obj");

	meshBuilding = new Mesh();
	meshBuilding->loadMesh("building.obj");

	meshGreenHouseStructure = new Mesh();
	meshGreenHouseStructure->loadMesh("greenhouseStructure.obj");

	meshGreenHouseGlass = new Mesh();
	meshGreenHouseGlass->loadMesh("greenhouse.obj");

	meshGreenHouseFloor = new Mesh();
	meshGreenHouseFloor->loadMesh("greenhouseFloor.obj");

	meshSolarPanel = new Mesh();
	meshSolarPanel->loadMesh("solar_panel.obj");

	meshWindTurbineTower = new Mesh();
	meshWindTurbineTower->loadMesh("windturbineTower.obj");

	meshWindTurbineBlades = new Mesh();
	meshWindTurbineBlades->loadMesh("windturbineBlades.obj");
}

///////////////////////////////////////////////////////////
// Main program entry point
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
 	glutCreateWindow("Shadow Maps");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutKeyboardUpFunc(keyboardUpFunc);
	glutKeyboardFunc(keyboardFunc);
	glutPassiveMotionFunc(mouseMovement);
	glutMouseFunc(mouseClicks);
	glutTimerFunc(33, TimerFunction, 1);

	LoadMesh();
	SetupRC();
	
	glutMainLoop();
    
    return 0;
}

