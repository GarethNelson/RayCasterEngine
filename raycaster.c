// Based on code from http://lodev.org/cgtutor/raycasting.html

// original license:
/*
Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdio.h>
#include <SDL.h>
#include <SOIL.h>
#include <SDL_image.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

int screen_w;
int screen_h;

double posX = 22, posY = 11.5;  //x and y start position
double dirX = -1, dirY = 0; //initial direction vector
double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane


double time = 0; //time of current frame
double oldTime = 0; //time of previous frame
double frameTime;
double moveSpeed;
double rotSpeed;
double oldDirX;
double oldPlaneX;
double current_dists[1200];
double current_weights[1200];
#define true 1
#define false 0 
SDL_Window *screen;
SDL_Surface *floor_tex;
SDL_GLContext glcontext;

#define mapWidth 24
#define mapHeight 24
#define bright_adjust 1.2

unsigned int textures[9];
int unclean=1;
GLuint floor_prog;

int worldMap[mapWidth][mapHeight]=
{
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
};

void handle_key_event(SDL_Event *e) {
     Uint8 *keystate = SDL_GetKeyboardState(NULL);
     if(keystate[SDL_GetScancodeFromKey(SDLK_UP)]) {
         if(worldMap[(int)(posX + dirX * moveSpeed)][(int)posY] == false) posX += dirX * moveSpeed;
         if(worldMap[((int)posX)][(int)(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
         unclean=1;
     }
     if(keystate[SDL_GetScancodeFromKey(SDLK_DOWN)]) {
         if(worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == false) posX -= dirX * moveSpeed;
         if(worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
         unclean=1;
     }
     if(keystate[SDL_GetScancodeFromKey(SDLK_LEFT)]) {
       oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
      unclean=1;
     }
     if(keystate[SDL_GetScancodeFromKey(SDLK_RIGHT)]) {
      oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
       oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
      unclean=1;
      }
     
}

void update() {
     SDL_Event e;
    oldTime = time;
    time = SDL_GetTicks();
    frameTime = (time - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds
    moveSpeed = frameTime * 3.0; //the constant value is in squares/second
    rotSpeed = frameTime * 2.5; //the constant value is in radians/second
    handle_key_event(&e);
     while (SDL_PollEvent(&e)) {
        switch(e.type) {
           case SDL_QUIT:
              exit(0);
           break;
           case SDL_WINDOWEVENT:
              for(int i=0; i<1200; i++) {
                 current_dists[i]   = screen_h / (2.0 * i - screen_h);
              }
              unclean=1;
           break;
        }

     }

}

Uint32 get_pixel32( SDL_Surface *surface, int x, int y )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;
    
    //Get the requested pixel
    return pixels[ ( y * surface->w ) + x ];
}

void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;
    
    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}

void render() {
     if(unclean==0) return;
     unclean=0;
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
     glLoadIdentity();

     glEnable(GL_BLEND);
     for(int y=(screen_h/2); y > 0; y--) {
         glBegin(GL_LINES);
          glColor4f(0.0,0.0,0.5,((float)y/(screen_h/2)*bright_adjust));
          glVertex2f(0,(float)y);
          glVertex2f(screen_w,(float)y);
         glEnd();
     }
     for(int y=(screen_h/2); y < screen_h; y++) {
         float line_alpha = (float)y;
         line_alpha -= (screen_h/2);
         glBegin(GL_LINES);
          glColor4f(0.5,0.5,0.5,line_alpha/(screen_h/2)*bright_adjust);
          glVertex2f(0,(float)y);
          glVertex2f(screen_w,(float)y);
         glEnd();
     } 
/*     glEnable(GL_TEXTURE_2D);
     glBindTexture(GL_TEXTURE_2D, textures[7]);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
     glColor4f(1.0,1.0,1.0,1.0);
     glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0, screen_h/2);
            glTexCoord2f(screen_w/256, 0.0f); glVertex2f(screen_w, screen_h/2 );
            glTexCoord2f(screen_w/256, (screen_h/2)/256); glVertex2f(screen_w,  screen_h );
            glTexCoord2f(0.0f, (screen_h/2)/256); glVertex2f(0,  screen_h );
     glEnd();*/

     for(int x=0; x < screen_w; x++) {
   double cameraX = 2 * x / ((double)screen_w) - 1; //x-coordinate in camera space
      double rayPosX = posX;
      double rayPosY = posY;
      double rayDirX = dirX + planeX * cameraX;
      double rayDirY = dirY + planeY * cameraX;
      //which box of the map we're in
      int mapX = ((int)rayPosX);
      int mapY = ((int)rayPosY);

      //length of ray from current position to next x or y-side
      double sideDistX;
      double sideDistY;

       //length of ray from one x or y-side to next x or y-side
      double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
      double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
      double perpWallDist;

      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit?
      //calculate step and initial sideDist
      if (rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (rayPosX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
      }
      if (rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (rayPosY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if (worldMap[mapX][mapY] > 0) hit = 1;
      }
      //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)
      if (side == 0) perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;
      else           perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;

      //Calculate height of line to draw on screen
      int lineHeight = (int)(screen_h / perpWallDist);

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + screen_h / 2;
//      if(drawStart < 0)drawStart = 0;
      int drawEnd = lineHeight / 2 + screen_h / 2;
//      if(drawEnd >= screen_h)drawEnd = screen_h - 1;

     double wallX; //where exactly the wall was hit
      if (side == 0) wallX = rayPosY + perpWallDist * rayDirY;
      else           wallX = rayPosX + perpWallDist * rayDirX;
      wallX -= floor((wallX));

      //x coordinate on the texture
      double texX = (wallX * 255.0);
//      if(side == 0 && rayDirX > 0) texX = 256 - texX - 1;
//      if(side == 1 && rayDirY < 0) texX = 256 - texX - 1; 

      

      glDisable(GL_BLEND);
      glLineWidth(1.0);
      glBegin(GL_LINES);
        glColor3f(0.0,0.0,0.0);
        glVertex2f(x,drawStart);
        glVertex2f(x,drawEnd);
      glEnd();
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textures[worldMap[mapX][mapY]-1]);
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      float col_adjust;
         glBegin(GL_LINES);
            col_adjust=((float)lineHeight/screen_h)*4;

            glColor4f(0.9,0.9,0.9,col_adjust);
            glTexCoord2f(texX/256.0f, 0.0f); glVertex2f(x, drawStart);


            glColor4f(0.9,0.9,0.9,col_adjust);
            glTexCoord2f(texX/256.0f, 1.0f); glVertex2f(x,  drawEnd);

         glEnd();
//FLOOR CASTING
      double floorXWall, floorYWall; //x, y position of the floor texel at the bottom of the wall

      //4 different wall directions possible
      if(side == 0 && rayDirX > 0)
      {
        floorXWall = mapX;
        floorYWall = mapY + wallX;
      }
      else if(side == 0 && rayDirX < 0)
      {
        floorXWall = mapX + 1.0;
        floorYWall = mapY + wallX;
      }
      else if(side == 1 && rayDirY > 0)
      {
        floorXWall = mapX + wallX;
        floorYWall = mapY;
      }
      else
      {
        floorXWall = mapX + wallX;
        floorYWall = mapY + 1.0;
      }
          
double distWall, distPlayer, currentDist;

      distWall = perpWallDist;
      distPlayer = 0.0;
        glBindTexture(GL_TEXTURE_2D, textures[7]);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
     glDisable(GL_BLEND);
     int y;
     int floorTexX,floorTexY;
     double currentFloorX,currentFloorY;
     double weight;
glBindTexture(GL_TEXTURE_2D, textures[7]);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );

     
     glBegin(GL_LINES);
     y=drawEnd+1;
     for(int y = drawEnd + 1; y < screen_h+40; y+=20) {
        currentDist = current_dists[y];

        weight = (currentDist - distPlayer) / (distWall - distPlayer);

        currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
        currentFloorY = weight * floorYWall + (1.0 - weight) * posY;



        glColor3f(0.6,0.6,0.6);
        glTexCoord2f(currentFloorX,currentFloorY); glVertex2f(x,y);


 currentDist = current_dists[y+25];

        weight = (currentDist - distPlayer) / (distWall - distPlayer);

        currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
        currentFloorY = weight * floorYWall + (1.0 - weight) * posY;



        glColor3f(0.6,0.6,0.6);
        glTexCoord2f(currentFloorX,currentFloorY); glVertex2f(x,y+25);



      }
      glEnd();
     }
     glUseProgram(0);
     glDisable(GL_TEXTURE_2D);
}

void load_gl_textures() {
     char tex_filename[16];
     int i;
     for(i=0; i<9; i++) {
        snprintf(tex_filename,15,"texture%d.png",i);
        printf("%s\n",tex_filename);
        textures[i] = SOIL_load_OGL_texture(tex_filename,0,0,SOIL_LOAD_AUTO);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
     }
     floor_tex = IMG_Load("texture7.png");
}

void load_gl_shaders() {
  GLuint floor_shader = glCreateShader(GL_FRAGMENT_SHADER);
     char* floor_shader_src =
"uniform sampler2D tex;"
"uniform float screen_h;"
"uniform float floorXWall;"
"uniform float floorYWall;"
"uniform float distWall;"
"uniform float posX;"
"uniform float posY;"
""
"void main() {"
"     float currentDist = screen_h / (2.0 * gl_FragCoord.y - screen_h);"
"     float weight = currentDist / distWall;"
"     float cur_floor_x = weight * floorXWall + (1.0 - weight) * posX;"
"     float cur_floor_y = weight * floorYWall + (1.0 - weight) * posY;"
"     gl_FragColor=texture2D(tex,vec2(cur_floor_x,cur_floor_y));"
"}";
     GLuint _err;
     int _len = strlen(floor_shader_src);
     glShaderSource(floor_shader,1,&floor_shader_src,&_len);
     _err = glGetError();
     if(_err != GL_NO_ERROR) {
        printf("Error compiling shader in glShaderSource %s\n",(char*)gluErrorString(_err));
        GLint maxLength = 0;
	glGetShaderiv(floor_shader, GL_INFO_LOG_LENGTH, &maxLength);
        char* infolog = (char*)malloc(maxLength);
        printf("%s\n",infolog);
     }
     glCompileShader(floor_shader);
     _err = glGetError();
     if(_err != GL_NO_ERROR) {
        printf("Error compiling shader in glCompileShader %d\n",_err);
     }
     floor_prog = glCreateProgram();
     _err = glGetError();
     if(_err != GL_NO_ERROR) {
        printf("Error compiling shader in glCreateProgram %d\n",_err);
     }
     glAttachShader(floor_prog,floor_shader);
     _err = glGetError();
     if(_err != GL_NO_ERROR) {
        printf("Error compiling shader in glAttachShader %d\n",_err);
     }
     glLinkProgram(floor_prog);
     _err = glGetError();
     if(_err != GL_NO_ERROR) {
        printf("Error compiling shader in glLinkProgram %d\n",_err);
     }


}

int main() {
    SDL_InitSubSystem(SDL_INIT_TIMER);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_PNG);
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 2);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);
    SDL_GL_SetSwapInterval(1);

    SDL_DisplayMode disp_mode;
    SDL_GetDesktopDisplayMode(0, &disp_mode);
    
    screen = SDL_CreateWindow("Raycasting test",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,
                                              SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_GL_GetDrawableSize(screen, &screen_w, &screen_h);
    glcontext = SDL_GL_CreateContext(screen);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, screen_w, screen_h, 0.0, 1.0, -1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glClearColor( 0.0, 0.0, 0.0, 1 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );
    glEnable(GL_MULTISAMPLE); */


    load_gl_textures();
    load_gl_shaders();
 
    char win_title[100];
    while(1) {
       SDL_GL_GetDrawableSize(screen, &screen_w, &screen_h);
       update();
       glMatrixMode( GL_PROJECTION );
       glLoadIdentity();
       glOrtho( 0.0, screen_w, screen_h, 0.0, 1.0, -1.0 );
       glMatrixMode( GL_MODELVIEW );
       glLoadIdentity();
       glViewport(0, 0, screen_w, screen_h);
       render();
       GLenum err = GL_NO_ERROR;
       while((err = glGetError()) != GL_NO_ERROR) {
          printf("%d\n",err);
       }
       snprintf(win_title,99,"Raycaster test: FPS=%d",((int)(1.0/ frameTime)));
       SDL_SetWindowTitle(screen,win_title);
       SDL_GL_SwapWindow(screen);
    }
}
