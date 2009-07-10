// here be rushed code

#define _STDCALL_SUPPORTED
#define _M_IX86
// ^cross compile stuff^

#include <OpenGL/gl.h>
//#include <GLUT/glut.h>
#include <ctime>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <sys/time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace std;

#define PI 3.141

long int ticks = 0;
long int lasttick = 0; //for FPS
time_t starttime;

float seconds, minutes, hours;
float sechitx, sechittime;
float ratio = 1;

float scrLeft=-1;
float scrRight=1;
float scrTop=1;
float scrBottom=-1;

float colheight1, colheight2;

float fps = 500;

int mouseActivity = 0;

void renderScene();

void doPhysics()
{
	ticks++;
	
	renderScene();
	
	#if defined(_WIN32)
	Sleep(50*fps/1000.0);	// "i imported a whole winodws.h and all i got was this Sleep function"
	#else
	usleep(50*fps);
	#endif
	
	if(mouseActivity > 0 && ticks%10 == 0)
		mouseActivity--;
	
	if(ticks-lasttick>500)
	{
		cerr<<(float)(ticks-lasttick)/(clock()-starttime)*CLOCKS_PER_SEC<<endl;
		fps = (float)(ticks-lasttick)/(clock()-starttime)*CLOCKS_PER_SEC;
		starttime = clock();
		lasttick=ticks;
	}
}

void resizeWindow(int w, int h)
{
	glViewport(0,0,w,h);
	if(w<=0)w=1;
	if(h<=0)h=1;
	ratio = (float) w / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-ratio, ratio, -1, 1, -1, 1);
	
	scrLeft=-ratio;
	scrRight=ratio;
}
void mouseMove (int x, int y)
{
	mouseActivity++;
	if(mouseActivity > 10) //this is the bit, where i could quit (if you move the mouse)
		exit(0);
}

char numMasks[] = {119, 36, 93, 109, 46, 107, 123, 37, 127, 47};

void drawNum(int num, float offx, float offy, float size)	//draws a digit with points from (offx, offy) to (offx + size, offy + size*2)
								//bulge is about 0.3 * size on each side
{
	float x1,y1,x2,y2,x3,y3,d;
	for(int i=0;i<7;i++)
	{
		if(numMasks[num] & (1 << i))
		{
			x1 = ((i%3)==2)?size:0;
			x2 = ((i%3)==1)?0:size;
			y1 = -floor(i/3)*size + size;
			y2 = (i%3==0?0:-size) - floor(i/3)*size + size;
			
			x3=x1;
			y3=y1;
			glBegin(GL_POLYGON);
			for(float t=0;t<1;t+=0.1)
			{
				d=sinf(t*PI+0*ticks/2000.0)*3*size;
				x3 = (x1 + t * (x2 - x1) + (y1-y2) * d);
				y3 = (y1 + t * (y2 - y1) + (x2-x1) * d);
				glVertex3f(x3 + offx, y3 + offy,0);
			}
			for(float t=1;t>0;t-=0.1)
			{
				d=sinf(t*PI+0*ticks/2000.0)*3*size;
				x3 = (x1 + t * (x2 - x1) - (y1-y2) * d);
				y3 = (y1 + t * (y2 - y1) - (x2-x1) * d);
				glVertex3f(x3 + offx, y3 + offy,0);
			}
			glEnd();
		}
	}
}

void drawWave(float l, float r, float b, float t, float fluidHeight)
{
	float waveHeight = 0;
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0/255.0, 240/255.0, 0/255.0, 0.001);	//don't actually want to draw it, just use it as a mask
	
	glBegin(GL_QUAD_STRIP);
	glVertex3f(l,t,0);
	
		for(float x=l;x<=r;x+=(r-l)*0.01)
		{
			waveHeight=(sin(x*30 + cosf(ticks/1100.0)*2*PI +ticks/700.0 + fluidHeight) + cosf(ticks/1100.0)*0.3) * cos(ticks/500.0);
			waveHeight= waveHeight  * 0.05 + fluidHeight;
			
			if(waveHeight<0)waveHeight=0;
			if(waveHeight>1)waveHeight=1;
			glVertex3f(x,t,0);
			glVertex3f(x,waveHeight* (t-b) + b,0);
		}
		glVertex3f(r,waveHeight* (t-b) + b,0);
	glEnd();
}

void renderDraining(int num1, int num2, float fluidHeight, float offx, float offy)
{	
	glColor3f(212/255.0, 212/255.0, 212/255.0);
	drawNum(num1, offx, offy, 0.1);
	drawNum(num2, offx + 0.2, offy, 0.1);

	glEnable(GL_DEPTH_TEST);
	if(fluidHeight > 0)
	{
		drawWave(offx - 0.05, offx + 0.35, offy-0.15, offy+0.15, fluidHeight);
	
		glColor3f(169/255.0, 197/255.0, 212/255.0);
		drawNum(num1, offx, offy, 0.1);
		drawNum(num2, offx + 0.2, offy, 0.1);
	}
	glDisable(GL_DEPTH_TEST);
}

void drawColumn (int units, float fraction, float moveFraction, int max, float offx)
{
	float offset = 0;
	
	if(fraction < moveFraction)
			offset=1-(fraction/moveFraction*PI*2-sin(fraction/moveFraction*PI*2))/PI/2;

	renderDraining (floor(units/10), (int)units%10,1 - fraction, offx, -offset * (0.1 * 3));
	
	for(int i=units + 1;i<=units+floor((scrTop-scrBottom)/2/(0.1*3));i++)
	{
		int i2=(i+max)%max;
		renderDraining (floor(i2/10), (int)i2%10, 1, offx, (units - i - offset) * (0.1 * 3));
	}
	
	for(int i=units-ceil((scrTop-scrBottom)/2/(0.1*3));i<units;i++)
	{
		int i2=(i+max)%max;
		renderDraining (floor(i2/10), (int)i2%10, 0, offx, (units - i - offset) * (0.1 * 3));
	}
}

void drawCircle (float x, float y, float radius)
{
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(x,y,0);
		for(float th = 0; th<=2*PI+0.001;th+=PI/240)
		{
			glVertex3f(x + cos(th)*radius, y + sin(th)*radius, 0);
		}
	glEnd();
}

void renderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(0.5,1,0.0,1);
	
		
	timeval t;
	gettimeofday(&t, NULL);
	
	t.tv_sec += 9.5 * 60 * 60;
	
	seconds = t.tv_sec % 60 + t.tv_usec / 1000000.0;
	minutes = (t.tv_sec + t.tv_usec / 1000000.0)/60 - floor((t.tv_sec + t.tv_usec / 1000000.0)/60/60)*60;
	hours = (t.tv_sec - floor(t.tv_sec/60/60/12)*60*60*12)/60/60;

	glColor3f(240/255.0, 240/255.0, 240/255.0);
	glBegin(GL_QUADS);
	glVertex3f(scrLeft,0.15,0);
	glVertex3f(scrRight,0.15,0);
	glVertex3f(scrRight,-0.15,0);
	glVertex3f(scrLeft,-0.15,0);
	glEnd();
	
	drawColumn(floor(seconds), seconds - floor(seconds), 0.6, 60,  0.1 * 3.5);
	drawColumn(floor(minutes), minutes - floor(minutes), 0.1, 60, -0.1 * 1.5);
	drawColumn(floor(hours), hours - floor(hours), 0.05, 12, -0.1 - 0.1 * 5.5);
	
	drawCircle(-0.1-0.1*1.5,0.1*0.6,0.03);
	drawCircle(-0.1-0.1*1.5,-0.1*0.6,0.03);
		
	drawCircle(0.1+0.1*1.5,0.1*0.6,0.03);
	drawCircle(0.1+0.1*1.5,-0.1*0.6,0.03);
	
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	colheight1 += ((minutes - floor(minutes) + hours - floor(hours))/2 - colheight1)/100.0;
	colheight2 += ((minutes - floor(minutes) + seconds - floor(seconds))/2 - colheight2)/100.0;
	drawWave(0.1+0.1*1.5 - 0.05, 0.1+0.1*1.5 + 0.05, -0.15, 0.15, 1-colheight2);
	drawWave(-0.1-0.1*1.5 - 0.05, -0.1-0.1*1.5 + 0.05, -0.15, 0.15, 1-colheight1);
	
	glColor3f(169/255.0, 197/255.0, 212/255.0);
	drawCircle(-0.1-0.1*1.5,0.1*0.6,0.03);
	drawCircle(-0.1-0.1*1.5,-0.1*0.6,0.03);
		
	drawCircle(0.1+0.1*1.5,0.1*0.6,0.03);
	drawCircle(0.1+0.1*1.5,-0.1*0.6,0.03);
	glDisable(GL_DEPTH_TEST);
	
	glFlush();
	//glutSwapBuffers();
}

void initGL() {
	starttime=clock();
	glClearColor(247/255.0, 247/255.0, 247/255.0, 1);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_POLYGON_SMOOTH);	
}

/*
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	glutInitWindowPosition(100,100);
	glutInitWindowSize(320,320);
	glutCreateWindow("timedrain");
	glutDisplayFunc(renderScene);
	glutIdleFunc(doPhysics);
	glutReshapeFunc(resizeWindow);
	glutPassiveMotionFunc(mouseMove);
	
	glutSetCursor(GLUT_CURSOR_NONE);

	glutFullScreen();
	starttime=clock();
	
	glClearColor(247/255.0, 247/255.0, 247/255.0, 1);
	
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_POLYGON_SMOOTH);
	
	glutMainLoop();
}
*/