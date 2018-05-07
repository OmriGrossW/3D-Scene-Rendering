#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include "glut.h"
#include "SOIL.h"

#define PI 3.14159265
#define maxSize 65535
#define initialZaxisOrigin -100.0f

using namespace std;
typedef struct scene
{
	GLfloat origin[3];
	GLfloat rotateMat[16];
	GLfloat translateMat[16];
} scene;

typedef struct camera
{
	GLfloat origin[3];
	GLfloat up[3];
	GLfloat right[3];
	GLfloat towards[3];

	// translateMat & rotateMat 
	// [0] stores the amount to move/rotate by camera.right Vector
	// same goes for [1] to camera.up Vector and [2] to camera.towards Vector
	GLfloat rotateMat[16];
	GLfloat translateMat[16];

} camera;

typedef struct object
{
	GLuint firstFaceElementIndex;
	GLuint lastFaceElementIndex;
	GLuint firstFacesIndex;
	GLuint lastFacesIndex;
	GLuint name;
	GLfloat COM[3];		// Center of mass, actually an array of size 3 for x,y,z cordinates
	GLfloat COMvec[4];
	GLfloat Ka[4];
	GLfloat Kd[4];
	GLfloat Ks[4];

	//GLfloat selfCameraMatrix[16];
	//GLfloat selfSceneMatrix[16];
	//GLfloat selfObjectMatrix[16];
	GLfloat selfMatrix[16];
	GLfloat inverseMatrix[16];

	//	GLfloat selfTranslate[16];
	GLfloat CRM[16];
	GLfloat CTM[16];
	GLfloat SRM[16];
	GLfloat STM[16];
	GLfloat OSRM[16];
	GLfloat OSTM[16];
	GLfloat ORM[16];
	GLfloat OTM[16];
	GLuint textureIndex;

	//GLfloat shinnines[1];
}object;

FILE *f;
scene myScene;
camera myCamera;
GLfloat vertices[maxSize * 3];
GLfloat normals[maxSize * 2 * 3];
GLuint faces[maxSize * 4 * 3];
GLuint faceElements[maxSize * 2];

GLfloat perspectiveMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
GLfloat cameraRotateMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
GLfloat cameraTranslateMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
GLfloat sceneRotateMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
GLfloat sceneTranslateMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
GLfloat viewMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };

GLfloat negate[16] = { -1.0f, 0.0f, 0.0f , 0.0f, 0.0f, -1.0f , 0.0f, 0.0f, 0.0f , 0.0f,-1.0f, 0.0f, 0.0f, 0.0f , 0.0f, -1.0f };

// globalRotateMat sceneModeRotateMatrix sceneModeScaleMatrix
// [0] stores the amount to rotate\move by X axis
// same goes for [1] to Y axis and [2] to Z axis
GLfloat globalRotateMat[3] = { 0.0f, 0.0f, 0.0f };
GLfloat mat_Ka[] = { 0.3f, 0.4f, 0.5f, 1.0f };
GLfloat mat_Kd[] = { 0.0f, 0.6f, 0.7f, 1.0f };
GLfloat mat_Ks[] = { 0.0f, 0.0f, 0.8f, 1.0f };
GLfloat low_sh[] = { 5.0f };
object objects[maxSize];
GLuint objectNameToIndex[maxSize];
GLuint 	sceneWidth, sceneHeight;
int vIndex, vnIndex, fIndex, eIndex, oIndex, mouseStartX, mouseStartY;
bool cameraMode, objectMode, sceneMode, play, pressed;

//Ass 4 addition
GLuint texName[maxSize]; //array for textures
GLuint numberOfTextures = 0;
GLuint	selectedObjectIndex = -1;
GLfloat rotateValues[3] = { 0.0f, 0.0f, 0.0f };
GLfloat translateValues[3] = { 0.0f, 0.0f, 0.0f };

void reshape(int width, int height);
void drawAxis();
void drawAllPolygons(bool drawBack);
void drawSphere();

void display();
void disp(int value);

//void setSelfRotateScaleMatrices(int objInd);
void setProjectionMatrix();
void setCameraViewMatrices();
void setSceneMatrices();
void loadViewPerspectiveMatrix();
void multGlobalSceneMatrix();
void initLight();
void initCamera();
void init();
void objectReader();
void colorTableReader();

void setLightIntensity(bool sceneMode);
void setMaterial(int objIndex, bool drawBack);

void myTranslatef(GLfloat dx, GLfloat dy, GLfloat dz);
void myRotatef(GLfloat ang, GLfloat x, GLfloat y, GLfloat z);
void myScalef(GLfloat sx, GLfloat sy, GLfloat sz);

void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);

void calcCenterOfMass(int objectIndex);

//ass 4
int loadTexture(unsigned char * imageName, int textureIndex);
void drawFace(int i, int k, int objInd, bool drawBack);
bool objectModeCameraFrame, objectModeSceneFrame, objectModeObjectFrame;
void setSelfCameraViewMatrices(int objIndex);
void setSelfSceneMatrices(int objIndex);
void objectMinorModesActions(int button);
void multMatVec(GLfloat * mat, GLfloat * vec, GLfloat * resVec);

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	sceneWidth = 700;
	sceneHeight = 700;
	glutInitWindowSize(sceneWidth, sceneHeight);
	glutInitWindowPosition(400, 150);
	glViewport(0, 0, (GLsizei)sceneWidth, (GLsizei)sceneHeight);

	glutCreateWindow("Assignment4");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(2, disp, 0);

	glutMainLoop();

	return 0;
}

void disp(int value) {
	if (play)
		glutPostRedisplay();
	glutTimerFunc(5, disp, 0);
}

void display()
{
	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(perspectiveMatrix);

	glMatrixMode(GL_MODELVIEW);

	drawAxis();
	drawAllPolygons(false);				//false stays for NOT draweing back buffer... drawing normal front buffer
										/////////////////////	glutSwapBuffers();		///////////////////////////////////////////////////
	drawSphere();

	glFlush();
}

void drawAxis() {
	glDrawBuffer(GL_FRONT);

	GLfloat Ka[] = { 1.0f, 0.0f, 0.0f, 1.0f };	// initialized for RED color
	GLfloat Kd[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat Ks[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat axisSize = fabs(initialZaxisOrigin)/2.0f;

	setCameraViewMatrices();
	setSceneMatrices();

	loadViewPerspectiveMatrix();
	multGlobalSceneMatrix();

	glLineWidth(5);


	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Ks);
	glBegin(GL_LINES);
	// drawing X axis
	//glColor4f(1.0, 0.0, 0.0, 1.0f);
	glVertex4f(0.0, 0.0, 0.0, 1.0f);
	glVertex4f(axisSize, 0.0, 0.0, 1.0f);
	glEnd();

	// changing Ka values for Y axis  - GREEN color
	Ka[0] = 0.0f;
	Ka[1] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ka);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Kd);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Ks);
	glBegin(GL_LINES);
	// drawing Y axis
	//	glColor4f(0.0, 1.0, 0.0, 1.0f);
	glVertex4f(0.0, 0.0, 0.0, 1.0f);
	glVertex4f(0.0, axisSize, 0.0, 1.0f);
	glEnd();

	// changing Ka values for Z axis  - BLUE color
	Ka[1] = 0.0f;
	Ka[2] = 1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ka);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Kd);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Ks);
	glBegin(GL_LINES);
	// drawing Z axis
	//	glColor4f(0.0, 0.0, 1.0, 1.0f);
	glVertex4f(0.0, 0.0, 0.0, 1.0f);
	glVertex4f(0.0, 0.0, axisSize, 1.0f);
	glEnd();

}

void drawAllPolygons(bool drawBack)
{
	int objInd = 0;
	glLoadIdentity();

	if (drawBack) {
		glDisable(GL_LIGHTING);
		glDrawBuffer(GL_BACK);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear back buffer	
	}
	else
		glDrawBuffer(GL_FRONT);
		
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0, k = 0; i<fIndex && k<eIndex; i += faceElements[k] * 2, k++)
	{
		if (k == objects[objInd].firstFaceElementIndex) {
		
			setSelfSceneMatrices(objInd);
			setSelfCameraViewMatrices(objInd);

			loadViewPerspectiveMatrix();

			multGlobalSceneMatrix();

			glMultMatrixf(objects[objInd].OSRM);
		//	glMultMatrixf(objects[objInd].OSTM);

			myTranslatef(objects[objInd].COM[0], objects[objInd].COM[1], objects[objInd].COM[2]);
			glMultMatrixf(objects[objInd].ORM);
		//	glMultMatrixf(objects[objInd].OTM);
		    myTranslatef(-objects[objInd].COM[0], -objects[objInd].COM[1], -objects[objInd].COM[2]);
		
			setMaterial(objInd, drawBack);

			objInd++;


		}
		drawFace(i, k, objInd - 1, drawBack);
	}
	if (drawBack)
		glEnable(GL_LIGHTING);

}

void drawFace(int i, int k, int objInd, bool drawBack) {

	if (objects[objInd].textureIndex != -1 && !drawBack) {
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, texName[objects[objInd].textureIndex]);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		if (faceElements[k] == 3) { //this face is a triangle
									//printf("triangle \n");
			glBegin(GL_TRIANGLES);

			glTexCoord2f(0.0, 0.0);
			//glNormal3f(normals[(faces[i + 0 + 1] - 1) * 3], normals[(faces[i + 0 + 1] - 1) * 3 + 1], normals[(faces[i + 0 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 0] - 1) * 3], vertices[(faces[i + 0] - 1) * 3 + 1], vertices[(faces[i + 0] - 1) * 3 + 2]);

			glTexCoord2f(0.0, 1.0);
			//glNormal3f(normals[(faces[i + 2 + 1] - 1) * 3], normals[(faces[i + 2 + 1] - 1) * 3 + 1], normals[(faces[i + 2 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 2] - 1) * 3], vertices[(faces[i + 2] - 1) * 3 + 1], vertices[(faces[i + 2] - 1) * 3 + 2]);

			glTexCoord2f(1.0, 1.0);
			//glNormal3f(normals[(faces[i + 4 + 1] - 1) * 3], normals[(faces[i + 4 + 1] - 1) * 3 + 1], normals[(faces[i + 4 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 4] - 1) * 3], vertices[(faces[i + 4] - 1) * 3 + 1], vertices[(faces[i + 4] - 1) * 3 + 2]);

			glEnd();
		}

		if (faceElements[k] == 4) { //this face is a quad
									//printf("Quad \n");
			glBegin(GL_QUADS);

			glTexCoord2f(0.0, 0.0);
			//glNormal3f(normals[(faces[i + 0 + 1] - 1) * 3], normals[(faces[i + 0 + 1] - 1) * 3 + 1], normals[(faces[i + 0 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 0] - 1) * 3], vertices[(faces[i + 0] - 1) * 3 + 1], vertices[(faces[i + 0] - 1) * 3 + 2]);

			glTexCoord2f(0.0, 1.0);
			//glNormal3f(normals[(faces[i + 2 + 1] - 1) * 3], normals[(faces[i + 2 + 1] - 1) * 3 + 1], normals[(faces[i + 2 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 2] - 1) * 3], vertices[(faces[i + 2] - 1) * 3 + 1], vertices[(faces[i + 2] - 1) * 3 + 2]);

			glTexCoord2f(1.0, 1.0);
			//glNormal3f(normals[(faces[i + 4 + 1] - 1) * 3], normals[(faces[i + 4 + 1] - 1) * 3 + 1], normals[(faces[i + 4 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 4] - 1) * 3], vertices[(faces[i + 4] - 1) * 3 + 1], vertices[(faces[i + 4] - 1) * 3 + 2]);

			glTexCoord2f(1.0, 0.0);
			//glNormal3f(normals[(faces[i + 6 + 1] - 1) * 3], normals[(faces[i + 6 + 1] - 1) * 3 + 1], normals[(faces[i + 6 + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + 6] - 1) * 3], vertices[(faces[i + 6] - 1) * 3 + 1], vertices[(faces[i + 6] - 1) * 3 + 2]);

			glEnd();
		}

		//glFlush();
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}
	else {

		glBegin(GL_POLYGON);

		for (GLuint j = 0; j < faceElements[k] * 2; j += 2)
		{
			glNormal3f(normals[(faces[i + j + 1] - 1) * 3], normals[(faces[i + j + 1] - 1) * 3 + 1], normals[(faces[i + j + 1] - 1) * 3 + 2]);
			glVertex3f(vertices[(faces[i + j] - 1) * 3], vertices[(faces[i + j] - 1) * 3 + 1], vertices[(faces[i + j] - 1) * 3 + 2]);
		}
		glEnd();
	}
	/*
	glBegin(GL_POLYGON);

	for (GLuint j = 0; j < faceElements[k] * 2; j += 2)
	{
	glNormal3f(normals[(faces[i + j + 1] - 1) * 3], normals[(faces[i + j + 1] - 1) * 3 + 1], normals[(faces[i + j + 1] - 1) * 3 + 2]);
	glVertex3f(vertices[(faces[i + j] - 1) * 3], vertices[(faces[i + j] - 1) * 3 + 1], vertices[(faces[i + j] - 1) * 3 + 2]);
	}
	glEnd();
	*/
}

void drawSphere() {
	if (selectedObjectIndex != -1) {		// if == -1 then no Object is being selected now.
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
		glDrawBuffer(GL_FRONT);
		setSelfSceneMatrices(selectedObjectIndex);
		setSelfCameraViewMatrices(selectedObjectIndex);
		glLoadIdentity();
		multGlobalSceneMatrix();

		glMultMatrixf(objects[selectedObjectIndex].OSRM);
	//	glMultMatrixf(objects[selectedObjectIndex].OSTM);

		myTranslatef(objects[selectedObjectIndex].COM[0], objects[selectedObjectIndex].COM[1], objects[selectedObjectIndex].COM[2]);
		glMultMatrixf(objects[selectedObjectIndex].ORM);
	//	glMultMatrixf(objects[selectedObjectIndex].OTM);
		myTranslatef(-objects[selectedObjectIndex].COM[0], -objects[selectedObjectIndex].COM[1], -objects[selectedObjectIndex].COM[2]);
		GLfloat currentModelView[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, currentModelView);
		multMatVec(currentModelView, objects[selectedObjectIndex].COM, objects[selectedObjectIndex].COMvec);

		glLoadIdentity();

		//glPushMatrix();
		myTranslatef(objects[selectedObjectIndex].COMvec[0], objects[selectedObjectIndex].COMvec[1], objects[selectedObjectIndex].COMvec[2]);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	/*	GLfloat Ka[] = { 1.0f, 0.0f, 0.0f, 1.0f };	// initialized for RED color
		GLfloat Kd[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat Ks[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ka);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Kd);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Ks);*/

		glutSolidSphere(0.3, 32, 32);
		
	//	glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_LIGHTING);

	}

}

void reshape(int width, int height)
{
	sceneWidth = width;
	sceneHeight = height;
	glViewport(0, 0, (GLsizei)sceneWidth, (GLsizei)sceneHeight);

	setProjectionMatrix();
}

void setSelfCameraViewMatrices(int objIndex) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(objects[objIndex].CTM);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraTranslateMatrix);

	glLoadIdentity();
	glLoadMatrixf(objects[objIndex].CRM);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraRotateMatrix);
}

void setSelfSceneMatrices(int objIndex) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(objects[objIndex].STM);
	glGetFloatv(GL_MODELVIEW_MATRIX, sceneTranslateMatrix);

	glLoadIdentity();
	glLoadMatrixf(objects[objIndex].SRM);
	glGetFloatv(GL_MODELVIEW_MATRIX, sceneRotateMatrix);
}

void setCameraViewMatrices() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(myCamera.translateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraTranslateMatrix);

	glLoadIdentity();
	glLoadMatrixf(myCamera.rotateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraRotateMatrix);

	
	/*glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	myRotatef(myCamera.rotateMat[0], myCamera.right[0], myCamera.right[1], myCamera.right[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraRotateMatrix);

	glLoadIdentity();
	myRotatef(myCamera.rotateMat[1], myCamera.up[0], myCamera.up[1], myCamera.up[2]);
	glMultMatrixf(cameraRotateMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraRotateMatrix);

	glLoadIdentity();
	myTranslatef(-myCamera.origin[0], -myCamera.origin[1], -myCamera.origin[2]);
	//glMultMatrixf(cameraRotateMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraTranslateMatrix);
	

	//glLoadIdentity();
	//	gluLookAt(myCamera.origin[0], myCamera.origin[1], myCamera.origin[2], myScene.origin[0], myScene.origin[1], myScene.origin[2], myCamera.up[0], myCamera.up[1], myCamera.up[2]);
	//glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
	*/
}

void setSceneMatrices() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(myScene.translateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, sceneTranslateMatrix);
	
	glLoadIdentity();
	glLoadMatrixf(myScene.rotateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, sceneRotateMatrix);
}

void setProjectionMatrix() {
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(60, (GLfloat)sceneWidth / (GLfloat)sceneHeight, 2.0, 500.0);
	glGetFloatv(GL_PROJECTION_MATRIX, perspectiveMatrix);
	glMatrixMode(GL_MODELVIEW);

}

void loadViewPerspectiveMatrix() {
	glLoadIdentity();
	glLoadMatrixf(perspectiveMatrix);
	glLoadMatrixf(viewMatrix);
}

void multGlobalSceneMatrix() {
	glMultMatrixf(cameraRotateMatrix);
	glMultMatrixf(cameraTranslateMatrix);
	glMultMatrixf(sceneTranslateMatrix);
	glMultMatrixf(sceneRotateMatrix);
}

void initLight() {
	GLfloat light_position[] = { 0,-1.0,0,0 };
	GLfloat light_shininess[] = { 5.0 };

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glClearColor(0, 0, 0, 1);  //black

	setLightIntensity(sceneMode);	//sceneMode is set to false as default then the lighting is normal
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SHININESS, light_shininess);

}

void initCamera() {
	// setting camera origin to 0,0,0 by assignment instructions. Drawing scene section a.
	myCamera.origin[0] = 0.0f;
	myCamera.origin[1] = 0.0f;
	myCamera.origin[2] = 0.0f;
	// setting camera Vectors , toward Vector calculated by scene center minus camera origin, normalized. by assignment instructions Drawing scene sections b. and a.
	myCamera.right[0] = 1.0f;
	myCamera.right[1] = 0.0f;
	myCamera.right[2] = 0.0f;
	myCamera.up[0] = 0.0f;
	myCamera.up[1] = 1.0f;
	myCamera.up[2] = 0.0f;
	myCamera.towards[0] = 0.0f;
	myCamera.towards[1] = 0.0f;
	myCamera.towards[2] = -1.0f;
	


}

void init()
{
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	glGenTextures(maxSize, texName);

	glReadBuffer(GL_BACK);


	myScene.origin[0] = 0.0f;
	myScene.origin[1] = 0.0f;
	myScene.origin[2] = initialZaxisOrigin;

	cameraMode = true;
	objectMode = false;
	sceneMode = false;


	initCamera();

	setProjectionMatrix();

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glGetFloatv(GL_MODELVIEW_MATRIX, myScene.translateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, myScene.rotateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, myCamera.translateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, myCamera.rotateMat);

	glLoadIdentity();
	myTranslatef(myScene.origin[0], myScene.origin[1], myScene.origin[2]);
	glMultMatrixf(myScene.translateMat);
	glGetFloatv(GL_MODELVIEW_MATRIX, myScene.translateMat);

	glLoadIdentity();
	gluLookAt(myCamera.origin[0], myCamera.origin[1], myCamera.origin[2], myScene.origin[0], myScene.origin[1], myScene.origin[2], myCamera.up[0], myCamera.up[1], myCamera.up[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);

	glLoadIdentity();

	initLight();
	objectReader();
	colorTableReader();

	play = true;
	pressed = false;
}

void myTranslatef(GLfloat dx, GLfloat dy, GLfloat dz) {
	GLfloat translateMatrix[16] = { 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f , 0.0f, 0.0f, 0.0f , 0.0f, 1.0f, 0.0f,  dx, dy, dz, 1.0f };
	glMultMatrixf(translateMatrix);
}

void myRotatef(GLfloat ang, GLfloat x, GLfloat y, GLfloat z) {
	GLfloat cosAlpha = cosf(ang);//(cosf(ang) * (float)180.0 / PI);
	GLfloat sinAlpha = sinf(ang);//(sinf(ang) * (float)180.0 / PI);
	GLfloat XrotateMatrix[16] = { 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, cosAlpha, sinAlpha, 0.0f, 0.0f , -sinAlpha, cosAlpha, 0.0f, 0.0f, 0.0f , 0.0f, 1.0f };
	GLfloat YrotateMatrix[16] = { cosAlpha, 0.0f, -sinAlpha, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, sinAlpha, 0.0f, cosAlpha, 0.0f, 0.0f, 0.0f , 0.0f, 1.0f };
	GLfloat ZrotateMatrix[16] = { cosAlpha, sinAlpha, 0.0f, 0.0f , -sinAlpha, cosAlpha,  0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f };

	if (x) {
		glMultMatrixf(XrotateMatrix);
	}
	if (y) {
		glMultMatrixf(YrotateMatrix);
	}
	if (z) {
		glMultMatrixf(ZrotateMatrix);
	}
}

void myScalef(GLfloat sx, GLfloat sy, GLfloat sz) {
	GLfloat scaleMatrix[16] = { sx, 0.0f, 0.0f , 0.0f, 0.0f,sy , 0.0f, 0.0f, 0.0f , 0.0f, sz, 0.0f,  0.0f, 0.0f , 0.0f, 1.0f };
	glMultMatrixf(scaleMatrix);
}

void objectMinorModesActions(int button){
	if (objectModeCameraFrame) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			glLoadIdentity();
			myRotatef(rotateValues[0], myCamera.right[0], myCamera.right[1], myCamera.right[2]);
			glMultMatrixf(objects[selectedObjectIndex].CRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].CRM);

			glLoadIdentity();
			myRotatef(rotateValues[1], myCamera.up[0], myCamera.up[1], myCamera.up[2]);
			glMultMatrixf(objects[selectedObjectIndex].CRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].CRM);
			break;
		case GLUT_RIGHT_BUTTON:
			glLoadIdentity();
			myTranslatef(translateValues[0] * myCamera.right[0], translateValues[0] * myCamera.right[1], translateValues[0] * myCamera.right[2]);
			myTranslatef(translateValues[1] * myCamera.up[0], translateValues[1] * myCamera.up[1], translateValues[1] * myCamera.up[2]);
			glMultMatrixf(objects[selectedObjectIndex].CRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].CRM);
			break;
		case GLUT_MIDDLE_BUTTON:
			glLoadIdentity();
			myTranslatef((-1.0f) * translateValues[2] * myCamera.towards[0], (-1.0f) * translateValues[2] * myCamera.towards[1], (-1.0f) * translateValues[2] * myCamera.towards[2]);
			glMultMatrixf(objects[selectedObjectIndex].CRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].CRM);
			break;
		}
	}
	if (objectModeSceneFrame) {
		switch (button) {
			
		case GLUT_LEFT_BUTTON:
			glLoadIdentity();
			//glLoadMatrixf(objects[selectedObjectIndex].SRM);
			myRotatef(rotateValues[0], 1, 0, 0);
			glMultMatrixf(objects[selectedObjectIndex].OSRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].OSRM);
			
			glLoadIdentity();
			//glLoadMatrixf(objects[selectedObjectIndex].SRM);
			myRotatef(rotateValues[1], 0, 1, 0);
			glMultMatrixf(objects[selectedObjectIndex].OSRM);

			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].OSRM);
			break;
		case GLUT_RIGHT_BUTTON:
			glLoadIdentity();
			//glLoadMatrixf(objects[selectedObjectIndex].STM);
			myTranslatef(translateValues[0], translateValues[1], 0);
			glMultMatrixf(objects[selectedObjectIndex].OSRM);

			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].OSRM);
			break;
		case GLUT_MIDDLE_BUTTON:
			glLoadIdentity();
			//glLoadMatrixf(objects[selectedObjectIndex].STM);
			myTranslatef(0, 0, translateValues[2]);
			glMultMatrixf(objects[selectedObjectIndex].OSRM);

			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].OSRM);
			break;
		}

	}
	if (objectModeObjectFrame)
	{
		switch (button) {

		case GLUT_LEFT_BUTTON:
			glLoadIdentity();
			glLoadMatrixf(objects[selectedObjectIndex].ORM);
			myRotatef(rotateValues[0], 1, 0, 0);
			//glMultMatrixf(objects[selectedObjectIndex].ORM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].ORM);

			glLoadIdentity();
			glLoadMatrixf(objects[selectedObjectIndex].ORM);
			myRotatef(rotateValues[1], 0, 1, 0);
			//glMultMatrixf(objects[selectedObjectIndex].ORM);

			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].ORM);
			break;
		case GLUT_RIGHT_BUTTON:
			glLoadIdentity();
			glLoadMatrixf(objects[selectedObjectIndex].ORM);
			myTranslatef(translateValues[0], translateValues[1], 0);
		//	glMultMatrixf(objects[selectedObjectIndex].ORM);

			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].ORM);
			break;
		case GLUT_MIDDLE_BUTTON:
			glLoadIdentity();
			glLoadMatrixf(objects[selectedObjectIndex].ORM);
			myTranslatef(0, 0, translateValues[2]);
		//	glMultMatrixf(objects[selectedObjectIndex].ORM);

			glGetFloatv(GL_MODELVIEW_MATRIX, objects[selectedObjectIndex].ORM);
			break;
		}
	}
}

void mouse(int button, int state, int x, int y) {
	if (pressed)	// On mouse release, mouse button up
	{
		GLfloat x_d = (GLfloat)(x - mouseStartX) / (GLfloat)sceneWidth;
		GLfloat y_d = (GLfloat)(y - mouseStartY) / (GLfloat)sceneHeight;
		//printf("Mouse pressed at : X,Y  %d,%d\n\t\tx_d =do %f , x_y = %f\n", x, y, (float)x_d, (float)y_d);
		GLuint objInd = 0;

		if (objectMode && selectedObjectIndex != -1) {
			switch (button) {
			case GLUT_LEFT_BUTTON:		// rotate the each object around its center (COM) according to mouse movement
				rotateValues[0] = -y_d*PI;
				rotateValues[1] = -x_d*PI;

				break;
			case GLUT_RIGHT_BUTTON:		// scale all object by 1.05 for each pixel the curser to the right and by 0.95 for each pixel it pass to the left
										// Using 1.01 and 0.99 are instead of 1.05 and 0.95 because the steps were too big 
				translateValues[0] = x_d*(GLfloat)75;
				translateValues[1] = -y_d*(GLfloat)75;

				break;

			case GLUT_MIDDLE_BUTTON:
				translateValues[2] = -y_d*(GLfloat)75;
				break;
			}
			objectMinorModesActions(button);
		}

		if (sceneMode) {
			switch (button) {
			case GLUT_LEFT_BUTTON:		// rotate scene around the origin according to mouse movement
				glLoadIdentity();

				myRotatef(-x_d*PI, myCamera.up[0], myCamera.up[1], myCamera.up[2]);
				myRotatef(y_d*PI, myCamera.right[0], myCamera.right[1], myCamera.right[2]);

				glMultMatrixf(myScene.rotateMat);
				glGetFloatv(GL_MODELVIEW_MATRIX, myScene.rotateMat);
				for (int i = 0, k = 0; i < fIndex && k < eIndex; i += faceElements[k] * 2, k++){
					if (k == objects[objInd].firstFaceElementIndex) {
						glLoadIdentity();
						myRotatef(-x_d*PI, myCamera.up[0], myCamera.up[1], myCamera.up[2]);
						myRotatef(y_d*PI, myCamera.right[0], myCamera.right[1], myCamera.right[2]);

						glMultMatrixf(objects[objInd].SRM);
						glGetFloatv(GL_MODELVIEW_MATRIX, objects[objInd].SRM);
						objInd++;
					}
				}

				break;
			case GLUT_RIGHT_BUTTON:		// translate the origin toward camera.right vector (right, left) or toward camera.up vector (up, down) according to mouse movement
										// Using 75 instead of 5 for step size in units, because the steps were too small with 5
				glLoadIdentity();
				myTranslatef(x_d*(GLfloat)75 * myCamera.right[0], x_d*(GLfloat)75 * myCamera.right[1], x_d*(GLfloat)75 * myCamera.right[2]);
				myTranslatef(-y_d*(GLfloat)75 * myCamera.up[0], -y_d*(GLfloat)75 * myCamera.up[1], -y_d*(GLfloat)75 * myCamera.up[2]);
				glMultMatrixf(myScene.translateMat);
				glGetFloatv(GL_MODELVIEW_MATRIX, myScene.translateMat);
				for (int i = 0, k = 0; i < fIndex && k < eIndex; i += faceElements[k] * 2, k++) {
					if (k == objects[objInd].firstFaceElementIndex) {
						glLoadIdentity();
						myTranslatef(x_d*(GLfloat)75 * myCamera.right[0], x_d*(GLfloat)75 * myCamera.right[1], x_d*(GLfloat)75 * myCamera.right[2]);
						myTranslatef(-y_d*(GLfloat)75 * myCamera.up[0], -y_d*(GLfloat)75 * myCamera.up[1], -y_d*(GLfloat)75 * myCamera.up[2]);
						glMultMatrixf(objects[objInd].STM);
						glGetFloatv(GL_MODELVIEW_MATRIX, objects[objInd].STM);
						objInd++;
					}
				}

				myScene.origin[0] += x_d*(GLfloat)75;
				myScene.origin[1] -= y_d*(GLfloat)75;
				break;
			case GLUT_MIDDLE_BUTTON:	// translate the origin toward camera.toward vector 
				glLoadIdentity();
				myTranslatef(y_d*(GLfloat)75 * myCamera.towards[0], y_d*(GLfloat)75 * myCamera.towards[1], y_d*(GLfloat)75 * myCamera.towards[2]);
				glMultMatrixf(myScene.translateMat);
				glGetFloatv(GL_MODELVIEW_MATRIX, myScene.translateMat);	
				for (int i = 0, k = 0; i < fIndex && k < eIndex; i += faceElements[k] * 2, k++) {
					if (k == objects[objInd].firstFaceElementIndex) {
						glLoadIdentity();
						myTranslatef(y_d*(GLfloat)75 * myCamera.towards[0], y_d*(GLfloat)75 * myCamera.towards[1], y_d*(GLfloat)75 * myCamera.towards[2]);
						glMultMatrixf(objects[objInd].STM);
						glGetFloatv(GL_MODELVIEW_MATRIX, objects[objInd].STM);
						objInd++;
					}
				}
				myScene.origin[2] -= y_d*(GLfloat)75;	// Determining how many units to zoom in or out on the scene in direction 
				break;
			}
			//sceneModeActions(button);
		}

		if (cameraMode) {  // Camera mode
			switch (button) {
			case GLUT_LEFT_BUTTON:		// rotate camera around its up or right axis (rotation when camera is located at (0,0,0))
										//...according to mouse movement and the window size
				glLoadIdentity();
				myRotatef(y_d*PI, myCamera.right[0], myCamera.right[1], myCamera.right[2]);
				myRotatef(x_d*PI, myCamera.up[0], myCamera.up[1], myCamera.up[2]);
				glMultMatrixf(myCamera.rotateMat);
				glGetFloatv(GL_MODELVIEW_MATRIX, myCamera.rotateMat);
				for (int i = 0, k = 0; i < fIndex && k < eIndex; i += faceElements[k] * 2, k++) {
					if (k == objects[objInd].firstFaceElementIndex) {
						glLoadIdentity();
						myRotatef(y_d*PI, myCamera.right[0], myCamera.right[1], myCamera.right[2]);
						myRotatef(x_d*PI, myCamera.up[0], myCamera.up[1], myCamera.up[2]);
						glMultMatrixf(objects[objInd].CRM);
						glGetFloatv(GL_MODELVIEW_MATRIX, objects[objInd].CRM);
						objInd++;
					}
				}

				break;
			case GLUT_RIGHT_BUTTON:
				glLoadIdentity();
				myTranslatef(-x_d*(GLfloat)75, -y_d*(GLfloat)75, 0);
				glMultMatrixf(myCamera.rotateMat);
				glGetFloatv(GL_MODELVIEW_MATRIX, myCamera.rotateMat);
				for (int i = 0, k = 0; i < fIndex && k < eIndex; i += faceElements[k] * 2, k++) {
					if (k == objects[objInd].firstFaceElementIndex) {
						glLoadIdentity();
						myTranslatef(-x_d*(GLfloat)75, -y_d*(GLfloat)75, 0);
						glMultMatrixf(objects[objInd].CRM);
						glGetFloatv(GL_MODELVIEW_MATRIX, objects[objInd].CRM);
						objInd++;
					}
				}

				myCamera.origin[0] += x_d*(GLfloat)75;
				myCamera.origin[1] -= y_d*(GLfloat)75;
				break;
			case GLUT_MIDDLE_BUTTON: // zoom in when the mouse moving up and zoom out when the mouse moving down
				glLoadIdentity();
				myTranslatef(0, 0, -y_d*(GLfloat)75);
				glMultMatrixf(myCamera.rotateMat);
				glGetFloatv(GL_MODELVIEW_MATRIX, myCamera.rotateMat);
				for (int i = 0, k = 0; i < fIndex && k < eIndex; i += faceElements[k] * 2, k++) {
					if (k == objects[objInd].firstFaceElementIndex) {
						glLoadIdentity();
						myTranslatef(0, 0, -y_d*(GLfloat)75);
						glMultMatrixf(objects[objInd].CRM);
						glGetFloatv(GL_MODELVIEW_MATRIX, objects[objInd].CRM);
						objInd++;
					}
				}

				myCamera.origin[2] += y_d*(GLfloat)75;
				break;
			}
			//cameraModeActions(button);
		}

	}
	else {		// On mouse click, button down
		mouseStartX = x;
		mouseStartY = y;

		if (button == GLUT_LEFT_BUTTON && objectMode) {	// COLOR PICKING
			unsigned char redCode = 5;
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			drawAllPolygons(true);		// stands for drawing on the back buffer 

			glReadPixels(x, viewport[3] - y, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &redCode);
			printf("The red code is %d\n", redCode);

			selectedObjectIndex = redCode - 1;		// Saving index for later on to draw the red sphere around this Object's COM
		}
	}
	pressed = !pressed;
}

void keyboard(unsigned char key, int x, int y) {
	if (pressed)	// if mouse is pressed ignore keyboard action, don't try to switch modes while playing with mouse functions
		return;
	if (key == 'c' || key == 'C') { //camera mode
		cameraMode = true;
		objectMode = false;
		sceneMode = false;
		selectedObjectIndex = -1;
		setLightIntensity(sceneMode);
		printf("\n\nCamera mode\n");
		printf("\tLeft Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: rotate camera by camera.RIGHT Vector\n");
		printf("\t\tUp/Down movement: rotate camera by camera.UP Vector\n");
		printf("\tRight Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: moving camera by camera.RIGHT Vector \n");
		printf("\t\tUp/Down movement: moving camera by camera.UP Vector\n");
		printf("\tMiddle Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: No action\n");
		printf("\t\tUp/Down movement: moving camera by camera.TOWARDS Vector\n");
	}

	if (key == 's' || key == 'S') { //scene mode
		cameraMode = false;
		objectMode = false;
		sceneMode = true;
		selectedObjectIndex = -1;
		setLightIntensity(sceneMode);

		printf("\n\nScene mode\n");
		printf("\tLeft Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: rotate scene by camera.RIGHT Vector\n");
		printf("\t\tUp/Down movement: rotate scene by camera.UP Vector\n");
		printf("\tRight Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: moving scene by camera.RIGHT Vector \n");
		printf("\t\tUp/Down movement: moving scene by camera.UP Vector\n");
		printf("\tMiddle Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: No action\n");
		printf("\t\tUp/Down movement: moving scene by camera.TOWARDS Vector\n");
	}


	if (key == 'o' || key == 'O') { //object mode
		cameraMode = false;
		objectMode = true;
		sceneMode = false;

		// Minor modes flags initialization
		objectModeCameraFrame = true;	// default
		objectModeSceneFrame = false;
		objectModeObjectFrame = false;

		//	setLightIntensity(sceneMode);
		printf("\n\nObject mode:::> Camera-frame minor mode 1 ACTIVE\n");
		printf("\tLeft Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: rotate camera by camera.RIGHT Vector\n");
		printf("\t\tUp/Down movement: rotate camera by camera.UP Vector\n");
		printf("\tRight Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: moving camera by camera.RIGHT Vector \n");
		printf("\t\tUp/Down movement: moving camera by camera.UP Vector\n");
		printf("\tMiddle Mouse Button:\n  ");
		printf("\t\tLeft/Right movement: No action\n");
		printf("\t\tUp/Down movement: moving camera by camera.TOWARDS Vector\n");
	}

	if (objectMode && key == '1') { //object mode - Camera-frame minor mode 1 
		printf("\nObject mode:::> Camera-frame minor mode 1 ACTIVE\n");
		objectModeCameraFrame = true;
		objectModeSceneFrame = false;
		objectModeObjectFrame = false;
	}
	if (objectMode && key == '2') { //object mode - Scene-frame minor mode 2 
		printf("\nObject mode:::> Scene-frame minor mode 2 ACTIVE\n");
		objectModeCameraFrame = false;
		objectModeSceneFrame = true;
		objectModeObjectFrame = false;
	}
	if (objectMode && key == '3') { //object mode - Object-frame minor mode 3
		printf("\nObject mode:::> Object-frame minor mode 3 ACTIVE\n");
		objectModeCameraFrame = false;
		objectModeSceneFrame = false;
		objectModeObjectFrame = true;
	}


}

void calcCenterOfMass(int objectIndex) {
	GLfloat sum[3];
	bool verticesVisited[maxSize];
	int numberOfVertices = 0;
	int k = 0;

	sum[0] = (GLfloat)0.0;
	sum[1] = (GLfloat)0.0;
	sum[2] = (GLfloat)0.0;

	for (int j = 0; j < maxSize; j++) {
		verticesVisited[j] = false;
	}

	for (GLuint i = objects[objectIndex].firstFacesIndex; i < objects[objectIndex].lastFacesIndex; i += 2) {
		if (verticesVisited[faces[i]] == false) {	// if this vertex is visited for the first time
			numberOfVertices++;
			verticesVisited[faces[i]] = true;
			sum[0] += vertices[(faces[i] - 1) * 3];
			sum[1] += vertices[(faces[i] - 1) * 3 + 1];
			sum[2] += vertices[(faces[i] - 1) * 3 + 2];
		}
	}

	//printf("Object %d has %d vertices\n\n", objectIndex, numberOfVertices);
	objects[objectIndex].COM[0] = (GLfloat)sum[0] / (GLfloat)numberOfVertices;
	objects[objectIndex].COM[1] = (GLfloat)sum[1] / (GLfloat)numberOfVertices;
	objects[objectIndex].COM[2] = (GLfloat)sum[2] / (GLfloat)numberOfVertices;
	
}

void setLightIntensity(bool sceneMode) {
	if (sceneMode) {	// Enhanced light intensity (by 15%)
		GLfloat light_ambient[] = { 0.575f, 0.575f, 0.575f, 1.0f };
		GLfloat light_diffuse[] = { 0.0f, 0.575f, 0.575f, 1.0f };
		GLfloat light_specular[] = { 0.0f, 0.0f, 0.575f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	}
	else {		// Normal light intensity
		GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
		GLfloat light_diffuse[] = { 0.0, 0.5, 0.5, 1.0 };
		GLfloat light_specular[] = { 0.0, 0.0, 0.5, 1.0 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	}
}

void setMaterial(int objIndex, bool drawBack) {
	if (drawBack) {
		glColor4f((1.0f + (GLfloat)objIndex) / 255.0f, 0.0f, 0.0f, 1.0f);
	
	}
	else {
		mat_Ka[0] = objects[objIndex].Ka[0];
		mat_Ka[1] = objects[objIndex].Ka[1];
		mat_Ka[2] = objects[objIndex].Ka[2];

		mat_Kd[0] = objects[objIndex].Kd[0];
		mat_Kd[1] = objects[objIndex].Kd[1];
		mat_Kd[2] = objects[objIndex].Kd[2];

		mat_Ks[0] = objects[objIndex].Ks[0];
		mat_Ks[1] = objects[objIndex].Ks[1];
		mat_Ks[2] = objects[objIndex].Ks[2];
		//	low_sh[0] = { 5.0 };
	}

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_Ks);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, low_sh);
}

void objectReader()
{
	char c;
	f = fopen("scene.obj", "r");
	if (f == NULL) {	// File open failed
		cout << "File open failed!" << endl;
		return;
	}
	int b = 0;
	vIndex = 0;
	vnIndex = 0;
	fIndex = 0;
	eIndex = 0;
	oIndex = -1;
	int line = 0;
	bool countingFaces = true;

	for (int i = 0; i < maxSize; i++) {
		objectNameToIndex[i] = -1;
	}

	c = fgetc(f);
	while (c != EOF && vIndex < maxSize * 3 && vnIndex < maxSize * 2 * 3 && fIndex < maxSize * 4 * 3)
	{
		line++;
		switch (c)
		{
		case 'v':
			if (countingFaces) {	// Means we have finished reading the former object's faces, initializing next objet.
				if (oIndex >= 0) {
					calcCenterOfMass(oIndex);

				}
				oIndex++;
				countingFaces = false;
				objects[oIndex].name = oIndex + 1;  //default naming
				objectNameToIndex[objects[oIndex].name] = oIndex;
				objects[oIndex].firstFaceElementIndex = eIndex;
				objects[oIndex].firstFacesIndex = fIndex;

			}

			c = fgetc(f);
			if (c == 'n')
			{
				fscanf(f, " %f %f %f\n", &normals[vnIndex], &normals[vnIndex + 1], &normals[vnIndex + 2]);
				vnIndex += 3;
			}
			else
			{
				if (c == 32) {
					fscanf(f, "%f %f %f\n", &vertices[vIndex], &vertices[vIndex + 1], &vertices[vIndex + 2]);
					vIndex += 3;
				}
			}
			break;
		case 'f':
			if (!countingFaces) {	// Means we start reading a new object's faces.
				countingFaces = true;
			}
			c = fgetc(f);
			b = 2;
			while (c != '\n' && b > 0)
			{
				b = fscanf(f, "%d//%d", &faces[fIndex], &faces[fIndex + 1]);
				if (b == 2)
				{

					fIndex += 2;
					c = fgetc(f);
					faceElements[eIndex]++; //number of vertice\normal pairs in this face
				}
			}
			objects[oIndex].lastFacesIndex = fIndex - 1;			//Will finally save the last faces index of the current object to objects[oIndex].lastFacesIndex (from first vertex to last normal !)
			objects[oIndex].lastFaceElementIndex = eIndex;		//Will finally save the last face elemet index of the current object to objects[oIndex].lastFaceElementIndex
			eIndex++;
			break;
		default:
			while (c != '\n' && c != EOF)
				c = fgetc(f);
			break;
		}
		if (c != EOF)
			c = fgetc(f);
	}
	calcCenterOfMass(oIndex);		// for the last object 
	oIndex++;
	fclose(f);
}

void colorTableReader() {
	char c = 'a';	//default value != EOF
	object tmpObj;

	unsigned char imageName[128];


	f = fopen("colorTable.csv", "r");
	if (f == NULL) {	// File open failed
		cout << "File open failed!" << endl;
		return;
	}

	while (c != EOF) {
		fscanf(f, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f", &tmpObj.name, &tmpObj.Ka[0], &tmpObj.Ka[1], &tmpObj.Ka[2], &tmpObj.Kd[0], &tmpObj.Kd[1], &tmpObj.Kd[2], &tmpObj.Ks[0], &tmpObj.Ks[1], &tmpObj.Ks[2]);

		if (objectNameToIndex[tmpObj.name] != -1) {	// Object with name=tmpObj.name  exists
													//initializing selfMatrices for each Object
			GLfloat tmpMat[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
			glLoadIdentity();
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].CRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].CTM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].SRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].STM); 
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].OSRM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].OSTM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].ORM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].OTM);

			glLoadIdentity();
			myTranslatef(myScene.origin[0], myScene.origin[1], myScene.origin[2]);
			glMultMatrixf(objects[objectNameToIndex[tmpObj.name]].STM);
			glGetFloatv(GL_MODELVIEW_MATRIX, objects[objectNameToIndex[tmpObj.name]].STM);




			glLoadIdentity();

			objects[objectNameToIndex[tmpObj.name]].Ka[0] = tmpObj.Ka[0];
			objects[objectNameToIndex[tmpObj.name]].Ka[1] = tmpObj.Ka[1];
			objects[objectNameToIndex[tmpObj.name]].Ka[2] = tmpObj.Ka[2];
			objects[objectNameToIndex[tmpObj.name]].Ka[3] = 1.0;

			objects[objectNameToIndex[tmpObj.name]].Kd[0] = tmpObj.Kd[0];
			objects[objectNameToIndex[tmpObj.name]].Kd[1] = tmpObj.Kd[1];
			objects[objectNameToIndex[tmpObj.name]].Kd[2] = tmpObj.Kd[2];
			objects[objectNameToIndex[tmpObj.name]].Kd[3] = 1.0;

			objects[objectNameToIndex[tmpObj.name]].Ks[0] = tmpObj.Ks[0];
			objects[objectNameToIndex[tmpObj.name]].Ks[1] = tmpObj.Ks[1];
			objects[objectNameToIndex[tmpObj.name]].Ks[2] = tmpObj.Ks[2];
			objects[objectNameToIndex[tmpObj.name]].Ks[3] = 1.0;

			c = fgetc(f);
			if (c == ',') {
				fscanf(f, "%s", imageName);
				fseek(f, -1, SEEK_CUR);
				printf("this object has texture - %s \n", imageName);
				if (loadTexture(imageName, numberOfTextures) == -1)
				{
					objects[objectNameToIndex[tmpObj.name]].textureIndex = -1;
				}
				else {
					objects[objectNameToIndex[tmpObj.name]].textureIndex = numberOfTextures;
					numberOfTextures++;
				}

			}
			else
			{
				objects[objectNameToIndex[tmpObj.name]].textureIndex = -1;
				printf("no texture for this object \n");
				fseek(f, -1, SEEK_CUR);
			}
		}

		c = fgetc(f);
		while (c != '\n' && c != EOF) {	// reading chares till end of line
			c = fgetc(f);
		}
		if (c == '\n') {
			c = fgetc(f);
			if (c != EOF) {
				fseek(f, -1, SEEK_CUR);
			}
		}
	}
	fclose(f);
}

int loadTexture(unsigned char * imageName, int textureIndex) {
	int width, height;
	unsigned char* image = SOIL_load_image((const char*)imageName, &width, &height, 0, SOIL_LOAD_RGB);
	if (image == 0) {
		printf("Error loading texture %s\n", imageName);
		return -1;
	}
	glBindTexture(GL_TEXTURE_2D, texName[textureIndex]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	SOIL_free_image_data(image);
	return 0;
}

void multMatVec(GLfloat * mat, GLfloat * vec, GLfloat * resVec){
	resVec[0] = mat[0] * vec[0] + mat[4] * vec[1] + mat[8] * vec[2] + mat[12];
	resVec[1] = mat[1] * vec[0] + mat[5] * vec[1] + mat[9] * vec[2] + mat[13];
	resVec[2] = mat[2] * vec[0] + mat[6] * vec[1] + mat[10] * vec[2] + mat[14];
	resVec[3] = mat[3] * vec[0] + mat[7] * vec[1] + mat[11] * vec[2] + mat[15];
	
}