#include <iostream>
#include <map>

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
Vector::Vector(char** theArray){this->m_theArray = theArray;};
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
public:
Interpreter(int(**thePrims)(Vector,IObject***),Vector initialContext){
    m_thePrims = thePrims;
    m_stack = new Vector(initialContext.getArray());
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
int main()
{
    cout << "Hello world!" << endl;
    return 0;
}
