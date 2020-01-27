#include <iostream>
#include <map>
extern "C"{
#ifdef __unix
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif // __unix
};
#ifdef wasm
extern "C"{
extern int getJSAttr(int,char*);
extern void setJSAttr(int,char*,int);
extern int importJSObject(char*);
extern int importJSObject(int);
extern int getJSGlobal();
extern int exportJSObject_int(int);
extern char* exportJSObject_charstar(int);
};
#endif // wasm
using namespace std;
class Assembler{
static int eval(int code){
int val;
    asm ("movl %1, %%ebx;"
        "call %%ebx"
         "movl %%ecx, %0;"
         : "=r" ( val )        /* output */
         : "r" ( code )         /* input */
         : "%ebx" "%ecx"        /* clobbered register */
     );
     return val;
     };
};
namespace display{
class DisplayObject{
public:
virtual void renderOn(void* theSurface) = 0;
};
class DisplaySurface: public DisplayObject{
public:
virtual void render(DisplayObject* theObject){theObject->renderOn(this);};
virtual uint32_t* getBuffer(){return NULL;};
};
};
namespace input{
class Buttons{
private:
int** m_allButtons;
public:
int** getAllButtons(){return m_allButtons;};
void setAllButtons(int** theValue){m_allButtons = theValue;};
int getButton(int p){return *m_allButtons[p];};
};
};
namespace os{
namespace input{
class CommandReader{static char read(){
char theBuf;
::read(3,&theBuf,1);
return theBuf;
};
};
};
namespace display{
class FBDisplayBase: public ::display::DisplaySurface{
public:
void renderOn(void* theDisplay){};
void render(::display::DisplayObject* theObject){::display::DisplaySurface::render(theObject);};
FBDisplayBase(){
#ifdef __unix
    int fb = ::open("/dev/fb0", O_RDWR);
    if(fb){
    struct ::fb_var_screeninfo info;
    if(!(0 == ::ioctl(fb, FBIOGET_VSCREENINFO, &info)))return;
    size_t len = 4 * info.xres * info.yres;
    m_buf = (uint32_t*)::mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if(m_buf == MAP_FAILED){m_buf = 0; return;};
    };
#endif // __unix
};
#ifdef __unix
private:
uint32_t* m_buf;
public:
uint32_t* getBuffer(){return m_buf;};
#endif
#ifdef os
private:
int m_addr = 0;
public:
uint32_t* getBuffer(){return (uint32_t*)m_addr};
#endif // __unix
};
};
};
namespace interpreter{
class IObject{
protected:
IObject** m_ivars;
std::map<char*,IObject*> m_methods;
public:
void send(void* &theStack,IObject*** theObjectStack,char* theToken);
};
class Vector: public IObject{
private:
char** m_theArray;
public:
Vector(char**);
Vector();
char** getArray();
friend void IObject::send(void* &theStack,IObject*** theObjectStack,char* theToken);
};
Vector::Vector(char** theArray){this->m_theArray = theArray;}
Vector::Vector(){this->m_theArray = (char**)&"quit";};
char** Vector::getArray(){return this->m_theArray;};

class VectorIterator{
private:
Vector* m_theVector;
long m_index;
public:
VectorIterator(Vector* theVector){m_theVector = theVector; m_index = 0;};
Vector* getVector(){return m_theVector;};
long getIndex(){return m_index;};
void setIndex(long theValue){m_index = theValue;};
char* next(){
    char* v_theString = m_theVector->getArray()[m_index];
    m_index++;
    return v_theString;
}
};
void IObject::send(void* &theStack,IObject*** theObjectStack,char* theToken){
    Vector* v_theStack = (Vector*)theStack;
    Vector* v_newStack = new Vector[(sizeof(v_theStack) / 8) + 1];
    for(int i = 0; i < sizeof(v_theStack) / 8; i++){
        v_newStack[i].m_theArray = v_theStack[i].getArray();
    };
    theStack = v_newStack;
    Vector* v_theNewFrame = v_newStack + (sizeof(v_theStack) / 8) + 1;
    Vector* v_theMethod = (Vector*)m_methods.at(theToken);
    v_theNewFrame->m_theArray = v_theMethod->getArray();
};
class Interpreter{
private:
Vector* m_stack;
IObject*** m_objectStack;
VectorIterator* m_theIterator;
int(**m_thePrims)(Vector,IObject***);
display::DisplaySurface* m_theDisplay;
ostream* m_out;
public:
display::DisplaySurface* getDisplay(){return m_theDisplay;};
void setDisplay(display::DisplaySurface* theDisplay){m_theDisplay = theDisplay;};
ostream* getOut(){return m_out;};
void setOut(ostream* theOut){m_out = theOut;};
Interpreter(void* thePrims,Vector* initialContext){
    m_thePrims = (int(**)(Vector,IObject***))thePrims;
    m_stack = new Vector(initialContext->getArray());
    m_theIterator = new VectorIterator(m_stack);
    m_objectStack = 0;
};
void evaluateToken(char* theToken){
    IObject* theObject = **m_objectStack;
    m_objectStack[0] = &m_objectStack[0][1];
    theObject->send((void*&)m_stack,m_objectStack,theToken);
};
void interpretOne(){
    char* v_theToken = m_theIterator->next();
    evaluateToken(v_theToken);
};
};
};
int main()
{
char* quit = "quit";
interpreter::Interpreter* i = new interpreter::Interpreter((void*)NULL,new interpreter::Vector(&quit));
os::display::FBDisplayBase* d = new os::display::FBDisplayBase();
i->setDisplay(d);
i->setOut(&cout);
    cout << "ObjectLand Kernel alpha 1" << endl;
    return 0;
}
