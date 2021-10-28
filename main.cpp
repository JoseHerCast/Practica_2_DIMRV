
#include <SDL.h>
#include <stdio.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>

#include "texture.h"
#include "figuras.h"
#include "Camera.h"
#include <String.h>

#if (_MSC_VER >= 1900)
#   pragma comment( lib, "legacy_stdio_definitions.lib" )
#endif

//Variables para manejo inicial de SDL

enum class State {PLAY,EXIT};

SDL_DisplayMode dm;
SDL_Window* window = NULL;
SDL_Event evnt;
SDL_GLContext glContext;

State _currentState = State::PLAY;

const Uint8* auxKey = SDL_GetKeyboardState(NULL);

int screenW = 0;
int screenH = 0;

int currentX=0, lastX=0;
int currentY=0, lastY=0;


CCamera objCamera;	//Create objet Camera
//Picking por color
GLubyte idSphere[3] = { 0x43, 0x77, 0xF8 };
GLubyte idSphere2[3] = { 0x00 ,0x00 ,0x02 };
GLubyte idCube[3] = { 0xB5, 0xC0, 0x08 };
GLubyte idCube2[3] = { 0x20, 0X48, 0xE0 };
GLubyte idCube3[3] = { 0xFF, 0x2D, 0x00 };
GLubyte idCono1[3] = { 0x9C, 0xB7, 0x2F };
GLubyte idCono2[3] = { 0x2C,0x46,0x50 };
GLubyte idArco1[3] = { 0x00, 0xC4, 0xB3 };
GLubyte idArco2[3] = { 0xE5, 0x9B, 0x29 };
GLubyte idCilindro1[3] = { 0x97, 0x34, 0xB4 };

GLubyte colorID[3];

//Variables para picking
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];
GLfloat winX, winY, winZ;
POINT mouse;
HWND hWnd;
GLdouble relX, relY, relZ;
GLfloat pos[3];

GLfloat g_lookupdown = 0.0f;    // Look Position In The Z-Axis (NEW)

CTexture t_sky; //Skybox
CTexture t_sky2; //Skybox

CFiguras sky_cube;



void Texture_Load() {

    t_sky.LoadTGA("Textures/Sky/sky5.tga");
    t_sky.BuildGLTexture();
    t_sky.ReleaseImage();

    t_sky2.LoadTGA("Textures/Sky/sky6.tga");
    t_sky2.BuildGLTexture();
    t_sky2.ReleaseImage();

}


void fatalError(const char* error) {
    printf(error, "\n\nPresione cualquier tecla par salir...");
    char tmp;
    scanf("%c",&tmp);
    SDL_Quit();
}

void initSDL() {
    
    //Inicializamos SDL con todos sus subsistemas
    SDL_Init(SDL_INIT_EVERYTHING);

	//Version OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    //Buffer de dibujo doble para poder realizar el repintado de manera mas suave
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    //Muestreo multiple para efecto alising
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    //Creamos nuestra ventana
    window = SDL_CreateWindow("Primer ejemplo en SDL",
        SDL_WINDOWPOS_UNDEFINED,    //Posición X
        SDL_WINDOWPOS_UNDEFINED,    //Posición Y
        600,                        //Width
        400,                        //Height
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);//Usaremos OpenGL para dibujar

    
    if (window == nullptr) {
        fatalError("\nSDL no pudo crear la ventana\n");
    }
	//Creamos el contexto de renderizado para OpenGL
    SDL_GLContext glContext=SDL_GL_CreateContext(window);
    
    if (glContext == nullptr) {
        fatalError("\nSDL no pudo crear el contexto para OpenGL\n");
    }

	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
	SDL_GL_SetSwapInterval(1);

  /*  GLenum error = glewInit();
    if (error != GLEW_OK) {
        fatalError("\nNo se pudo iniciar GLEW\n");
    }*/

}

void initGL(void)     // Inicializamos parametros
{
    //Inicializamos la matriz de proyección (perspectiva)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glFrustum(-0.1, 0.1, -0.1, 0.1, 0.1, 20000.0);
	//Inicializamos la matriz de vista-modelo (renderizado)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();



	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Negro de fondo
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);									// Configuramos Buffer de profundidad
    glEnable(GL_DEPTH_TEST);							// Habilitamos Depth Testing
    glDepthFunc(GL_LEQUAL);								// Tipo de Depth Testing a realizar
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    glEnable(GL_BLEND);

    Texture_Load();

    objCamera.Position_Camera(6, 2.5f, 3, 10, 2.5f, 0, 0, 1, 0);

    GLint redBits, greenBits, blueBits; 
    glGetIntegerv(GL_RED_BITS, &redBits); 
    glGetIntegerv(GL_GREEN_BITS, &greenBits); 
    glGetIntegerv(GL_BLUE_BITS, &blueBits);

    printf("Color depth (%d,%d,%d)",redBits,greenBits,blueBits);
}

void sdlDisplay() {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Limpiamos pantalla y profundidad
	glLoadIdentity();
    
    CFiguras ejemplo;

    glRotatef(g_lookupdown, 1.0f, 0, 0);

    gluLookAt(objCamera.mPos.x, objCamera.mPos.y, objCamera.mPos.z,
        objCamera.mView.x, objCamera.mView.y, objCamera.mView.z,
        objCamera.mUp.x, objCamera.mUp.y, objCamera.mUp.z);

    glPushMatrix(); //Se pinta el cielo

        //sky_cube.skybox(10000.0, 5000.0, 5000.0, t_sky2.GLindex);

        sky_cube.skybox(10000.0, 5000.0, 5000.0, t_sky.GLindex);

    glPopMatrix();

    //Prueba de picking
    glPushMatrix();
    glColor3ub(idSphere[0], idSphere[1], idSphere[2]);
    ejemplo.esfera(0.5, 15, 15, NULL);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(16.7, 15.75, 12);
    glColor3ub(idSphere2[0], idSphere2[1], idSphere2[2]);
    ejemplo.esfera(0.5, 15, 15, NULL);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(20, -20, 15);
    glColor3ub(idCube[0], idCube[1], idCube[2]);
    ejemplo.prisma(10, 10, 10, NULL);
    glPopMatrix();

    //Cubo2
    glPushMatrix();
    glTranslatef(5, 5, 5);
    glColor3ub(idCube2[0], idCube2[1], idCube2[2]);
    ejemplo.prisma(2, 3, 5, NULL);
    glPopMatrix();

    //Cubo3
    glPushMatrix();
    glTranslatef(2, 0, 10);
    glColor3ub(idCube3[0], idCube3[1], idCube3[2]);
    ejemplo.prisma(3, 2, 3, NULL);
    glPopMatrix();

    //Cono2
    glPushMatrix();
    glTranslatef(-5, -5, -5);
    glRotatef(180, 1, 0, 0);
    glColor3ub(idCono1[0], idCono1[1], idCono1[2]);
    ejemplo.cono(4, 2, 10, NULL);
    glPopMatrix();

    //Cono3
    glPushMatrix();
    glTranslatef(15, -5, -10);
    glColor3ub(idCono2[0], idCono2[1], idCono2[2]);
    ejemplo.cono(5, 1, 10, NULL);
    glPopMatrix();

    //arco1
    glPushMatrix();
    glTranslatef(10, -5, 0);
    glColor3ub(idArco1[0], idArco1[1], idArco1[2]);
    ejemplo.arco(4, 2, 10, 10);
    glPopMatrix();

    //arco2
    glPushMatrix();
    glTranslatef(-5, 5, -2);
    glRotatef(180, 1, 1, 0);
    glColor3ub(idArco2[0], idArco2[1], idArco2[2]);
    ejemplo.arco(4, 2, 10, 10);
    glPopMatrix();

    //cilindro1
    glPushMatrix();
    glTranslatef(5, 0, -4);
    glColor3ub(idCilindro1[0], idCilindro1[1], idCilindro1[2]);
    ejemplo.cilindro(1, 2, 10, NULL);
    glPopMatrix();

    //Relleno
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);//Color rojo
    glBegin(GL_LINES);//Eje X
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(100.0, 0.0, 0.0);
    glEnd();
    glPopMatrix();
    glPushMatrix();
    glColor3f(0.0, 1.0, 0.0);//Color verde
    glBegin(GL_LINES);//Eje Y
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 100.0, 0.0);
    glEnd();
    glPopMatrix();
    glPushMatrix();
    glColor3f(0.0, 0.0, 1.0);//Color azul
    glBegin(GL_LINES);//Eje Z
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 100.0);
    glEnd();
    glPopMatrix();

    //Prueba de picking
    glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(relX, relY, relZ);
    ejemplo.esfera(0.1, 15, 15, NULL);
    glPopMatrix();

    SDL_GL_SwapWindow(window);
}

void reshape(int width, int height)   // Creamos funcion Reshape
{
    if (height == 0)										// Prevenir division entre cero
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);						// Seleccionamos Projection Matrix
    glLoadIdentity();

    // Tipo de Vista

    glFrustum(-0.1, 0.1, -0.1, 0.1, 0.1, 20000.0);

    glMatrixMode(GL_MODELVIEW);							// Seleccionamos Modelview Matrix
    glLoadIdentity();
}

void picking_pipeline() {

    //Con esto solo se obtendrá un vector de dirección con coordenadas relativas al viewport

    glGetIntegerv(GL_VIEWPORT, viewport);//{x,y,width,height} Retrieves The Viewport Values (X, Y, Width, Height)
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);// Retrieve The Modelview Matrix
    glGetDoublev(GL_PROJECTION_MATRIX, projection);// Retrieve The Projection Matrix

    mouse.x = lastX;    // Gets The Current Cursor Coordinates (Mouse Coordinates)
    mouse.y = lastY;
    ScreenToClient(hWnd, &mouse);

    winX = (float)mouse.x;// Holds The Mouse X Coordinate
    winY = (float)mouse.y;// Holds The Mouse Y Coordinate

    winY = (float)viewport[3] - winY;// Subtract The Current Mouse Y Coordinate From The Screen Height.

    glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &relX, &relY, &relZ);


    //////////////////////////////////////////////////////////////////////////////

    //Ahora hay que calcular la posición en el mundo real del objeto
}

void picking_color() {

    glDisable(GL_TEXTURE_2D); 
 
    glGetIntegerv(GL_VIEWPORT, viewport);//{x,y,width,height} Retrieves The Viewport Values (X, Y, Width, Height)

    mouse.x = lastX;    // Gets The Current Cursor Coordinates (Mouse Coordinates)
    mouse.y = lastY;
    ScreenToClient(hWnd, &mouse);

    winX = (float)mouse.x;// Holds The Mouse X Coordinate
    winY = (float)mouse.y;// Holds The Mouse Y Coordinate

    winY = (float)viewport[3] - winY;// Subtract The Current Mouse Y Coordinate From The Screen Height.
    
    glReadPixels(winX, winY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, colorID);
    glEnable(GL_TEXTURE_2D);
}


void processInput() {

    SDL_GetMouseState(&lastX, &lastY);
    SDL_GetWindowSize(window, &screenW, &screenH);
    //Manejo de interacción con el usuario.
    while (SDL_PollEvent(&evnt) == 1) {
        switch (evnt.type) {
        case SDL_QUIT:
            _currentState = State::EXIT;
            break;
        //Cuando se redimensione la ventana
        case SDL_WINDOWEVENT:
            if (evnt.window.event == SDL_WINDOWEVENT_RESIZED) {
                reshape(evnt.window.data1, evnt.window.data2);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
          
            if (evnt.button.button == SDL_BUTTON_LEFT) {
                picking_pipeline();
                if (relX >= 14 && relX <= 26) {
                    if (relY >= -26 && relY <= -14) {
                        if (relZ >= 9 && relZ <= 21) {
                            printf("\n\nSeleccionaste el cubo amarillo\n");
                        }
                    }
                }
                if (relX >= 3 && relX <= 7) {
                    if (relY >= 2.5 && relY <= 7.5) {
                        if (relZ >= -1 && relZ <= 11) {
                            printf("\n\nSeleccionaste el cubo azul fuerte\n");
                        }
                    }
                }
                if (relX >= -0.5 && relX <= 4.5) {
                    if (relY >= -2 && relY <= 2) {
                        if (relZ >= 7.5 && relZ <= 12.5) {
                            printf("\n\nSeleccionaste el cubo rojo\n");
                        }
                    }
                }

                picking_color();

                if (colorID[0] == idSphere[0] && colorID[1] == idSphere[1] && colorID[2] == idSphere[2])
                    printf("\n\nSe clickeo la esfera azul\n");
                else if (colorID[0] == idSphere2[0] && colorID[1] == idSphere2[1] && colorID[2] == idSphere2[2])
                    printf("\n\nSe clickeo la esfera negra\n");
                else if (colorID[0] == idCono1[0] && colorID[1] == idCono1[1] && colorID[2] == idCono1[2])
                    printf("\n\nSe clickeo el cono mostaza\n");
                else if (colorID[0] == idCono2[0] && colorID[1] == idCono2[1] && colorID[2] == idCono2[2])
                    printf("\n\nSe clickeo el cono gris\n");
                else if (colorID[0] == idArco1[0] && colorID[1] == idArco1[1] && colorID[2] == idArco1[2])
                    printf("\n\nSe clickeo el arco verde\n");
                else if (colorID[0] == idArco2[0] && colorID[1] == idArco2[1] && colorID[2] == idArco2[2])
                    printf("\n\nSe clickeo el arco anaranjado\n");
                else if (colorID[0] == idCilindro1[0] && colorID[1] == idCilindro1[1] && colorID[2] == idCilindro1[2])
                    printf("\n\nSe clickeo el cilindro  morado\n");

                printf("\n*******************\nRColor: (%x,%x,%x)\n*******************\n\n",
                    colorID[0],
                    colorID[1],
                    colorID[2]);

                printf("\n*******************\nRelative coordinates: (%.2lf,%.2lf,%.2lf)\n*******************\n\n",
                    relX,
                    relY,
                    relZ);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (evnt.button.button == SDL_BUTTON_LEFT) {

                relX = relY = relZ = 0;
            }
            break;
        case SDL_MOUSEMOTION:
            if (evnt.button.button == SDL_BUTTON(3)) {

                currentX = evnt.motion.x;
                currentY = evnt.motion.y;

                if (currentX < (lastX))
                    objCamera.Rotate_View(-CAMERASPEED);
                else if (currentX > (lastX))
                    objCamera.Rotate_View(CAMERASPEED);

                if (currentY < (lastY))
                    g_lookupdown -= 1.3;
                else if (currentY > (lastY))
                    g_lookupdown += 1.3;

                /*printf("Mouse coordinates: (%d,%d)\nWindow size: %d W %d H\n\n",
                    currentX,
                    currentY,
                    screenW,
                    screenH);*/
            }
            break;
        //Todo lo relacionado con presionar alguna tecla
        case SDL_KEYDOWN:
            switch (evnt.key.keysym.sym) {
            //Manejo de teclas de dirección
            case SDLK_LEFT:
                break;
            case SDLK_RIGHT:
                break;
            case SDLK_UP:
                break;
            case SDLK_DOWN:
                break;
            //Manejo de teclas alfabéticas
            case 'a':
            case 'A':
                objCamera.Strafe_Camera(-(CAMERASPEED + 0.1));
                //printf("\nCoordenadas de cámara: %f,%f", objCamera.mPos.x, objCamera.mPos.z);
                break;
            case 'd':
            case 'D':
                objCamera.Strafe_Camera(CAMERASPEED + 0.1);
                //printf("\nCoordenadas de cámara: %f,%f", objCamera.mPos.x, objCamera.mPos.z);
                break;
            case 'w':
            case 'W':
                if (auxKey[SDL_SCANCODE_RSHIFT] || auxKey[SDL_SCANCODE_LSHIFT])
                    objCamera.UpDown_Camera(CAMERASPEED+0.1);
                else
                objCamera.Move_Camera(CAMERASPEED + 0.1);
                //printf("\nCoordenadas de cámara: %f,%f", objCamera.mPos.x, objCamera.mPos.z);
                break;
            case 's':
            case 'S':
                if (auxKey[SDL_SCANCODE_RSHIFT] || auxKey[SDL_SCANCODE_LSHIFT])
                    objCamera.UpDown_Camera(-(CAMERASPEED+0.1));
                else
                objCamera.Move_Camera(-(CAMERASPEED + 0.1));
                //printf("\nCoordenadas de cámara: %f,%f", objCamera.mPos.x, objCamera.mPos.z);
                break;
            //Manejo de teclas especiales como su valor en ASCII (opcional, SDL ya cuenta con constantes)
            case 32:
                break;
            case 27:        // Cuando Esc es presionado...
                exit(0);   // Salimos del programa
                break;
            default:        // Cualquier otra
                break;

            }
            break;
        default:
                break;
        }
    }
}

void sdlMainLoop() {
    
    while (_currentState != State::EXIT) {
        processInput();
        sdlDisplay();
    }
}

int main(int argc, char **argv)
{ 
    initSDL();

    SDL_GetDesktopDisplayMode(0,&dm);
    if (window == nullptr) {
        fatalError("\nSDL no pudo obtener el modo de pantalla\n");
    }


    initGL();

    sdlMainLoop();

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

    return 0;
}



