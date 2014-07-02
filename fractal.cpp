#ifndef __GMP_FRACTAL__
#define __GMP_FRACTAL__

#include "fractal.h"


SDL_Surface* CreateEmptySurface(int w, int h, bool fill, SDL_Color color);
Uint32 GetPixel(SDL_Surface *surface, int x, int y);
void SetPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

CFractal::CFractal(unsigned int numPixelsW, unsigned int numPixelsH, real xMax, real xMin, 
		   real yMax, real yMin) :
  m_numPixelsW(numPixelsW), m_numPixelsH(numPixelsH)
{
  //Initialize all multiple precision numbers first
  mpf_init(m_curCoords.xMax); mpf_set(m_curCoords.xMax, xMax);
  mpf_init(m_curCoords.xMin); mpf_set(m_curCoords.xMin, xMin);
  mpf_init(m_curCoords.yMax); mpf_set(m_curCoords.yMax, yMax);
  mpf_init(m_curCoords.yMin); mpf_set(m_curCoords.yMin, yMin);

  mpf_init(m_xInc);
  mpf_init(m_yInc);

  //Calculate the real number width of a single
  //pixel.
  CalcIncrement();

  //SDL surface to be rendered too.
  m_pixelData = 0;
  m_surface = 0;

  SDL_Color color = {10, 120, 200, 0};
  m_surface = CreateEmptySurface(m_numPixelsW, m_numPixelsH, 0, color);

  //Start first backup for the undo function
  // m_frames.push_back(m_curCoords);

  //Number of backups currently
  m_numFrames = 0;
  m_curFrameNum = 0;

  //IMG_Init(IMG_INIT_PNG);
  m_colorMap = 0;

  //Seed random number genrator
  //Charnged to a GMP generator
  //std::srand(std::time(0));
  gmp_randinit_mt( randState );

  //
  LoadColorMap();

	
}

CFractal::~CFractal()
{
  if(m_pixelData)
    delete [] m_pixelData;
  if(m_surface)
    {
      SDL_FreeSurface(m_surface);
      m_surface = 0;
    }
  if(m_colorMap)
    {
      SDL_FreeSurface(m_colorMap);
      m_colorMap = 0;
    }

  mpf_clear(m_curCoords.xMax);
  mpf_clear(m_curCoords.xMin);
  mpf_clear(m_curCoords.xMax);
  mpf_clear(m_curCoords.yMin);

  mpf_clear(m_xInc);
  mpf_clear(m_yInc);

  gmp_randclear(randState);

  //IMG_Quit();
	
}

void CFractal::CalcIncrement()
{
  //Updates the increment 
  //Updated to GMP Multiple Precision

  //  m_xInc = (m_curCoords.xMax - m_curCoords.xMin) / ((real)m_numPixelsW - 1 );
  //  m_yInc = (m_curCoords.yMax - m_curCoords.yMin) / ((real)m_numPixelsH - 1 );

  mpf_sub(m_xInc,m_curCoords.xMax,  m_curCoords.xMin);
  mpf_sub(m_yInc,m_curCoords.yMax,  m_curCoords.yMin);


  mpf_div_ui(m_xInc, m_xInc, (unsigned int)(m_numPixelsW - 1));
  mpf_div_ui(m_yInc, m_yInc, (unsigned int)(m_numPixelsH - 1));
  

}

void CFractal::PrintValues(unsigned int prec, unsigned int x, unsigned int y)
{

  CalcIncrement();

  printf("Window width: %i \n", m_numPixelsW);
  printf("Window height: %i \n", m_numPixelsH);

  gmp_printf("X Axis Increment: %.*Ff \n", prec,m_xInc);
  gmp_printf("Y Axis Increment: %.*Ff \n", prec,m_yInc);

  gmp_printf("Current Coordinate X Max: %.*Ff \n", prec,m_curCoords.xMax);
  gmp_printf("Current Coordinate X Min: %.*Ff \n", prec,m_curCoords.xMin);
  gmp_printf("Current Coordinate y Max: %.*Ff \n", prec,m_curCoords.yMax);
  gmp_printf("Current Coordinate y Min: %.*Ff \n", prec,m_curCoords.yMin);

  real x_real, y_img;
  mpf_init(x_real);
  mpf_init(y_img);

  real_pos_x(x_real,x);
  real_pos_y(y_img, y);

  gmp_printf("Complex Plain X Mouse Coordinates: %.*Ff \n", prec,x_real);
  gmp_printf("Complex Plain Y Mouse Coordinates: %.*Ff \n", prec, y_img);

  mpf_clear(x_real);
  mpf_clear(y_img);


 
}

void CFractal::UpdateCurrentCoords()
{
}
/*
void CFractal::Zoom(real factor, unsigned int pixelX, unsigned int pixelY)
{
  CalcIncrement();
  real xPos = this->returnX(pixelX);
  real yPos = this->returnY(pixelY);

  real curWidth = m_curCoords.xMax - m_curCoords.xMin;
  real curHeight = m_curCoords.yMax - m_curCoords.yMin;

  curWidth = curWidth * factor;
  curHeight = curWidth * factor;

  m_curCoords.xMax = xPos + (curWidth / 2);
  m_curCoords.xMin = xPos - (curWidth / 2);
  m_curCoords.yMax = yPos + (curHeight / 2);
  m_curCoords.yMin = yPos - (curHeight / 2);
  }*/

 void CFractal::real_pos_x(real result, unsigned int pixelX)
{
  CalcIncrement();

  //  return (m_curCoords.xMin + (m_xInc / 2.0))+ ((real)pixelX * m_xInc);

  //Multiplying  ((real)pixelX * m_xInc) and storing in m_result
  mpf_mul_ui(result, m_xInc, pixelX);

  //Divide (m_xInc / 2.0)) and store in m_xInc
  mpf_div_ui(m_xInc, m_xInc, 2);

  //Add current coords to (m_xInc / 2.0) which is m_xInc
  //(m_curCoords.xMin + (m_xInc / 2.0))
  mpf_add(m_xInc, m_curCoords.xMin, m_xInc);

  //Do final sum 
  mpf_add(result, result, m_xInc);
}

void CFractal::real_pos_y(real result, unsigned int pixelY)
{
  CalcIncrement();
  //  (m_curCoords.yMax - (m_yInc / 2.0))- ((real)pixelY * m_yInc);

  //Multiplying  ((real)pixelX * m_xInc) and storing in m_result
  mpf_mul_ui(result, m_yInc, pixelY);

  //Divide (m_xInc / 2.0)) and store in m_yInc
  mpf_div_ui(m_yInc, m_yInc, 2);

  //Subtract (m_yInc / 2.0) from current coords
  //(m_curCoords.yMax - (m_yInc / 2.0))
  mpf_sub(m_yInc, m_curCoords.yMax, m_yInc);

  //Do final difference
  mpf_sub(result,  m_yInc, result);
}

void CFractal::setCurCoords(frameCoords frame)
{
  /*m_curCoords.xMax = frame.xMax;
  m_curCoords.xMin = frame.xMin;
  m_curCoords.yMax = frame.yMax;
  m_curCoords.yMin = frame.yMin;*/

  mpf_set(m_curCoords.xMax , frame.xMax);
  mpf_set(m_curCoords.xMin , frame.xMin);
  mpf_set(m_curCoords.yMax , frame.yMax);
  mpf_set(m_curCoords.yMin , frame.yMin);
}

void CFractal::saveCurCoords()
{

  // m_frames.push_back(m_curCoords);
}

void CFractal::undoCurCoords()
{
  /*  if(m_frames.size()){
    m_curCoords = m_frames[m_frames.size() - 1];
    m_frames.pop_back();
    }*/
}

void CFractal::getCurCoords(frameCoords returnCoords)
{
  mpf_set(returnCoords.xMax, m_curCoords.xMax);
  mpf_set(returnCoords.xMin, m_curCoords.xMin);
  mpf_set(returnCoords.yMax, m_curCoords.yMax);
  mpf_set(returnCoords.yMin, m_curCoords.yMin);
}

void CFractal::Smooth(real radius, real nIterations)
{
  //
  //return nIterations  -  (log(log(radius)) / log(2.0));
}

void CFractal::RenderMandelbrot(real numIterations, bool smooth, bool optimize)
{
  SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v3.0 w/GMP - Render In Progress...",0);

  CalcIncrement();

  //BuildColors(numIterations);

  //This will set firstX and firstY to the center of 
  //the first pixel.

  //Init all algorithm variables
  real firstX; mpf_init(firstX); 
  real firstY; mpf_init(firstY);
  real q1;     mpf_init(q1); 
  real cIm;    mpf_init(cIm);
  real cReal;  mpf_init(cReal);
  real zIm;    mpf_init(zIm);
  real zIm2;   mpf_init(zIm2);
  real zReal;    mpf_init(zReal);
  real zReal2;   mpf_init(zReal2);
  real radius;   mpf_init(radius);

  real m1;     mpf_init(m1);
  real m2;     mpf_init(m2);

  real i;      mpf_init(i);

  real two; mpf_init(two); mpf_set_ui(two, (unsigned int)2);

  real PI; mpf_init(PI); mpf_set_d(PI, M_PI);


  /*real firstX = m_curCoords.xMin + (m_xInc / 2);
    real firstY = m_curCoords.yMax - (m_yInc / 2);*/
  mpf_set_ui(q1, 2);
  mpf_div(q1, m_xInc, q1);
  mpf_add(firstX, m_curCoords.xMin , q1);

  mpf_set_ui(q1, 2);
  mpf_div(q1, m_yInc, q1);
  mpf_sub(firstY, m_curCoords.yMax, q1);

  //Now start the iterations
  for(unsigned int y = 0; y < m_numPixelsH; y++){

    cout<<"Percentage Complete:  "<< ((double)y / (double)m_numPixelsH)* 100 <<endl;
    //real cIm = firstY - ((real)y * m_yInc);
    mpf_mul_ui(m1, m_yInc, y);
    mpf_sub(cIm, firstY, m1);

    //printf("%d \n", y);

    for(unsigned int x = 0; x < m_numPixelsW; x++){
      //A few printouts of useful info
      //cout<<"Pixel:  "<<x<<" , "<<y<<endl;
       
      //real cReal = firstX + ((real)x * m_xInc);
      mpf_mul_ui(m2, m_xInc, x);
      mpf_add(cReal, m2, firstX);

      //These values will hold the real and imaginary values
      //at each pixel while the iterations are being calculated.
      mpf_set(zReal, cReal);
      mpf_set(zIm, cIm);

      bool inside = true;

      for(int its = 0; its <  mpf_get_ui(numIterations) ; its++)
	{
	  //Calc squares
	  //real zReal2 = zReal * zReal;
	  //real zIm2 = zIm * zIm;
	  //real radius = zReal2 + zIm2;
	   // mpf_pow_ui(zReal2, zReal, 2);
	  mpf_mul(zReal2, zReal, zReal);  //Faster than mpf_pow_ui()
	   // mpf_pow_ui(zIm2, zIm, 2);
	  mpf_mul(zIm2, zIm, zIm);  //Faster than mpf_pow_ui()
	  mpf_add(radius, zReal2, zIm2);

	  if(mpf_cmp_d(radius,4.0) > 0)
	    {
	      inside = false;

	      /* if(i == 360){
		int x = 10;
		}*/
					
	      //Uint32 pixel = GetPixel(m_colorMap,  m_colorMap->w % (i + 1), m_colorMap->h % (i + 1));
	      unsigned char pred = 0;
	      unsigned char pgreen = 0;
	      unsigned char pblue = 0;

	      if(smooth){
		/*	real mu = Smooth(sqrt(radius), i);
					
		real red =  sin((PI/180) * mu );
		real green = sin((PI/180) * mu  );
		real blue = sin((PI/180) * mu );

		pred  = (red >= 0) ? 255 * red : 255 * (-red) ; 
		pgreen  = (green >= 0) ? 255 * green : 255 * (-green) ; 
		pblue  = (blue >= 0) ? 255 * blue : 255 * (-blue) ;*/
	      }else{
		//     printf("Got here\n");
		//	double red   = sin((M_PI/180) * mpf_get_ui(i));
		//double green = sin((M_PI/180) * mpf_get_ui(i)  * 0.1);
		//double blue  = sin((M_PI/180) * mpf_get_ui(i) * 1.2);

		double red   = sin((M_PI/180) * its);
		double green = sin((M_PI/180) * its  * 0.1);
		double blue  = sin((M_PI/180) * its * 1.2);

		//	cout << "Red:" << pred << endl;
		pred  = (red >= 0) ? 255 * red : 255 * (-red) ; 
		pgreen  = (green >= 0) ? 255 * green : 255 * (-green) ; 
		pblue  = (blue >= 0) ? 255 * blue : 255 * (-blue) ;
		//	cout << "Red:" << (int)pred << endl;
	      }
									
	      Uint32 pixelTest = SDL_MapRGB(m_surface->format, 
					    pred, 
					    pgreen,
					    pblue);

	      //printf("%f %d \n", sin((PI/180) * i), i);
					
	      //SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format,255% (i+1),255% ((i+2)*2),255% ((i+1)*(i+1))));
	      SetPixel(m_surface, x, y,pixelTest);
					
	      //SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format,colorTable[i].r,colorTable[i].g,colorTable[i].b));
					
	      break;
	    }
				
	  //Execute formula
	  //zIm   = (two * zReal * zIm) + cIm;
	  mpf_mul(zIm, zIm, zReal);
	  mpf_mul(zIm, zIm, two);
	  mpf_add(zIm, zIm, cIm);

	  //zReal = (zReal2 - zIm2) + cReal;
	  mpf_sub(zReal, zReal2, zIm2);
	  mpf_add(zReal, zReal, cReal);

	}				
      if(inside)
	SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 255, 255, 123));			
    }
  }

  mpf_clear(firstX);
  mpf_clear(firstY);
  mpf_clear(q1); 
  mpf_clear(cIm);
  mpf_clear(cReal);
  mpf_clear(zIm);
  mpf_clear(zIm2);
  mpf_clear(zReal);
  mpf_clear(zReal2);
  mpf_clear(radius);
  mpf_clear(m1);
  mpf_clear(m2);
  mpf_clear(i);
  mpf_clear(two);
  mpf_clear(PI); 

  SDL_WM_SetCaption("Fractal Video Renderer by Taylor Deiaco v2.0 - Render Finished!",0);
}
/*
void CFractal::RenderMandelbrotArbPrec(unsigned int numIterations, bool optimize)
{
  SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0 - Render In Progress...",0);

  CalcIncrement();

  //BuildColors(numIterations);

  //This will set firstX and firstY to the center of 
  //the first pixel.
  double incx = double((double)m_xInc / 2);
  double incy = double((double)m_yInc / 2);
  double minx = double((double)m_curCoords.xMin);
  double maxy = double((double)m_curCoords.yMax);
  double firstX = minx + incx;
  double firstY = maxy - incy;

	
  //Now start the iterations
	
  for(double y =  double(0); y < m_numPixelsH; y++){

    double cIm = firstY - (y * incy);
    cout<< y << endl;

    for(double x = double(0); x < m_numPixelsW; x++){

      double cReal = firstX + (x * incx);


      //These values will hold the real and imaginary values
      //at each pixel while the iterations are being calculated.
      double zReal = cReal;
      double zIm   = cIm;

      bool inside = true;
      //if(optimize){
      //	real p = sqrt((zReal*zReal - (0.5*zReal) + 0.0625) + zIm * zIm);
      //	if(zReal < (p - (2 * p * p) + 0.25)){
      //		SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 10, 123, 10));			
      //		break;
      //	}

      //	if((zReal * zReal) + (2*zReal) + 1 + (zIm*zIm) < (1/16)){
      //		SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 23, 14, 123));			
      //		break;
      //	}
      //}

      for(unsigned int i = 0; i < numIterations; i++)
	{
	  //Calc squares
	  double zReal2 = zReal * zReal;
	  double zIm2 = zIm * zIm;
	  if( zReal2 + zIm2 > 4)
	    {
	      inside = false; 
	      SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format,sin(3.14*i/180),i,i));
	      break;
	    }
				
	  //Execute formula
	  double two = 2;
	  zIm   = (two * zReal * zIm) + cIm;
	  zReal = (zReal2 - zIm2) + cReal;

	}				
      if(inside)
	SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 123, 123, 123));			
    }
  }
  SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0 - Render Finished!",0);
}

void CFractal::RenderMandelbrot3(unsigned int numIterations)
{
  CalcIncrement();
  //This will set firstX and firstY to the center of 
  //the first pixel.
  real firstX = m_curCoords.xMin + (m_xInc / 2);
  real firstY = m_curCoords.yMax - (m_yInc / 2);

	
  //Now start the iterations
	
  for(unsigned int y = 0; y < m_numPixelsH; y++){

    real cIm = firstY - ((real)y * m_yInc);
    //printf("%d \n", y);

    for(unsigned int x = 0; x < m_numPixelsW; x++){

      real cReal = firstX + ((real)x * m_xInc);


      //These values will hold the real and imaginary values
      //at each pixel while the iterations are being calculated.
      real zReal = cReal;
      real zIm   = cIm;

      bool inside = true;

      for(unsigned int i = 0; i < numIterations; i++)
	{
	  //Calc squares
	  real zReal2 = zReal * zReal;
	  real zIm2 = zIm * zIm;
	  real zReal3 = zReal2 * zReal;
	  real zIm3 = zIm2 * zIm;

	  if( zReal2 + zIm2 > 4)
	    {
	      inside = false; 
	      SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 256%((i*13) + 1),256%((i*3) + 1), 256%((i*7)+1)));
	      break;
	    }
				
	  //Execute formula
	  real zImTemp = zIm;
	  zIm   = (zReal3 - (3* zReal * zIm2)) + cIm;
	  zReal = ((3*zReal2*zImTemp) - zIm3) + cReal;
	}
      if(inside)
	SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 123, 123, 123));			
    }
  }

}

void CFractal::RenderJulia(unsigned int numIterations, unsigned int x, unsigned int y)
{
  CalcIncrement();
  //This will set firstX and firstY to the center of 
  //the first pixel.
  real firstX = m_curCoords.xMin + (m_xInc / 2);
  real firstY = m_curCoords.yMax - (m_yInc / 2);

  real juliaCR = returnX(x);
  real juliaCI = returnY(y);

	
  //Now start the iterations
	
  for(unsigned int y = 0; y < m_numPixelsH; y++){

    real cIm = firstY - ((real)y * m_yInc);
    printf("%d \n", y);

    for(unsigned int x = 0; x < m_numPixelsW; x++){

      real cReal = firstX + ((real)x * m_xInc);


      //These values will hold the real and imaginary values
      //at each pixel while the iterations are being calculated.
      real zReal = cReal;
      real zIm   = cIm;

      bool inside = true;

      for(unsigned int i = 0; i < numIterations; i++)
	{
	  //Calc squares
	  real zReal2 = zReal * zReal;
	  real zIm2 = zIm * zIm;
	  if( zReal2 + zIm2 > 4)
	    {
	      inside = false; 
	      SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 256%((i) + 1),256%((i*3) + 1), 256%((i*7)+1)));
					
	    }
				
	  //Execute formula
	  real two = 2;
	  zIm   = (two * zReal * zIm) + juliaCI;
	  zReal = (zReal2 - zIm2) + juliaCR;
	}
      if(inside)
	SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 123, 123, 123));			
    }
  }

}

void CFractal::RenderBuddhabrot(unsigned int numIterations, unsigned int numSamples)
{
  SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0 - Buddah Render In Progress...",0);

  CalcIncrement();

  //Initialize counter array
  real counter[m_numPixelsW][m_numPixelsH];
  for(unsigned int x=0; x<m_numPixelsW; x++){
    for(unsigned int y=0; y<m_numPixelsH; y++){
      counter[x][y] = 0.0;
    }
  }

  //Fill entire surface with zeros and/or black
  SDL_FillRect(m_surface, 0, SDL_MapRGB(m_surface->format, 0, 0, 0));
  
  for(unsigned int n = 0; n < numSamples; n++){
    //These values will hold the real and imaginary values
    //at each pixel while the iterations are being calculated.
    real cReal, cIm, zReal, zIm, zReal2, zIm2;
    real tempReal = zReal  = cReal = RandRWithinWindow();
    real tempIm =   zIm    = cIm = RandIWithinWindow();

    //.  std::cout<<"x = "<<  zReal<< " y = "<< zIm <<endl;

    //Random number must be chosen to be iterated.

    bool escape = false;
  		
    for(unsigned int i = 0; i < numIterations; i++)
      {
	//Calc squares
	real zReal2 = zReal * zReal;
	real zIm2 = zIm * zIm;
	if( zReal2 + zIm2 > 4.0)
	  {
	    escape = true; 
	    break;
	  }		
	//Execute formula				
	zIm   = (2.0 * zReal * zIm) + cIm;
	zReal = (zReal2 - zIm2) + cReal;

      }	
    //If the point escapes, start iteration over, 
    //plotting the points as it goes
    zReal2 = zIm2 =  0.0;
    if(escape){
      zReal = tempReal;
      zIm   = tempIm;
      while(zReal2 + zIm2 < 4.0)
	{	
	  zReal2 = zReal * zReal;
	  zIm2 = zIm * zIm;
	  //Execute formula				
	  zIm   = (2.0 * zReal * zIm) + cIm;
	  zReal = (zReal2 - zIm2) + cReal;

	  //Determine which pixel the point is on
	  int xCoord = ((real)m_numPixelsW * zReal) / (m_curCoords.xMax - m_curCoords.xMin);
	  int yCoord = ((real)m_numPixelsH * zIm) / (m_curCoords.yMax - m_curCoords.yMin);
  		
	  //Weed out pixels that have escapes too far off screen
	  if(xCoord > m_numPixelsW || xCoord<0)
	    break;
	  if(yCoord > m_numPixelsH || xCoord<0)
	    break;

	  //  cout<<"XCoord = "<<xCoord<<" YCoord = "<< yCoord<<endl;

	  counter[xCoord][yCoord] += 1.0;	
	}
    }
  }

  //Find max counter value
  real max = 0.0;
  for(unsigned int x=0; x<m_numPixelsW; x++){
    for(unsigned int y=0; y<m_numPixelsH; y++){
      if(counter[x][y] > max){
	max = counter[x][y];

	//cout<<x<< "  " <<y<<"  "<< counter[x][y] << endl;
      }
    }
  }

  //Normalize counters to range from 0-1
  for(unsigned int x=0; x<m_numPixelsW; x++){
    for(unsigned int y=0; y<m_numPixelsH; y++){
      counter[x][y] = counter[x][y] / max;
      //      cout<<x<< "  " <<y<<"  "<< counter[x][y] << endl;
    }
  }

  //Set pixels
  for(unsigned int x=0; x<m_numPixelsW; x++){
    for(unsigned int y=0; y<m_numPixelsH; y++){
      SetPixel(m_surface, x, y, SDL_MapRGB(m_surface->format, 
					   counter[x][y]*255, 
					   counter[x][y]*255, 
					   counter[x][y]*255));
    }
  }

  SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0 - Buddah Render Finished!",0);
}

real CFractal::RandRWithinWindow()
{
  int r = std::rand();

  int xMax  = m_curCoords.xMax * 100;
  int xMin =  m_curCoords.xMin * 100;

  return ((r % (xMax - xMin ) ) + xMin) * 0.01;
  //  return returnX(r % m_numPixelsH);
}

real CFractal::RandIWithinWindow()
{
  int r = std::rand();

  int yMax = m_curCoords.yMax * 100;
  int yMin = m_curCoords.yMin * 100;

  return ((r % (yMax - yMin ) ) + yMin) * 0.01;
	
    // return returnY(  r % m_numPixelsH);
}
*/

bool CFractal::BuildColors(unsigned int numColors)
{
  return true;
}

bool CFractal::BuildLargeColorTable()
{
  /*Testing
   */
  /*FILE *colorTableFile = 0;
    fopen_s(&colorTableFile, "colorTable.txt", "w");
    if(colorTable == NULL)
    {
    printf("Could not open file...");
    return false;
    }*/
  /*
  SDL_Color color = {0,0,0,0};
  for(unsigned int i = 0; i < (256 * 256 * 256); i++)
    {
      if(color.r >= 255){
	color.r = 0;
	color.g += 1;
      }
      if(color.g >= 255){
	color.g = 0;
	color.b += 1;
      }
      color.r += 1;

      //		colorTable[i] = color;

	
      //fprintf(colorTableFile, "%d  %d  %d  %d  \n", color.r, color.g, color.b, i);
    }
 
  //fclose(colorTableFile);
  //SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0 - Large Color Table Finished!",0);
	
  */
  return true;
}

void CFractal::RenderColorTable()
{/*
  for(unsigned int y = 0; y < m_numPixelsH; y++){

    for(unsigned int x = 0; x < m_numPixelsW; x++){
      unsigned int i = (m_numPixelsW * y) + x;
      unsigned char pred = 0;
      unsigned char pgreen = 0;
      unsigned char pblue = 0;
					
      real red = sin(((PI/180) * (i+1)/255));
      real green = sin(((PI/180) * (i+1))) * cos(((PI/180) * (i+1)) / 510);
      real blue = sin((PI/180) * i  );
					
      pred  = (red >= 0) ? 255 * red : 255 * (1 - red) ; 
      pgreen  = (green >= 0) ? 255 * green : 255 * (1 - green) ; 
      pblue  = (blue >= 0) ? 255 * blue : 255 * (1 - blue) ;

			
      Uint32 pixelTest = SDL_MapRGB(m_surface->format, 
				    pred, 
				    pgreen,
				    pblue);

      SetPixel(m_surface, x, y,pixelTest);
    }
  }
  //SDL_WM_SetCaption("Fractal Renderer by Taylor Deiaco v2.0 - Color Table Finished!",0);
  */	
}

bool CFractal::LoadColorMap()
{
  //m_colorMap = IMG_Load("Color Maps//A Distant Fire.png");
  /*	if(!m_colorMap){
	printf("IMG_Load %s \n", IMG_GetError());
	return false;
	}*/
  printf("IMG_Load Successful! \n");
  return true;
}

#endif
