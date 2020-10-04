/*
 * Warmup
 * Part 1
 * Counting the number of basic blocks executed by the program
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

// make it static to help the compiler optimize basic_block_count
static UINT64 bbcount = 0;

// This function is called before every block thus using the same to increase block count
VOID basic_block_count() { bbcount ++;}

// Pin calls this function every time a new basic block is encountered
// It inserts a call to basic_block_count which increments the counter for every basic block
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to basic_block_count before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)basic_block_count, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "count_basic_block.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write count to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << endl << "Total basic block count: " << bbcount << endl;
    OutFile.close();
}

// This function is called when any expection is seen, thus it writes the basic block count 
// obtained from basic_block_count function into the output file as application closes in such a way 
// doesn't calls Fini function thus we didn't get the count printed in the file.
// Thus, this function takes care of such scenarios like in case of firefox, etc.
VOID ExceptionHandle(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT *cFrom,CONTEXT *cTo, INT32 info, VOID *v)
{   OutFile.setf(ios::showbase);
    OutFile << endl << "Total basic block count: " << bbcount << endl;
    OutFile.close();

}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initializing pin
    PIN_Init(argc, argv);

    OutFile.open(KnobOutputFile.Value().c_str());

    //Application exception handling to be used for applications like firefox
    PIN_AddContextChangeFunction(ExceptionHandle, 0);
 
    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
