/*
 * Warmup
 * Part 2
 * Pintool for wrapping calls to malloc
 * In the wrapper, code keeps a track of the number of calls made to malloc and
 * the total amount of memory allocated
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::hex;
using std::cerr;
using std::string;
using std::ofstream;
using std::ios;
using std::endl;
using std::endl;

/* ===================================================================== */
/* Names of malloc */
/* ===================================================================== */
#if defined(TARGET_MAC)
#define MALLOC "_malloc"
#else
#define MALLOC "malloc"
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

ofstream OutFile;

static UINT64 total_malloc_count = 0;
static UINT64 total_mem_size = 0;

// This function is called for everey malloc and increase the counter for every occurence
VOID malloccount() { total_malloc_count++;} 
	
// This function is called for every malloc and takes input the size allocated thus adding it
// to the total memory allocated
VOID addmem(UINT32 s) { total_mem_size += s;}

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "count_malloc_mem_wrapper.out", "specify trace file name");

/* ===================================================================== */


/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */
 
VOID Arg1Before(CHAR * name, ADDRINT size)
{
    malloccount();
    addmem(size);
}


/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */
   
VOID Image(IMG img, VOID *v)
{
    // Instrument the malloc() and free() functions.  Print the input argument
    // of each malloc() or free(), and the return value of malloc().
    //
    //  Find the malloc() function.
    RTN mallocRtn = RTN_FindByName(img, MALLOC);
    if (RTN_Valid(mallocRtn))
    {
        RTN_Open(mallocRtn);

        // Instrument malloc() to print the input argument value and the return value.
        RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
                       IARG_ADDRINT, MALLOC,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
        RTN_Close(mallocRtn);
    }

}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    OutFile.setf(ios::showbase);
    OutFile << "Total malloc count : " << total_malloc_count << endl;
    OutFile << "Total memory allocated : " << total_mem_size << endl;
    OutFile.close();

}

// This function is called when any expection is seen, thus it writes the basic block count 
// obtained from docount function into the output file as application closes in such a way 
// doesn't calls Fini function thus we didn't get the count printed in the file.
// Thus, this function takes care of such scenarios like in case of firefox, etc.
VOID ExceptionHandle(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT *cFrom,CONTEXT *cTo, INT32 info, VOID *v)
{   OutFile.setf(ios::showbase);
    OutFile << "Total malloc count : " << total_malloc_count << endl;
    OutFile << "Total memory allocated : " << total_mem_size << endl;
    OutFile.close();
 
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    // Initialize pin & symbol manager
    PIN_InitSymbols();
    PIN_Init(argc,argv);
    
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.open(KnobOutputFile.Value().c_str());

    //Application exception handling to be used for applications like firefox
    PIN_AddContextChangeFunction(ExceptionHandle, 0);
 
    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
