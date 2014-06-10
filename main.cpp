#include "SDL/SDL.h"
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <vector>
#include "fractal.h"
#include "gmp.h"


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define FRAME_WIDTH 800
#define FRAME_HEIGHT 600

SDL_Surface* CreateEmptySurface(int w, int h, bool fill, SDL_Color color);
void SetPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void Render();

//SDL structures
SDL_Surface *g_screen;
SDL_Surface *mandelbrot;

//Fractal generator engine
CFractal *fractal;
frameCoords zoom;
real yDist;


//Backups, like an Undo
//std::vector<frameCoords> *m_backup;
//bool saveJourney;
unsigned int saveNumber;

//Holds the 
real iterationCount;

bool guideStart;
unsigned int guideFirstX;
unsigned int guideFirstY;
unsigned int guideCurX;
unsigned int guideCurY;

void RenderGuideSquare(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  Uint32 squareColor = SDL_MapRGB(g_screen->format, 255, 255, 255);

  for(unsigned int k=0; k<w; k++)
    {
      SetPixel(g_screen, x+k, y, squareColor);
      SetPixel(g_screen, x+k, y+h, squareColor);
    }
  for(unsigned int k=0; k<h; k++)
    {
      SetPixel(g_screen, x, y+k, squareColor);
      SetPixel(g_screen, x+w, y+k, squareColor);
    }

}
				
		


bool Input()
{

  unsigned char *keys = SDL_GetKeyState(0);

  int x = 0;
  int y = 0;
  SDL_GetMouseState(&x, &y);
  float x2 = x;
  float y2 = y;

  guideCurX = x;
  guideCurY = y;

  //printf("%i  ", x);
  //printf("%i  \n",y);  //Causes major slowdown

  /*coord mouse = coordMng->calcGridspaceCoord(x, y);
    mouseLocation = mouse;
    printf("%f  %f\n", mouse.x, mouse.y);*/

  SDL_Event event;
  while ( SDL_PollEvent(&event) ) 
    {
      if ( event.type == SDL_QUIT ) 
	{
	  return false;
	}
      if ( event.type == SDL_KEYDOWN ) 
	{
	  switch (event.key.keysym.sym)
	    {
	    case SDLK_ESCAPE:
	      {
		return false;
	      }
	    case SDLK_1:
	      {
		mpf_set_ui(iterationCount, 100);
		fractal->RenderMandelbrot(iterationCount, false,  false);
		break;
	      }
	    case SDLK_2:
	      {
		mpf_set_ui(iterationCount, 250);
		fractal->RenderMandelbrot(iterationCount,false, false);
		break;
	      }
	    case SDLK_3:
	      {
		mpf_set_ui(iterationCount , 500);
		fractal->RenderMandelbrot(iterationCount, false,  false);
		break;
	      }
	    case SDLK_4:
	      {
		mpf_set_ui(iterationCount , 1000);
		fractal->RenderMandelbrot(iterationCount,false,  false);
		break;
	      }
	    case SDLK_5:
	      {
		mpf_set_ui(iterationCount , 2500);
		fractal->RenderMandelbrot(iterationCount, false, false);
		break;
	      }
	    case SDLK_6:
	      {
		mpf_set_ui(iterationCount , 5000);
		fractal->RenderMandelbrot(iterationCount, false,  false);
		break;
	      }
	    case SDLK_7:
	      {
		mpf_set_ui(iterationCount , 10000);
		fractal->RenderMandelbrot(iterationCount, false,  false);
		break;
	      }
	    case SDLK_m:
	      {
		fractal->RenderMandelbrot(iterationCount, true, false);
		break;
	      }
	    case SDLK_t:
	      {
		//fractal->RenderMandelbrot3(iterationCount);
		break;
	      }
	    case SDLK_c:
	      {
		//fractal->BuildLargeColorTable();
		//fractal->RenderColorTable();
		break;
	      }
	    case SDLK_o:
	      {
		//fractal->RenderMandelbrotArbPrec(1000, false);
		break;
	      }
	    case SDLK_b:
	      {
		//fractal->undoCurCoords();
		//fractal->RenderMandelbrot(1000, true, false);
		break;
	      }
	    case SDLK_z:
	      {
		//fractal->Zoom(0.8, x, y);
		//fractal->RenderMandelbrot(iterationCount,true, false);
		//SDL_SetMouseState(x, y);
		break;
	      }
	    case SDLK_p:
	      {
		//fractal->RenderMandelbrot(iterationCount, false, false);
		break;
	      }
	    case SDLK_l:
	      {
		//fractal->Zoom(0.5, x, y);
		//fractal->RenderMandelbrot(iterationCount, false, false);
		//Render();

	      }
	    case SDLK_g:
	      {
		//fractal->RenderBuddhabrot(100, 1000000);
		break;
	      }
			
	    default:
	      {
		return true;
	      }
	    }

	}

      if(event.type == SDL_MOUSEMOTION){
	//fractal->RenderJulia(200, x, y);
			
	return true;
      }
			
      if(event.type == SDL_MOUSEBUTTONDOWN){
	fractal->real_pos_x(zoom.xMin, x);
	fractal->real_pos_y(zoom.yMax, y);
	//zoom.xMin = fractal->returnX(x);
	//zoom.yMax = fractal->returnY(y);

	if(guideStart == true)
	  {
	    guideFirstX = x;
	    guideFirstY = y;
	    guideStart = false;
	  }
		
			
	return true;
      }
      if(event.type == SDL_MOUSEBUTTONUP){
	guideStart = true;

	//zoom.xMax = fractal->returnX(x);
	//zoom.yMin = fractal->returnY(y);
	fractal->real_pos_x(zoom.xMax, x);
	fractal->real_pos_y(zoom.yMin, y);

	//real yDist = ((zoom.xMax - zoom.xMin) * WINDOW_HEIGHT) / WINDOW_WIDTH;

	mpf_set_d(yDist, 0.0);
	mpf_sub(yDist, zoom.xMax, zoom.xMin);
	mpf_mul_ui(yDist, yDist, WINDOW_HEIGHT);
	mpf_div_ui(yDist, yDist, WINDOW_WIDTH);

	//zoom.yMin = zoom.yMax - yDist;
	mpf_sub(zoom.yMin, zoom.yMax , yDist);

	fractal->setCurCoords(zoom);
	fractal->PrintValues(20, x, y);
	fractal->RenderMandelbrot(iterationCount, false, false);
	//fractal->saveCurCoords();
			
	char fileName[256];
	for(int i = 0; i < 256; i++)
	  fileName[i] = 0;

	sprintf(fileName, "Fractal number %d.bmp", saveNumber++);


	SDL_SaveBMP(fractal->m_surface, fileName);

	//m_backup->push_back(fractal->getCurCoords());
			
	return true;
      }
    }
	return true;
}


void Render()
{

  SDL_FillRect(g_screen, 0, SDL_MapRGB(fractal->m_surface->format, 200,200,0));
  SDL_Rect dest = {0,0,0,0};
  SDL_BlitSurface(fractal->m_surface, 0, g_screen, &dest);
  if(guideStart == false){
    if((guideCurX > guideFirstX) && (guideCurY > guideFirstY)){
      RenderGuideSquare(guideFirstX, guideFirstY, 
			guideCurX - guideFirstX,(( guideCurX - guideFirstX)*WINDOW_HEIGHT)/WINDOW_WIDTH);
    }
  }
  /*
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();*/

  SDL_Flip(g_screen);
}


bool InitSDL()
{
  if(SDL_Init(SDL_INIT_VIDEO))
    {
      printf("Couldn't initalize SDL!");
      return 0;
    }

  if((g_screen = SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,24, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL)
    {
      printf("Couldn't set video mode! Quitting...");
      SDL_Quit();	
      return 0;
    }
  //glClearColor(0.0, 0.0, 0.0, 0.0);
  return 1;
}

bool MainLoop()
{
  for(unsigned int i = 0; i < 25; i++){
    //printf("%f, \n", fractal->RandRWithinWindow());
  }
  Render();
  return Input();
}

int main(int argc, char** argv)
{
  InitSDL();

  //Init GMP variables
  mpf_set_default_prec(256);

  mpf_init(iterationCount);
  mpf_init(zoom.xMax);
  mpf_init(zoom.xMin);
  mpf_init(zoom.yMax);
  mpf_init(zoom.yMin);
  mpf_init(yDist);

  //saveJourney = true;
  saveNumber = 0;
  mpf_set_ui(iterationCount, 500);

  guideStart = true;
  guideFirstX = guideFirstY = 0;
  guideCurX = guideCurY = 0;

  real x1, x2, y1, y2;
  mpf_init(x1); mpf_set_d(x1, 1.0);
  mpf_init(x2); mpf_set_d(x2, -2.0);
  mpf_init(y1); mpf_set_d(y1, 1.0);
  mpf_init(y2); mpf_set_d(y2, -1.0);

  fractal = new CFractal(FRAME_WIDTH, FRAME_HEIGHT, x1, x2, y1, y2);
  fractal->CalcIncrement();
  fractal->RenderMandelbrot(iterationCount, false, false);
  SDL_SaveBMP(fractal->m_surface, "Save.bmp");

  //fractal->PrintValues(20);

  //m_backup = new std::vector<frameCoords>;
  //m_backup->push_back(fractal->getCurCoords());

  SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0",0);

   while(MainLoop());

  //Clear and free GMP variables
  mpf_clear(iterationCount);
  mpf_clear(yDist);

  mpf_clear(zoom.xMax);
  mpf_clear(zoom.xMin);
  mpf_clear(zoom.yMax);
  mpf_clear(zoom.yMin);

  mpf_clear(x1);
  mpf_clear(x2);
  mpf_clear(y1);
  mpf_clear(y2);

  //delete m_backup;
  delete fractal;
	
  SDL_FreeSurface(g_screen);
  SDL_Quit();
  return true;

}
