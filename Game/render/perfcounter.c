#include "perfcounter.h"
#include <GL/glew.h>
#include <GL/wglew.h>

static bool s_bRunning = false;

// function pointers to execution code, based on chipset codepath
int (* s_fpShutdown )() = NULL;  
int (* s_fpStart )() = NULL;  
int (* s_fpStop )() = NULL;  
void (* s_fpSample )() = NULL;  
void (* s_fpDisplay )(int, int, int) = NULL;  


#if defined(WIN32)

#include "utils.h"
#include "SuperAssert.h"
#include "entDebug.h"

enum { MAX_COUNTERS = 128 };
static int s_maxValidIdx = 0;
static SPerfCounter s_counters[MAX_COUNTERS];


//////////////////////////////////////////////////////////////////////////
// NVidia routines for performance counters
//////////////////////////////////////////////////////////////////////////

#include "NVPerfSDK.h"

int nvEnumFuncCountersCB(UINT unCounterIndex, char *pcCounterName)
{
    char zString[200] = "\0";
    unsigned int unLen = 200;
    U64 attrib_type  = -1;
    U64 attrib_value = NVPM_CV_RAW;

    if(s_maxValidIdx >= MAX_COUNTERS)
    {
        devassertmsg(false, "Need to up MAX_COUNTERS value for performance counters");
        return NVPM_FAILURE;
    }

    //set default values
    s_counters[s_maxValidIdx].enable = false;   // true;
    s_counters[s_maxValidIdx].uIndex = unCounterIndex;
    s_counters[s_maxValidIdx].nType = NVPM_CV_RAW;
    s_counters[s_maxValidIdx].pName = pcCounterName;

    if(NVPMGetCounterDescription(unCounterIndex, zString, &unLen) != NVPM_OK) 
    {
        OutputDebugStringf("Error retrieving type attribute for counter [%d] <%s>", unCounterIndex, pcCounterName);
    }

    if(NVPMGetCounterAttribute(unCounterIndex, NVPMA_COUNTER_TYPE, &attrib_type) != NVPM_OK)
    {
        OutputDebugStringf("Error retrieving type attribute for counter [%d] <%s>", unCounterIndex, pcCounterName);
    }

    if(NVPMGetCounterAttribute(unCounterIndex, NVPMA_COUNTER_VALUE, &attrib_value) != NVPM_OK)
    {
        OutputDebugStringf("Error retrieving counter attribute for counter [%d] <%s>", unCounterIndex, pcCounterName);
    }

        
    switch(attrib_type)
    {
        case NVPM_CT_GPU :    OutputDebugStringf("GPU Counter %d [%s] : %s [%s]\n", unCounterIndex, pcCounterName, zString, (attrib_value == NVPM_CV_RAW) ? "RAW" : "%");
            s_counters[s_maxValidIdx].nType = attrib_value;
            s_maxValidIdx++;
            break;
        case NVPM_CT_OGL :    OutputDebugStringf("OGL Counter %d [%s] : %s [%s]\n", unCounterIndex, pcCounterName, zString, (attrib_value == NVPM_CV_RAW) ? "RAW" : "%"); 
            s_counters[s_maxValidIdx].nType = attrib_value;
            s_maxValidIdx++;
            break;
        case NVPM_CT_D3D :    OutputDebugStringf("skipping D3D Counter %d [%s] : %s\n", unCounterIndex, pcCounterName, zString);    break;
        case NVPM_CT_SIMEXP : OutputDebugStringf("skipping SIMEXP Counter %d [%s] : %s\n", unCounterIndex, pcCounterName, zString); break;
    }


    return(NVPM_OK);
}

static int perfCounterShutdownNV(void)
{
    return NVPMShutdown();
}

static int perfCounterStartNV(void)
{
    int i;

    //add the counters that user wants enabled
    for(i = 0; i < s_maxValidIdx; i++)
    {
        if(s_counters[i].enable)
        {
            NVPMAddCounter(s_counters[i].uIndex);
        }
    }

    s_bRunning = true;
    return s_bRunning;
}

static int perfCounterStopNV(void)
{
    int result = NVPMRemoveAllCounters();
    if(result != NVPM_OK)
        OutputDebugStringf("error removing all performance counters, code = %d \n", result);

    s_bRunning = false;
    return 1;
}

static void perfCounterSampleNV(void)
{
    //to only be done once per frame!
    UINT uCount;
    int result = NVPMSample(NULL, &uCount);
    if(result != NVPM_OK)
        OutputDebugStringf("NVPMSample returned error code %d\n", result);
}

static void perfCounterDisplayNV(int offset_x, int offset_y, int line_height)
{
    char buffer[256] = "\0";
    int i;
    int count = 0;

    //if just started this pass, don't display
    if(perfCounterStart())
    {
        return;
    }

    for(i = 0; i < s_maxValidIdx; i++)
    {
        if(s_counters[i].enable && (i % 3 < 2))
        {
            const int OFFSET_VALUE = 60;
            const int OFFSET_LABEL = 140;
            U64 events = 0;
            U64 cycles = 0;
            int result = NVPMGetCounterValue(s_counters[i].uIndex, 0, &events, &cycles);  

            if(result != NVPM_OK)
                OutputDebugStringf("error getting performance counter #%d[idx=%d], code = %d \n", i, s_counters[i].uIndex, result);

            //
            // output #s with name label
            //
            count++;

            if(s_counters[i].nType == NVPM_CV_RAW)
            {
                printDebugString(getCommaSeparatedInt(events), offset_x + OFFSET_VALUE, offset_y, 1.0f, line_height, (count & 0x01) ? 4 : 6, 0, 255, NULL);
            }
            else
            {
                snprintf(buffer, 256, "%5.2f%%", (cycles) ? (100.0f * (float) events / (float) cycles) : 0.0f);
                printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, (count & 0x01) ? 4 : 6, 0, 255, NULL);
                printDebugString(getCommaSeparatedInt(events), offset_x + OFFSET_VALUE, offset_y, 1.0f, line_height, (count & 0x01) ? 4 : 6, 0, 255, NULL);
            }
            printDebugString(s_counters[i].pName, offset_x + OFFSET_LABEL, offset_y, 1.0f, line_height, (count & 0x01) ? 0 : 7, 0, 255, NULL);

            offset_y += line_height;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// AMD routines for performance counters
//////////////////////////////////////////////////////////////////////////

static int perfCounterShutdownAMD(void)
{
    OutputDebugStringf("shutdown AMD performance counters through GL_AMD_performance_monitor extension\n");
    return 0;
}

static int perfCounterStartAMD(void)
{
    OutputDebugStringf("AMD performance monitor start\n");

    s_bRunning = true;
    return s_bRunning;
}


static int perfCounterStopAMD(void)
{
    OutputDebugStringf("AMD performance monitor stop\n");

    s_bRunning = false;
    return 1;
}

static void perfCounterDisplayAMD(int offset_x, int offset_y, int line_height)
{
    OutputDebugStringf("display AMD performance counters through GL_AMD_performance_monitor extension\n");
}

static void perfCounterSampleAMD(void)
{
    OutputDebugStringf("sample AMD performance counters through GL_AMD_performance_monitor extension\n");
}


void perfCounterInit(int chipset_nv, int chipset_ati)
{
    if(false)   //chipset_nv)
    {
        int result = NVPMInit();
        if(result == NVPM_OK)
        {
            //setup function pointers
            s_fpShutdown = perfCounterShutdownNV;  
            s_fpStart = perfCounterStartNV;  
            s_fpStop = perfCounterStopNV;  
            s_fpSample = perfCounterSampleNV;  
            s_fpDisplay = perfCounterDisplayNV;  

            NVPMEnumCounters(nvEnumFuncCountersCB);
        }
        else
        {
            unsigned error = 0;
            NVPMGetExtendedError(&error);
            OutputDebugStringf("ERROR: NVPMGetExtendedError() = %u, return code = %d", error, result);
        }
    }
    if(true)    //chipset_ati)
    {
        int result = 0;
        //TODO: work out if extension exists without wrangler
        //if (glewIsSupported("GL_AMD_performance_monitor"))
        //  result = 1;

        if(result)
        {
            //setup function pointers
            s_fpShutdown = perfCounterShutdownAMD;  
            s_fpStart = perfCounterStartAMD;  
            s_fpStop = perfCounterStopAMD;  
            s_fpSample = perfCounterSampleAMD;  
            s_fpDisplay = perfCounterDisplayAMD;  

            OutputDebugStringf("init AMD performance counters through GL_AMD_performance_monitor extension\n");
        }
        else
            OutputDebugStringf("AMD performance counters work out if extension GL_AMD_performance_monitor exists\n");
    }
}


#else

void perfCounterInit(void)      {}
    
#endif  //WIN32

int perfCounterShutdown(void)
{
    int result = (s_fpShutdown) ? s_fpShutdown() : 0;

    s_fpShutdown = NULL;    
    s_fpStart = NULL;   
    s_fpStop = NULL;   
    s_fpSample = NULL;   
    s_fpDisplay = NULL;  

    return result;
}

int perfCounterStart(void)
{
    return (!s_bRunning && s_fpStart) ? s_fpStart() : 0;
}

int perfCounterStop(void)
{
    return (s_bRunning && s_fpStop) ? s_fpStop() : 0;
}

void perfCounterSample(void)    
{
    if(s_bRunning && s_fpSample) s_fpSample();
}

void perfCounterDisplay(int offset_x, int offset_y, int line_height)
{
    if(s_fpDisplay) s_fpDisplay(offset_x, offset_y, line_height);
}
