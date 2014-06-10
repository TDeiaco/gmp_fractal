/* CFractal:

This CFractal class will manage the window coordinates and 
compute a certain fractal and save in memory.

CFractal acts as a state machine. Therefore if a rendering call
is issued to the fractal calculator, it will render the fractal 
within the limits set previously.  This means if you need to update the
veiwport, do it before rendering.

Various zoom functions will be implemented as well.

When zooming occures, the ability to go back to the previous zoom 
will allow one to go back in history to the beginning of the set.

*/
#define _USE_MATH_DEFINES

#include <SDL/SDL.h>
//#include <SDL_image.h>
#include <vector>
#include <random>
#include <map>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <gmp.h>

using namespace std;

typedef mpf_t real; //GMP Multiple Precision Float Variable


//const real PI(4.0*std::atan2(1.0,1.0));


typedef SDL_Color color; //Just in can we use another color type

struct frameCoords
{
	real xMin, xMax, yMin, yMax;
};

class CFractal
{
	//Number of pixels wide and number of pixles high
	//This is usually the window width and window hiegth
	unsigned int m_numPixelsW, m_numPixelsH;

	//These define the limits or viewport of the real number gridspace that 
	//the mandelbrot fractal is located in.
	frameCoords m_curCoords;

	//These define the increment of real numbers defined by the the realspace 
	//dimensions divided by the pixel dimensions.  It will be called an 
	//increment, but it is also the size of one pixel according to the real space
	real m_xInc, m_yInc;

	//This is thre raw pixel data from the fractal calculations
	char *m_pixelData;

	//This stack will save zooms
	std::vector<frameCoords> m_frames;
	//Number of saved frames
	unsigned int m_numFrames;
	//Current frame number
	unsigned int m_curFrameNum;

	//Coloring system
	SDL_Surface *m_colorMap;

	//Random number generation values
	gmp_randstate_t randState;

	

public:
	//SDL_Surface for rendering
	SDL_Surface *m_surface;

public:

	CFractal(unsigned int numPixelsW, unsigned int numPixelsH, real xMax, real xMin, 
		     real yMax, real yMin);

	~CFractal();

	void CalcIncrement();

	void PrintValues(unsigned int prec, unsigned int x, unsigned int y);

	void UpdateCurrentCoords();

	//Rendering functions

	void Smooth(real radius, real nIterations);

	void RenderMandelbrot(real numIterations, bool smooth, bool optimize);

	void RenderMandelbrotArbPrec(unsigned int numIterations, bool optimize);

	void RenderMandelbrot3(unsigned int numIterations);

	void RenderJulia(unsigned int numIterations, unsigned int x, unsigned int y);

	void RenderBuddhabrot(unsigned int numIterations, unsigned int numSamples);

	//Coordinate system functions

	//	void Zoom(real factor, unsigned int pixelX, unsigned int pixelY);

	void real_pos_x(real result, unsigned int pixelX);

	void real_pos_y(real result, unsigned int pixelY);

	void setCurCoords(frameCoords frame);

	void saveCurCoords();

	void undoCurCoords();

	void getCurCoords(frameCoords returnCoords);

	void RandRWithinWindow();

	void RandIWithinWindow();

	//Colormap functions
	bool BuildColors(unsigned int numColors);

	bool BuildLargeColorTable();

	void RenderColorTable();

	bool LoadColorMap();


};
