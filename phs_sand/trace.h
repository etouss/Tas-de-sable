/* File: trace.h
 *
 * specific declarations for handling varying data size
 */

/* trace.c */
/*=========*/
extern const char * tracing_slices_header;
extern const char * tracing_words_header;
extern void putnchars (char c, unsigned int n);
extern void traceindentin();
extern void traceindentout();
extern void traceindentmess();

#define DOTRACEINW(...)		do{traceindentin();printf (" %s", __func__); printf(__VA_ARGS__);terpri();}while(0)
#define DOTRACEOUTW(...) 	do{traceindentout();printf(" %s <- ",__func__);printf(__VA_ARGS__);terpri();}while(0)
#define DOTRACEOUT 		do{traceindentout();printf(" %s",__func__);terpri();}while(0)
#define DOTRACEMESS(...) 	{traceindentmess();printf(__VA_ARGS__);terpri();}
#define DOTRACESIGNEDMESS(...) 	{traceindentmess();printf("%s: ",__func__);printf(__VA_ARGS__);terpri();}
#define DOTRACEIN 		DOTRACEINW("")
#define DOTRACEWAITFORCHAR	do{traceindentmess();printf(" %s: Press ENTER to continue",__func__);getchar();terpri();}while(0)

#ifndef TRACE
#define TRACEIN
#define TRACEINW(...)
#define TRACEMESS(...)
#define TRACEOUT
#define TRACEOUTW(...)
#define TRACESIGNEDMESS(...)
#define TRACEWAITFORCHAR
#else
#define TRACEIN 		DOTRACEIN
#define TRACEINW(...)		DOTRACEINW(__VA_ARGS__)
#define TRACEMESS(...) 		DOTRACEMESS(__VA_ARGS__)
#define TRACEOUT 		DOTRACEOUT
#define TRACEOUTW(...)		DOTRACEOUTW(__VA_ARGS__)
#define TRACESIGNEDMESS(...) 	DOTRACESIGNEDMESS(__VA_ARGS__)
#define TRACEWAITFORCHAR	DOTRACEWAITFORCHAR
#endif /* TRACE */

/* DEEPTRACE is a TRACE for lower level functions that would generate a lot of output when traced.
   Hence only turned on with -DTRACE -DDEEPTRACE.
   HINT: WE SHOULD FIND A BETTER NAME */
#ifndef DEEPTRACE
#define DEEPTRACEINW(...)
#define DEEPTRACEOUTW(...)
#define DEEPTRACEOUT
#define DEEPTRACEMESS(...)
#define DEEPTRACEIN
#else
#define DEEPTRACEINW(...)  	TRACEINW(__VA_ARGS__)
#define DEEPTRACEOUTW(...)	TRACEOUTW(__VA_ARGS__)
#define DEEPTRACEOUT   		TRACEOUT
#define DEEPTRACEMESS(...)   	TRACEMESS(__VA_ARGS__)
#define DEEPTRACEIN   		TRACEIN
#endif /* DEEPTRACE */

