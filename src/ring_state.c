/*
**  Copyright (c) 2013-2016 Mahmoud Fayed <msfclipper@yahoo.com> 
**  Include Files 
*/
#include "ring.h"
#ifdef _WIN32
/* Windows only */
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
/* Define Functions */
#if RING_TESTUNITS

static void ring_testallunits ( void ) ;
#endif
#if RING_TESTPERFORMANCE

static void ring_showtime ( void ) ;
#endif

void segfaultaction ( int sig ) ;

int ring_issourcefile ( const char *cStr ) ;
/* API Functions */

RING_API RingState * ring_state_new ( void )
{
	RingState *pRingState  ;
	pRingState = (RingState *) malloc(sizeof(RingState)) ;
	if ( pRingState == NULL ) {
		printf( RING_OOM ) ;
		exit(0);
	}
	pRingState->pRingFilesList = NULL ;
	pRingState->pRingFilesStack = NULL ;
	pRingState->pRingGenCode = NULL ;
	pRingState->pRingFunctionsMap = NULL ;
	pRingState->pRingClassesMap = NULL ;
	pRingState->pRingPackagesMap = NULL ;
	pRingState->pRingCFunctions = NULL ;
	pRingState->nISCGI = 0 ;
	pRingState->nRun = 1 ;
	pRingState->nPrintIC = 0 ;
	pRingState->nPrintICFinal = 0 ;
	pRingState->nPrintTokens = 0 ;
	pRingState->nPrintRules = 0 ;
	pRingState->nPrintInstruction = 0 ;
	pRingState->argc = 0 ;
	pRingState->argv = NULL ;
	pRingState->pVM = NULL ;
	pRingState->lStartup = 0 ;
	return pRingState ;
}

RING_API RingState * ring_state_delete ( RingState *pRingState )
{
	if ( pRingState->pRingFilesList != NULL ) {
		pRingState->pRingFilesList = ring_list_delete(pRingState->pRingFilesList);
		pRingState->pRingFilesStack = ring_list_delete(pRingState->pRingFilesStack);
	}
	if ( pRingState->pRingGenCode   != NULL ) {
		pRingState->pRingGenCode = ring_list_delete(pRingState->pRingGenCode);
		pRingState->pRingFunctionsMap = ring_list_delete(pRingState->pRingFunctionsMap);
		pRingState->pRingClassesMap = ring_list_delete(pRingState->pRingClassesMap);
		pRingState->pRingPackagesMap = ring_list_delete(pRingState->pRingPackagesMap);
		if ( pRingState->pRingCFunctions != NULL ) {
			/* We check because the execution may end by the compiler error */
			pRingState->pRingCFunctions = ring_list_delete(pRingState->pRingCFunctions);
		}
	}
	if ( pRingState->pVM != NULL ) {
		ring_vm_delete(pRingState->pVM);
	}
	free( pRingState ) ;
	return NULL ;
}

void ring_state_cgiheader ( RingState *pRingState )
{
	if ( pRingState->nISCGI == 1 ) {
		printf( "Content-Type: text/plain \n\n" ) ;
	}
}

RING_API void ring_print_line ( void )
{
	puts("==================================================================");
}

RING_API RingState * ring_state_init ( void )
{
	RingState *pRingState  ;
	pRingState = ring_state_new();
	ring_vm_init(pRingState);
	return pRingState ;
}

RING_API void ring_state_runcode ( RingState *pRingState,const char *cStr )
{
	ring_vm_runcode(pRingState->pVM,cStr);
}

RING_API List * ring_state_findvar ( RingState *pRingState,const char *cStr )
{
	VM *pVM  ;
	List *pList  ;
	pVM = pRingState->pVM ;
	pList = NULL ;
	if ( ring_vm_findvar(pVM,cStr) ) {
		pList = (List *) RING_VM_STACK_READP ;
		RING_VM_STACK_POP ;
	}
	return pList ;
}

RING_API List * ring_state_newvar ( RingState *pRingState,const char *cStr )
{
	VM *pVM  ;
	List *pList  ;
	pVM = pRingState->pVM ;
	if ( ring_vm_findvar(pVM,cStr) == 0 ) {
		ring_vm_newvar(pVM,cStr);
	}
	pList = (List *) RING_VM_STACK_READP ;
	RING_VM_STACK_POP ;
	return pList ;
}

RING_API void ring_state_main ( int argc, char *argv[] )
{
	int x,nCGI,nRun,nPrintIC,nPrintICFinal,nTokens,nRules,nIns,nPerformance,nSRC  ;
	const char *cStr  ;
	/* Init Values */
	nCGI = 0 ;
	nRun = 1 ;
	nPrintIC = 0 ;
	nPrintICFinal = 0 ;
	nTokens = 0 ;
	nRules = 0 ;
	nIns = 0 ;
	nPerformance = 0 ;
	cStr = NULL ;
	nSRC = 0 ;
	signal(SIGSEGV,segfaultaction);
	#if RING_TESTUNITS
	ring_testallunits();
	#endif
	if ( argc > 1 ) {
		for ( x = 1 ; x < argc ; x++ ) {
			if ( strcmp(argv[x],"-cgi") == 0 ) {
				nCGI = 1 ;
			}
			else if ( strcmp(argv[x],"-tokens") == 0 ) {
				nTokens = 1 ;
			}
			else if ( strcmp(argv[x],"-rules") == 0 ) {
				nRules = 1 ;
			}
			else if ( strcmp(argv[x],"-ic") == 0 ) {
				nPrintIC = 1 ;
			}
			else if ( strcmp(argv[x],"-norun") == 0 ) {
				nRun = 0 ;
			}
			else if ( strcmp(argv[x],"-icfinal") == 0 ) {
				nPrintICFinal = 1 ;
			}
			else if ( strcmp(argv[x],"-ins") == 0 ) {
				nIns = 1 ;
			}
			else if ( strcmp(argv[x],"-performance") == 0 ) {
				nPerformance = 1 ;
			}
			else if ( ring_issourcefile(argv[x]) && nSRC == 0 ) {
				cStr = argv[x] ;
				nSRC = 1 ;
			}
		}
	}
	#if RING_TESTPERFORMANCE
	if ( nPerformance ) {
		ring_showtime();
	}
	#endif
	srand(time(NULL));
	/* Check Startup ring.ring */
	if ( ring_fexists("ring.ring") && argc == 1 ) {
		ring_execute("ring.ring",nCGI,nRun,nPrintIC,nPrintICFinal,nTokens,nRules,nIns,argc,argv);
		exit(0);
	}
	/* Print Version */
	if ( (argc == 1) || (cStr == NULL) ) {
		ring_print_line();
		puts("Ring version 1.0 \n2013-2016, Mahmoud Fayed <msfclipper@yahoo.com> ");
		puts("Usage : ring filename.ring [Options]");
		ring_print_line();
		/* Options */
		puts("-tokens   :  Print a list of tokens in the source code file");
		puts("-rules    :  Print grammar rules applied on the tokens");
		puts("-ic       :  Print the intermediate byte code (before execution)");
		puts("-icfinal  :  Print the final byte code (after execution)");
		puts("-cgi      :  Print http response header before error messages");
		puts("-norun    :  Don't run the program after compiling");
		puts("-ins      :  Print instruction operation code before execution");
		puts("-clock    :  Print clock before and after program execution");
		ring_print_line();
		exit(0);
	}
	ring_execute(cStr,nCGI,nRun,nPrintIC,nPrintICFinal,nTokens,nRules,nIns,argc,argv);
	#if RING_TESTPERFORMANCE
	if ( nPerformance ) {
		ring_showtime();
	}
	#endif
}

RING_API void ring_state_runfile ( RingState *pRingState,const char *cFileName )
{
	ring_scanner_readfile(cFileName,pRingState);
}
#if RING_TESTUNITS

static void ring_testallunits ( void )
{
	/* Test */
	ring_hashtable_test();
	printf( "end of test \n  " ) ;
	getchar();
}
#endif
#if RING_TESTPERFORMANCE

static void ring_showtime ( void )
{
	time_t timer  ;
	char buffer[50]  ;
	struct tm*tm_info  ;
	clock_t myclock  ;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer,50,"Date  : %Y/%m/%d Time : %H:%M:%S", tm_info);
	printf( "\n" ) ;
	ring_print_line();
	puts(buffer);
	myclock = clock();
	printf( "Clock : %ld \n", myclock ) ;
	ring_print_line();
}
#endif

void segfaultaction ( int sig )
{
	printf( "Content-Type: text/plain\n\n" ) ;
	printf( "Ring Unexpected Error - Caught segfault : %d ",sig ) ;
	exit(0);
}

int ring_issourcefile ( const char *cStr )
{
	int x  ;
	x = strlen(cStr) - 1 ;
	if ( x > 5 ) {
		if ( tolower(cStr[x]) == 'g' && tolower(cStr[x-1]) == 'n' && tolower(cStr[x-2]) == 'i' && tolower(cStr[x-3]) == 'r' && cStr[x-4] == '.' ) {
			return 1 ;
		}
	}
	return 0 ;
}
/* General Functions */

int ring_fexists ( const char *cFileName )
{
	FILE *fp  ;
	fp = fopen(cFileName , "r" );
	if ( fp ) {
		fclose( fp ) ;
		return 1 ;
	}
	return 0 ;
}

int ring_currentdir ( char *cDirPath )
{
	int nSize  ;
	nSize = 200 ;
	if ( !GetCurrentDir(cDirPath, nSize) ) {
		return errno ;
	}
	cDirPath[nSize-1] = '\0' ;
	return 0 ;
}

int ring_exefilename ( char *cDirPath )
{
	int nSize  ;
	nSize = 200 ;
	#ifdef _WIN32
	/* Windows only */
	GetModuleFileName(NULL,cDirPath,nSize);
	#else
	#ifdef __linux__
	readlink("/proc/self/exe",cDirPath,nSize);
	#endif
	#endif
	return 0 ;
}
