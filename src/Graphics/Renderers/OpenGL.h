#include "IRenderer.h"

class Render_OpenGL : public IRenderer
{
public:
	Render_OpenGL();
	virtual ~Render_OpenGL();
	bool InitGL();
	virtual void Render() override;
	virtual void UpdateData(std::vector<std::shared_ptr<ObjectData>> r) override;
protected:
	std::vector<std::shared_ptr<ObjectData>> SharedData;
};

/*#author: b8horpet

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from Graphics.SurfaceCommon import Surface
import numpy as np
from enum import  Enum
import sys
from Physics.Basics import Vector2D,memberfunctor
import datetime


circle_ang=36
circle_temp=tuple([Vector2D(np.cos(i*np.pi*2/circle_ang),np.sin(i*np.pi*2/circle_ang)) for i in range(0,circle_ang)])

#def foo(*args,**kwargs):
#    pass

class Keys(Enum):
    Escape = b'\x1b'
    Up = b'w'
    Down = b's'
    Left = b'a'
    Right = b'd'
    ZoomIn = b'+'
    ZoomOut = b'-'
    FullScreen = b'f'
    Slow = b' '
    Home = b'h'


def InitGL(Width, Height):
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClearDepth(1.0)
    glDepthFunc(GL_LESS)
    glEnable(GL_DEPTH_TEST)
    #glEnable(GL_DEPTH_CLAMP)
    glShadeModel(GL_SMOOTH)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    gluPerspective(45.0, float(Width)/float(Height), 0.1, 1000.0)

    glMatrixMode(GL_MODELVIEW)


class OpenGL2DSurface(Surface.SurfaceInterface):
    def __init__(self,u):
        self.window=0
        self.updater=u
        self.dist=64 # for fullhd fullscreen
        self.shift=Vector2D()
        self.Width=640
        self.Height=480
        self.FullScreen=True
        self.Slow=False
        self.TimeMS=50
        now=datetime.datetime.now()
        self.LastRender=[now for i in range(0,10)]
        glutInit(sys.argv)
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
        glutInitWindowSize(self.Width, self.Height)
        glutInitWindowPosition(0, 0)
        self.window = glutCreateWindow(b"Genesis")
        glutDisplayFunc(memberfunctor(self, OpenGL2DSurface.ClearColor))
        #glutDisplayFunc(foo)
        glutFullScreen()
        #glutIdleFunc(memberfunctor(self, OpenGL2DSurface.DrawGLScene))
        glutReshapeFunc(memberfunctor(self,OpenGL2DSurface.ReSizeGLScene))
        glutKeyboardFunc(memberfunctor(self,OpenGL2DSurface.keyPressed))
        InitGL(self.Width, self.Height)

    def StartRender(self):
        glutTimerFunc(self.TimeMS,memberfunctor(self, OpenGL2DSurface.DrawGLScene),0)
        glutMainLoop()

    def ClearColor(self, *args, **kwargs):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glutSwapBuffers()

    def DrawGLScene(self, *args, **kwargs):
        glutTimerFunc(self.TimeMS,memberfunctor(self, OpenGL2DSurface.DrawGLScene),0)
        now=datetime.datetime.now()
        timediff=now-self.LastRender.pop()
        self.LastRender.insert(0,now)
        timediff/=len(self.LastRender)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        nc=1
        glScale(64/self.dist,64/self.dist,1)
        glTranslatef(self.shift.x,self.shift.y,-64)
        p,s,c,t=self.updater()
        for i in range(0,len(p)):
            glTranslatef(p[i][0], p[i][1], 0)
            glBegin(GL_POLYGON)
            glColor3f(c[i][0],c[i][1],c[i][2])
            r=s[i]
            for j in range(0,circle_ang):
                glVertex3f(circle_temp[j].x*r, circle_temp[j].y*r, 0.0)
            glEnd()
            if len(t[i])>0:
                nc+=1
                glWindowPos2i(0,self.Height-18*nc)
                glutBitmapString(OpenGL.GLUT.GLUT_BITMAP_HELVETICA_18,bytes(t[i],'utf-8'))
            glTranslatef(-p[i][0], -p[i][1], 0)
        glTranslatef(-self.shift.x,-self.shift.y,64)

        glColor3f(1,1,1)
        glWindowPos2i(0,self.Height-18)
        glutBitmapString(OpenGL.GLUT.GLUT_BITMAP_HELVETICA_18,bytes("%d object @%5.2f FPS" % (len(p),1000000.0/timediff.microseconds),'utf-8'))
        glutSwapBuffers()

    def keyPressed(self, *args):
        k=args[0]
        if k == Keys.Escape.value:
            sys.exit()
        elif k == Keys.FullScreen.value:
            if self.FullScreen:
                self.FullScreen=False
                glutReshapeWindow(640,480)
            else:
                self.FullScreen=True
                glutFullScreen()
        elif k == Keys.Up.value:
            self.shift.y-=self.dist/10.0
        elif k == Keys.Down.value:
            self.shift.y+=self.dist/10.0
        elif k == Keys.Left.value:
            self.shift.x+=self.dist/10.0
        elif k == Keys.Right.value:
            self.shift.x-=self.dist/10.0
        elif k == Keys.ZoomIn.value:
            if self.dist>1:
                self.dist//=2
        elif k == Keys.ZoomOut.value:
            if self.dist<1024:
                self.dist*=2
        elif k == Keys.Slow.value:
            if self.Slow:
                self.Slow=False
                self.TimeMS//=4
            else:
                self.Slow=True
                self.TimeMS*=4
        elif k == Keys.Home.value:
            self.shift=Vector2D()
            self.dist=64

    def ReSizeGLScene(self, Width, Height):
        if Height == 0:
                Height = 1

        self.Width=Width
        self.Height=Height
        glViewport(0, 0, Width, Height)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45.0, float(Width)/float(Height), 0.1, 100.0)
        glMatrixMode(GL_MODELVIEW)
*/
