#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stack>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

using namespace std;

//=====================================================
//data Structurs
//=======================================================
//--------- Point structure -------------------
struct POINT
{
    int x;
    int y;
};

//--------- Line structure -------------------
struct LINE {
    POINT start;
    POINT end;
    float distance;
};

//--------- Server structure -------------------
struct SERVER{
    POINT location;
    int sid;
    GC color;
    float totalDistance;
    POINT *VisitedLocations;
    int noServiced;
};


//--------- State of a server structure -------------------
struct STATE {
    int i;
    int j;
    int k;
    int t;
    int lastSTATE;
    float servDist;
};

//=====================================================
//Global Variables
//=======================================================

SERVER yellServer;
SERVER redServer;
SERVER blueServer;

Display *dis;
Window win;
XEvent report;
GC green_gc, red_gc, blue_gc, yell_gc, red_gc_thick, blue_gc_thick, yell_gc_thick;
XColor green_col, red_col, blue_col, yell_col;
Colormap colormap;

char green[] = "#00FF00";
char red[] = "#FF0000";
char blue[] = "#0000FF";
char yell[] = "#FFFF00";
unsigned int win_width, win_height;

int point_index, state_index;
POINT *point_map;
STATE *state_map;
LINE lines[1000];
int no_of_lines;
LINE visited_edge[1000];
int visited_index=0;
bool pathFound=false;

float myTotalDistance;
float totalOptimalDistance;
bool done;
//initial poits and state_map
void iniPS()
{
    point_map = new POINT[50];
    state_map = new STATE[999999];
    point_index=0;
    state_index=0;
    done=false;
}


//===================================================
//---------- some structure related functions
POINT newPoint()
{
    POINT p;
    p.x=-1;
    p.y=-1;
    return p;
}

POINT newPoint(int X, int Y)
{
    POINT p;
     p.x = X;
     p.y = Y;
     return p;
}
LINE newLine(POINT a, POINT b)
{
    LINE l;
    l.start = a;
    l.end =b;
    return l;
}
SERVER newServer(POINT location, GC color, int id )
{
    SERVER s;
    s.location = location;
    s.color= color;
    s.sid =id;
    s.totalDistance=0;
    s.noServiced=1;
    s.VisitedLocations =new POINT[100];
    s.VisitedLocations[0]=location;
    return s;
}

STATE newState(int lastStateId, int pT, int pI, int pJ, int pK, float pDist)
{
    STATE s;
    s.i = pI;
    s.j = pJ;
    s.k = pK;
    s.t = pT;
    s.lastSTATE = lastStateId;
    s.servDist = pDist;
    return s;
}
STATE newState(int i, int t, float pDist, int pI, int pJ, int pK)
{
    return newState(i,t, pI, pJ, pK, pDist);
}
bool isEqualPoints(POINT p1, POINT p2)
{
    return ((p1.x == p2.x) && (p1.y == p2.y))? true: false;
}

float getDistance(POINT a, POINT b){
    return sqrt (pow(b.y - a.y, 2) + pow(b.x - a.x,2) );
}

float getLineLength(LINE l)
{
    return getDistance(l.start, l.end);
}
void addState(STATE s)
{
   state_map[state_index++] = s;
}

void addPoint(POINT p)
{
    point_map[point_index++] = p;
}

float dist(int x, int y)
{
    return getDistance(point_map[x],point_map[y]);
}

//==========================================================
// graphics functions
//========================================================


//Create window
void loadWindow()
{
    win_width = 800;
    win_height = 600;
    dis = XOpenDisplay(NULL);
    win = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1, win_width, win_height, 0, BlackPixel (dis, 0), BlackPixel(dis, 0));
    XMapWindow(dis, win);
    XSelectInput(dis, win, ExposureMask | KeyPressMask | ButtonPressMask);
}

//initate cloors
void initiateColor()
{
    colormap = DefaultColormap(dis, 0);

    green_gc = XCreateGC(dis, win, 0, 0);
    XParseColor(dis, colormap, green, &green_col);
    XAllocColor(dis, colormap, &green_col);
    XSetForeground(dis, green_gc, green_col.pixel);

    red_gc = XCreateGC(dis, win, 0, 0);
    XParseColor(dis, colormap, red, &red_col);
    XAllocColor(dis, colormap, &red_col);
    XSetForeground(dis, red_gc, red_col.pixel);

    blue_gc = XCreateGC(dis, win, 0, 0);
    XParseColor(dis, colormap, blue, &blue_col);
    XAllocColor(dis, colormap, &blue_col);
    XSetForeground(dis, blue_gc, blue_col.pixel);

    yell_gc = XCreateGC(dis, win, 0, 0);
    XParseColor(dis, colormap, yell, &yell_col);
    XAllocColor(dis, colormap, &yell_col);
    XSetForeground(dis, yell_gc, yell_col.pixel);

    red_gc_thick = XCreateGC(dis, win, 0, 0);
    XSetLineAttributes( dis, red_gc_thick, 2, LineOnOffDash, CapRound, JoinRound);
    XParseColor(dis, colormap, red, &red_col);
    XAllocColor(dis, colormap, &red_col);
    XSetForeground(dis, red_gc_thick, red_col.pixel);

    blue_gc_thick = XCreateGC(dis, win, 0, 0);
    XSetLineAttributes( dis, blue_gc_thick, 2, LineOnOffDash, CapRound, JoinRound);
    XParseColor(dis, colormap, blue, &blue_col);
    XAllocColor(dis, colormap, &blue_col);
    XSetForeground(dis, blue_gc_thick, blue_col.pixel);

    yell_gc_thick = XCreateGC(dis, win, 0, 0);
    XSetLineAttributes( dis, yell_gc_thick, 2, LineOnOffDash, CapRound, JoinRound);
    XParseColor(dis, colormap, yell, &yell_col);
    XAllocColor(dis, colormap, &yell_col);
    XSetForeground(dis, yell_gc_thick, yell_col.pixel);

}

void drawLine(POINT A, POINT B, GC LC)
{
    XDrawLine(dis,win, LC, A.x, A.y, B.x, B.y);
}

void drawThickLine(GC pGC, LINE e)
{
    XDrawLine(dis,win, pGC, e.start.x, e.start.y, e.end.x, e.end.y);
}

void drawCircle(POINT A, int r, GC LC)
{
     XFillArc( dis, win, LC, A.x -r/2, A.y- r/2, r, r, 0, 360*64);
}
void DrawPOINT(GC pGC,POINT a)
{
    XFillArc( dis, win, pGC, a.x-5, a.y-5, 10, 10, 0, 360*64);
}
//========================================
//Draw a line between two states
void drawLine(STATE a, STATE b)
{
    LINE l;
    GC c;
    POINT Aij = newPoint(a.i, a.j);
    POINT Ajk = newPoint(a.j, a.k);
    POINT Aik = newPoint(a.i, a.k);
    POINT Bij = newPoint(b.i, b.j);
    POINT Bjk = newPoint(b.j, b.k);
    POINT Bik = newPoint(b.i, b.k);
    if (isEqualPoints (Aij, Bij))
    {
        l = newLine(point_map[a.k], point_map[b.k]);
        c=blue_gc_thick;
    }
    else if(isEqualPoints (Ajk, Bjk))
    {
        l = newLine(point_map[a.i], point_map[b.i]);
        c=yell_gc_thick;
    }
    else if (isEqualPoints (Aik, Bik))
    {
        l = newLine(point_map[a.j], point_map[b.j]);
        c=red_gc_thick;
    }
    drawThickLine(c, l);
    totalOptimalDistance=totalOptimalDistance+getLineLength(l);
}


void drawServer(SERVER s)
{
    XDrawArc(dis, win, s.color,s.location.x-10, s.location.y-10, 20,20, 0, 360*64);
    drawCircle(s.location, 10, green_gc);

    for(int i=1; i<s.noServiced; i++)
    {
        drawCircle(s.VisitedLocations[i-1], 10, green_gc);
        drawCircle(s.VisitedLocations[i], 10, green_gc);
        drawLine(s.VisitedLocations[i-1], s.VisitedLocations[i], s.color);
    }
}

//draw lines in reverse order
void drawOptimumPath(int prevStateId)
{
    STATE prevState;
    prevState.i =-1;
    while(prevStateId != -1)
    {
        if (prevState.i>=0)
            drawLine(prevState, state_map[prevStateId]);
        prevState = state_map[prevStateId];
        prevStateId = prevState.lastSTATE;
    }
}

void drawSceen()
{
        XClearWindow(dis, win);
        drawServer(yellServer);
        drawServer(redServer);
        drawServer(blueServer);
        XFlush(dis);
}
//==============================================
//========  End of graphics functions =========
//=============================================

void printResult()
{
    cout<<"Total Cost (server nearby Strategy): "<<myTotalDistance<<endl;
    cout<<"Total Cost optimal : "<<totalOptimalDistance<<endl;
    float ratio;
    if(totalOptimalDistance>0)
        ratio = myTotalDistance/totalOptimalDistance;
    cout<<"Ratio: "<<ratio<<endl;
}

//===================================================
//each time a new point is added
//we have 3 chosice to serve that point
//this function add all three state to the satevariable
void addNewState(POINT p)
{
    STATE tempState, prevState;
    int i=0;
    int newPoint;
    addPoint(p);
    tempState = state_map[state_index-1];
    newPoint = point_index-1;

    while(i<state_index)
    {
       prevState = state_map[i];
       int t=tempState.t+1;
       if (prevState.t == tempState.t)
        {

            float di = dist(newPoint,prevState.i);
            float dj = dist(newPoint,prevState.j);
            float dk = dist(newPoint,prevState.k);

            if(di<=dj || di <=dk)
                addState(newState(i, t, prevState.servDist+di, newPoint, prevState.j, prevState.k));
            if (dj<=di || dj<=dk )
                addState(newState(i, t, prevState.servDist+dj, prevState.i, newPoint, prevState.k));
            if (dk<=di || dk <=dj)
                addState(newState(i, t, prevState.servDist+dk, prevState.i, prevState.j, newPoint));
        }
        i++;
    }
}

//find optimum path
int findOptimumPath()
{
    int prevStateId=0;
    float minDistance =99999;
    for(int i=1;i<state_index;i++)
    {
        if (state_map[i].servDist < minDistance && state_map[state_index-1].t == state_map[i].t)
        {
            minDistance = state_map[i].servDist;
            prevStateId = i;
        }
    }
    return prevStateId;
}

// after server parameters
void moveServer(SERVER &s, POINT p)
{
    myTotalDistance = myTotalDistance+getDistance(p,s.location);
    s.totalDistance=s.totalDistance+getDistance(p,s.location);
    s.VisitedLocations[s.noServiced]=p;
    s.noServiced= s.noServiced+1;
    s.location = p;
}

//Serve a point
void serve(POINT p)
{
    int dist_yell= getDistance(p, yellServer.location);
    int dist_red= getDistance(p, redServer.location);
    int dist_blue= getDistance(p, blueServer.location);

    cout<<endl<<"Point: ("<<p.x<<", "<<p.y<<")  ";
    if(dist_yell<dist_red && dist_yell <dist_blue)
    {
        cout<<"yellow nearby. Distance: "<<dist_yell<<endl;
        moveServer(yellServer,p);
    }
    else if(dist_red<dist_yell && dist_red<dist_blue)
    {
        cout<<"redw nearby. Distance: "<<dist_red<<endl;
        moveServer(redServer,p);
    }
    else if(dist_blue<dist_yell && dist_blue<dist_red)
    {
        cout<<"blue nearby. Distance: "<<dist_blue<<endl;
        moveServer(blueServer,p);
    }
}

//========================================================
// Main loop. an infinate loop that waits for user input
//=============================================================
void runMainLoop()
{
    while (1)
    {
        XNextEvent(dis, &report);
        switch  (report.type)
        {
            case Expose:
                drawSceen();
                XFlush(dis);

            break;

            case ButtonPress: //Case of mouse Click
                POINT p;
                p.x = report.xbutton.x;
                p.y = report.xbutton.y;
                if (report.xbutton.button == Button1 ) //at mouse left button click
                {
                    if(done) exit(0);

                    addNewState(p);
                    serve(p);
                    drawSceen();
                }
                else if (report.xbutton.button == Button3 )
                {
                    if(done) exit(0);
                    int oi = findOptimumPath();
                    drawOptimumPath(oi);
                    printResult();
                    done=true;
                }
            break;

            case KeyPress: //Exit on pressing Q or Esc
                if (XLookupKeysym(&report.xkey, 0) == XK_q || XLookupKeysym(&report.xkey, 0) == XK_Escape)
                    exit(0);
            break;
        }
    }
}

//========================================================
//Program Entry point
//========================================================
int main ()
{
    loadWindow();
    initiateColor();
    iniPS();

    yellServer = newServer(newPoint(100, 50), yell_gc,0);
    redServer = newServer(newPoint(700, 50), red_gc,1);
    blueServer = newServer(newPoint(350, 550), blue_gc,2);

    addPoint(yellServer.location);
    addPoint(redServer.location);
    addPoint(blueServer.location);
    addState(newState(-1, 0, yellServer.sid, redServer.sid, blueServer.sid,0.0));

    runMainLoop();
    return 0;
}

