#ifndef ABATTOIR_H
#define ABATTOIR_H

typedef unsigned char Bool;
typedef char Char;
typedef void Void;

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef unsigned long UInt;

typedef char Int8;
typedef short Int16;
typedef long Int32;
typedef long Int;

typedef Void(* TypeFuncVoidVPtr)(Void*);
typedef Void*(* TypeFuncVPtrUInt)(UInt);
typedef Void*(* TypeFuncVPtrVPtr)(Void*);

typedef Bool(* TypeFuncBoolVPtrVPtr)(Void*, Void*);
typedef UInt(* TypeFuncUIntVPtr)(Void*);
typedef Void(* TypeFuncVoidVPtrUInt)(Void*, UInt);

#define ARG0 \
    ( void )
#define ARG1(t1,n1) \
    (t1 n1)
#define ARG2(t1,n1,t2,n2) \
    (t1 n1, t2 n2)
#define ARG3(t1,n1,t2,n2,t3,n3) \
    (t1 n1, t2 n2, t3 n3)
#define ARG4(t1,n1,t2,n2,t3,n3,t4,n4) \
    (t1 n1, t2 n2, t3 n3, t4 n4)
#define ARG5(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5) \
    (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)
#define ARG6(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6) \
    (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)
#define ARG7(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7) \
    (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7)
#define ARG8(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7,t8,n8) \
    (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8)
#define ARG9(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7,t8,n8,t9,n9) \
    (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9)

#define ASSERT(cond)    /* cond */

#endif
