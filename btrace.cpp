/* 
 * Security Applications
 * System Call Interception
 *     btrace:
 * Implement something similar to strace
 * using binary instrumentation
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::ostream;
using std::cout;
using std::cerr;
using std::string;
using std::endl;

#if !defined(TARGET_WINDOWS)
#include <sys/syscall.h>
#endif

FILE * trace;

static UINT64 syscall_encountered=0;

// Print syscall number and arguments
VOID SysBefore(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5)
{	
#if defined(TARGET_LINUX) && defined(TARGET_IA32)
    // On ia32 Linux, there are only 5 registers for passing system call arguments,
    // but mmap needs 6. For mmap on ia32, the first argument to the system call
    // is a pointer to an array of the 6 arguments
    if (num == SYS_mmap)
    {
        ADDRINT * mmapArgs = reinterpret_cast<ADDRINT *>(arg0);
        arg0 = mmapArgs[0];
        arg1 = mmapArgs[1];
        arg2 = mmapArgs[2];
        arg3 = mmapArgs[3];
        arg4 = mmapArgs[4];
        arg5 = mmapArgs[5];
    }
#endif
       
    // Mapping command number to respective system call
    char sys_cmd_name[10];
    switch((long)num) {
		case 1  :  strcpy(sys_cmd_name,"exit");
				break;
		case 3  :  strcpy(sys_cmd_name,"read");
				break;
		case 4  :  strcpy(sys_cmd_name,"write");
				break;
		case 5  :  strcpy(sys_cmd_name,"open");
				break;
		case 6  :  strcpy(sys_cmd_name,"close");
				break;
		case 13  :  strcpy(sys_cmd_name,"time");
				break;
		case 15  :  strcpy(sys_cmd_name,"chmod");
				break;
		case 33  :  strcpy(sys_cmd_name,"access");
				break;
		case 37  :  strcpy(sys_cmd_name,"kill");
				break;
		case 38  :  strcpy(sys_cmd_name,"rename");
				break;
		case 45  :  strcpy(sys_cmd_name,"brk");
				break;
		case 54  :  strcpy(sys_cmd_name,"ioctl");
				break;
		case 91  :  strcpy(sys_cmd_name,"munmap");
				break;
		case 102  :  strcpy(sys_cmd_name,"socketcall");
				break;
		case 116  :  strcpy(sys_cmd_name,"sysinfo");
				break;
		case 117  :  strcpy(sys_cmd_name,"ipc");
				break;
		case 118  :  strcpy(sys_cmd_name,"fsync");
				break;
		case 120  :  strcpy(sys_cmd_name,"clone");
				break;
		case 122  :  strcpy(sys_cmd_name,"newuname");
				break;
		case 125  :  strcpy(sys_cmd_name,"mprotect");
				break;
		case 140  :  strcpy(sys_cmd_name,"llseek");
				break;
		case 146  :  strcpy(sys_cmd_name,"writev");
				break;
		case 168  :  strcpy(sys_cmd_name,"poll");
				break;
		case 172  :  strcpy(sys_cmd_name,"prctl");
				break;
		case 174  :  strcpy(sys_cmd_name,"rt_sigaction");
				break;
		case 175  :  strcpy(sys_cmd_name,"rt_sigprocmask");
				break;
		case 181  :  strcpy(sys_cmd_name,"pwrite64");
				break;
		case 191  :  strcpy(sys_cmd_name,"getrlimit");
				break;
		case 192  :  strcpy(sys_cmd_name,"mmap");
				break;
		case 195  :  strcpy(sys_cmd_name,"stat64");
				break;
		case 196  :  strcpy(sys_cmd_name,"lstat64");
				break;
		case 197  :  strcpy(sys_cmd_name,"fstat64");
				break;
		case 199  :  strcpy(sys_cmd_name,"getuid");
				break;
		case 201  :  strcpy(sys_cmd_name,"geteuid");
				break;
		case 202  :  strcpy(sys_cmd_name,"getegid");
				break;
		case 209  :  strcpy(sys_cmd_name,"getresuid");
				break;
		case 211  :  strcpy(sys_cmd_name,"getresgid");
				break;
		case 219  :  strcpy(sys_cmd_name,"madvise");
				break;
		case 220  :  strcpy(sys_cmd_name,"getdents64");
				break;
		case 221  :  strcpy(sys_cmd_name,"fcntl64");
				break;
		case 240  :  strcpy(sys_cmd_name,"futex");
				break;
		case 243  :  strcpy(sys_cmd_name,"set_thread_area");
				break;
		case 252  :  strcpy(sys_cmd_name,"exit");
				break;
		case 258  :  strcpy(sys_cmd_name,"set_tid_address");
				break;
		case 266  :  strcpy(sys_cmd_name,"clock_getres");
				break;
		case 268  :  strcpy(sys_cmd_name,"statfs64");
				break;
		case 269  :  strcpy(sys_cmd_name,"fstatfs64");
				break;
		case 272  :  strcpy(sys_cmd_name,"fadvise64_64");
				break;
		case 292  :  strcpy(sys_cmd_name,"inotify_add_watch");
				break;
		case 293  :  strcpy(sys_cmd_name,"inotify_rm_watch");
				break;
		case 311  :  strcpy(sys_cmd_name,"set_robust_list");
				break;
		case 324  :  strcpy(sys_cmd_name,"fallocate");
				break;
		case 328  :  strcpy(sys_cmd_name,"eventfd2");
				break;
		case 332  :  strcpy(sys_cmd_name,"inotify_init1");
				break;
	default		:	strcpy(sys_cmd_name,"not_listed");
    }

    if (strcmp(sys_cmd_name,"not_listed")==0)
    {	fprintf(trace,"\n%ld(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
        (long)num,
        (unsigned long)arg0,
        (unsigned long)arg1,
        (unsigned long)arg2,
        (unsigned long)arg3,
        (unsigned long)arg4,
        (unsigned long)arg5);
    }
    else
    {
	fprintf(trace,"\n%s(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
        sys_cmd_name,
        (unsigned long)arg0,
        (unsigned long)arg1,
        (unsigned long)arg2,
        (unsigned long)arg3,
        (unsigned long)arg4,
        (unsigned long)arg5);
     }    
}

// Print the return value of the system call
VOID SysAfter(ADDRINT ip, ADDRINT eax)
{   syscall_encountered = 0; 

    fprintf(trace,"  return value: 0x%lx", (unsigned long)eax);
    fflush(trace);
}

// Pin calls this function every time a new basic block is encountered.
// It inserts a call to for every system call encountered
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {		
	for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins=INS_Next(ins))
	{	
		if (syscall_encountered == 1)
		{	INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(SysAfter),
                       			IARG_INST_PTR,
					IARG_REG_VALUE, REG_EAX,
                        		IARG_END);
			syscall_encountered = 0;
		}
	
		if (INS_IsSyscall(ins))
        	{	// Arguments and syscall number is only available before
        		INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SysBefore),
                       			IARG_INST_PTR, IARG_SYSCALL_NUMBER,
                       			IARG_SYSARG_VALUE, 0, IARG_SYSARG_VALUE, 1,
                       			IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3,
                       			IARG_SYSARG_VALUE, 4, IARG_SYSARG_VALUE, 5,
                       			IARG_END);
			syscall_encountered = 1;
		}

    	}
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    fprintf(trace,"#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    trace = fopen("btrace.out", "w");

    // Register Instruction to be called to instrument instructions.
    TRACE_AddInstrumentFunction(Trace, NULL);

    // Register Fini to be called when the application exits.
    PIN_AddFiniFunction(Fini, NULL);

    // Start the program, never returns
    PIN_StartProgram();

    return 1;
}

