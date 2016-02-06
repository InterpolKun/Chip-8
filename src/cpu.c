#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <GL/glut.h>
#include "chip8.h"

/* Some workaround for delay in *NIX and Win systems. Tested on Linux, works well */

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


int display_width = 64 * 10;
int display_height = 32 * 10;

void display();
void reshape_window(GLsizei w, GLsizei h);
void keyboardUp(unsigned char ikey, int x, int y);
void keyboardDown(unsigned char ikey, int x, int y);

#define DRAWWITHTEXTURE

uint8_t screenData[32][64][3];
void setupTexture();

int main(int argc, char ** argv){

    if(argc != 2){
        printf("There is no ROM. Correct usage: \"chip8 rom\"\n");
        return 1;
    }

    loadRom(argv[1]);

    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(320, 320);
	glutCreateWindow("myChip8 (almost done)");

	glutDisplayFunc(display);
	glutIdleFunc(display);
    glutReshapeFunc(reshape_window);
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);

#ifdef DRAWWITHTEXTURE
	setupTexture();
#endif

	glutMainLoop();
	return 0;
}

void setupTexture()
{
	// Clear screen
	for(int y = 0; y < 32; ++y)
		for(int x = 0; x < 64; ++x)
			screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

	// Create a texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void updateTexture()
{
	// Update pixels
	for(int y = 0; y < 32; ++y)
		for(int x = 0; x < 64; ++x)
			if(gfx[(y * 64) + x] == 0)
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
			else
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled

	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, 64, 32, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			  0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			  display_height);
	glEnd();
}

// Old gfx code
void drawPixel(int x, int y)
{
	glBegin(GL_QUADS);
		glVertex3f((x * 10) + 0.0f,     (y * 10) + 0.0f,	 0.0f);
		glVertex3f((x * 10) + 0.0f,     (y * 10) + 10, 0.0f);
		glVertex3f((x * 10) + 10, (y * 10) + 10, 0.0f);
		glVertex3f((x * 10) + 10, (y * 10) + 0.0f,	 0.0f);
	glEnd();
}

void updateQuads()
{
	// Draw
	for(int y = 0; y < 32; ++y)
		for(int x = 0; x < 64; ++x)
		{
			if(gfx[(y*64) + x] == 0){
				glColor3f(0.0f,0.0f,0.0f);
			} else {
				glColor3f(1.0f,1.0f,1.0f);
            }
			drawPixel(x, y);
		}
}

void display()
{
    #ifdef _WIN32
    Sleep(30);
    #else
    usleep(4000);  /* sleep for 100 milliSeconds */
    #endif

	emulate();

	if(drawFlag == 1)
	{
		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT);

#ifdef DRAWWITHTEXTURE
		updateTexture();
#else
		updateQuads());
#endif

		// Swap buffers!
		glutSwapBuffers();

		// Processed frame
		drawFlag = 0;
	}
}

void reshape_window(GLsizei w, GLsizei h)
{
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

	// Resize quad
	display_width = w;
	display_height = h;
}

void keyboardDown(unsigned char ikey, int x, int y)
{
	if(ikey == 27)    // esc
		exit(0);

	if(ikey == '1')         key[0x1] = 1;
    else if(ikey == '2')    key[0x2] = 1;
	else if(ikey == '3')	key[0x3] = 1;
	else if(ikey == '4')	key[0xC] = 1;

	else if(ikey == 'q')	key[0x4] = 1;
	else if(ikey == 'w')	key[0x5] = 1;
	else if(ikey == 'e')	key[0x6] = 1;
	else if(ikey == 'r')	key[0xD] = 1;

	else if(ikey == 'a')	key[0x7] = 1;
	else if(ikey == 's')	key[0x8] = 1;
	else if(ikey == 'd')	key[0x9] = 1;
	else if(ikey == 'f')	key[0xE] = 1;

	else if(ikey == 'z')	key[0xA] = 1;
	else if(ikey == 'x')	key[0x0] = 1;
	else if(ikey == 'c')	key[0xB] = 1;
	else if(ikey == 'v')	key[0xF] = 1;

}

void keyboardUp(unsigned char ikey, int x, int y)
{
	if(ikey == '1')		key[0x1] = 0;
	else if(ikey == '2')	key[0x2] = 0;
	else if(ikey == '3')	key[0x3] = 0;
	else if(ikey == '4')	key[0xC] = 0;

	else if(ikey == 'q')	key[0x4] = 0;
	else if(ikey == 'w')	key[0x5] = 0;
	else if(ikey == 'e')	key[0x6] = 0;
	else if(ikey == 'r')	key[0xD] = 0;

	else if(ikey == 'a')	key[0x7] = 0;
	else if(ikey == 's')	key[0x8] = 0;
	else if(ikey == 'd')	key[0x9] = 0;
	else if(ikey == 'f')	key[0xE] = 0;

	else if(ikey == 'z')	key[0xA] = 0;
	else if(ikey == 'x')	key[0x0] = 0;
	else if(ikey == 'c')	key[0xB] = 0;
	else if(ikey == 'v')	key[0xF] = 0;
}
