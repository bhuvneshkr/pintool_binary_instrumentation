/*
 * Security Applications
 * Stack Use Analysis and Stack Pivoting Detection
 */

#include <iostream>
#include <fstream>
#include "pin.H"

using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;

ofstream OutFile;

// Declaration of global variables 
static ADDRINT base_stack_size=0;
static ADDRINT prev_sp=0;
static ADDRINT stack_size=0;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "stack_analysis.out", "specify output file name");

// This function is called before every instruction is executed
// SP and ADD Info passed and values are assigned for base_stack_size, and update
// sp & prev_sp and updated accordingly

VOID stackanalysis(ADDRINT sp, ADDRINT addrInfo)
{
	if(base_stack_size != 0)
	{	if(prev_sp > sp)
		{	
			prev_sp = sp;
		}

	}
	else
	{	
		base_stack_size=sp;
		prev_sp=sp;
	}
		

}    

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    // Insert a call to stackanalysis before every instruction with sp register values
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)stackanalysis,IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    stack_size = base_stack_size - prev_sp;
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << " Stack size reached :" << stack_size << " bytes"<< endl;
    OutFile.close();
}

// This function is called when any expection is seen, thus it writes the basic block count 
// obtained from docount function into the output file as application closes in such a way 
// doesn't calls Fini function thus we didn't get the count printed in the file.
// Thus, this function takes care of such scenarios like in case of firefox, etc.
VOID ExceptionHandle(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT *cFrom,CONTEXT *cTo, INT32 info, VOID *v)
{   
    stack_size = base_stack_size - prev_sp;
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << " Stack size reached :" << stack_size << " bytes"<< endl;
    OutFile.close();

}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    OutFile.open(KnobOutputFile.Value().c_str());

    //Application exception handling to be used for applications like firefox
    PIN_AddContextChangeFunction(ExceptionHandle, 0);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
