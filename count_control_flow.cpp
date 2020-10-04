/*
 * Warmup 3
 * Pintool to count 
 * (a) the number of direct control flow transfer 
 * instructions executed by a program, and
 * (b) the number of indirect control flow transfers 
 * executed by a program
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <unistd.h>
#include "pin.H"
using std::setw;
using std::cerr;
using std::string;
using std::endl;
using std::ofstream;
using std::ios;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "count_control_flow.out", "specify profile file name");

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

static UINT64 direct_call_count = 0;
static UINT64 indirect_call_count = 0;
static UINT64 syscall_count = 0;
static UINT64 return_count = 0;


// This function is called before every block
VOID direct_flow_call() { direct_call_count++;}

VOID indirect_flow_call() { indirect_call_count++;} 

VOID syscall_total_count() { syscall_count++ ; }

VOID return_total_count() { return_count++ ; }


/* ===================================================================== */

VOID Instruction(INS ins, void *v)
{
    if( INS_IsRet(ins) )
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) return_total_count, IARG_END);
    }
    else if( INS_IsSyscall(ins) )
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) syscall_total_count, IARG_END);
    }
    else if (INS_IsDirectControlFlow(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) direct_flow_call, IARG_END);
    }
    else if( INS_IsIndirectControlFlow(ins) )
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) indirect_flow_call, IARG_END);
    }

}

/* ===================================================================== */

ofstream OutFile;

VOID Fini(int n, void *v)
{
    OutFile.setf(ios::showbase);
    OutFile << "Total direct flow call: " << direct_call_count << endl;
    OutFile << "Total indirect flow call: " << indirect_call_count << endl;
    OutFile << "Total syscall: " << syscall_count << endl;
    OutFile << "Total return: " <<  return_count << endl;
    OutFile.close();

}

// This function is called when any expection is seen, thus it writes the basic block count 
// obtained from docount function into the output file as application closes in such a way 
// doesn't calls Fini function thus we didn't get the count printed in the file.
// Thus, this function takes care of such scenarios like in case of firefox, etc.
VOID ExceptionHandle(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT *cFrom,CONTEXT *cTo, INT32 info, VOID *v)
{   OutFile.setf(ios::showbase);
    OutFile << "Total direct flow call: " << direct_call_count << endl;
    OutFile << "Total indirect flow call: " << indirect_call_count << endl;
    OutFile << "Total syscall: " << syscall_count << endl;
    OutFile << "Total return: " <<  return_count << endl;
    OutFile.close();

}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{

    PIN_Init(argc,argv);

    OutFile.open(KnobOutputFile.Value().c_str());

    //Application exception handling to be used for applications like firefox
    PIN_AddContextChangeFunction(ExceptionHandle, 0);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
