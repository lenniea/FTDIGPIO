#ifndef __TRACE_H__
#define __TRACE_H__

#if defined(_DEBUG)

#ifdef __cplusplus
    extern "C" {
#endif
void DebugTrace(const char* f, ...);
#ifdef __cplusplus
    }
#endif

    #define TRACE(x)              DebugTrace(x)
    #define TRACE1(x,a)           DebugTrace(x, a)
    #define TRACE2(x,a,b)         DebugTrace(x, a, b)
    #define TRACE3(x,a,b,c)       DebugTrace(x, a, b, c)
    #define TRACE4(x,a,b,c,d)     DebugTrace(x, a, b, c, d)
    #define TRACE5(x,a,b,c,d,e)   DebugTrace(x, a, b, c, d, e)
    #define TRACE6(x,a,b,c,d,e,f) DebugTrace(x, a, b, c, d, e, f)
#else
    #define TRACE(x)              do {} while (0)
    #define TRACE1(x,a)           do {} while (0)
    #define TRACE2(x,a,b)         do {} while (0)
    #define TRACE3(x,a,b,c)       do {} while (0)
    #define TRACE4(x,a,b,c,d)     do {} while (0)
    #define TRACE5(x,a,b,c,d,e)   do {} while (0)
    #define TRACE6(x,a,b,c,d,e,f) do {} while (0)
#endif

#endif /* __TRACE_H__ */
