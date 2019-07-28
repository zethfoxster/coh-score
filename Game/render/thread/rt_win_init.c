#include "rt_win_init.h"
#include "ogl.h"
#include "timing.h"
#include "fpmacros.h"
#include "file.h"
#include "gfxSettings.h"
#include "win_init.h"
#include "assert.h"
#include "mathutil.h"
#include "NvPanelApi.h"
#include "renderUtil.h"
#define RT_PRIVATE
#include "rt_pbuffer.h"
#include "rt_cgfx.h"
#include "rt_state.h"
#include "rt_graphfps.h"

static HGLRC		glRC = 0;
static HDC			hDC = 0;

static GLuint sFenceIDs[MAX_FRAMES] = {0};
static GLuint sQueryIDs[MAX_FRAMES] = {0};

void wglMakeCurrentSafeDirect( HDC hDC, HGLRC glRC, F32 timeout)
{
	int num_failures=0;
	int timer=0;
	while (!wglMakeCurrent( hDC, glRC )) {
		num_failures++;
		if (!timer)
			timer = timerAlloc();
		if (timerElapsed(timer)>timeout) {
			FatalErrorf("wglMakeCurrent failed for %f seconds", timeout);
		}
		Sleep(1);
	}
	if (timer)
		timerFree(timer);

	OnGLContextChange();
}

void makeCurrentCBDirect()
{
	wglMakeCurrentSafeDirect(hDC, glRC, 15);
}
static int setupPFD()
{
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
			1,                              // version number
			PFD_DRAW_TO_WINDOW              // support window
			|  PFD_SUPPORT_OPENGL           // support OpenGL
			|  PFD_DOUBLEBUFFER             // double buffered
			|  PFD_SWAP_EXCHANGE ,          // SLI friendly
			PFD_TYPE_RGBA,                  // RGBA type
			24,                             // 24-bit color depth (excludes alpha bitplanes)
			0, 0, 0, 0, 0, 0,               // color bits ignored
			0,                              // no alpha buffer
			0,                              // shift bit ignored
			0,                              // no accumulation buffer
			0, 0, 0, 0,                     // accum bits ignored
			24,                             // 24-bit z-buffer      
			8,                              // with 8-bit stencil buffer
			0,                              // no auxiliary buffer
			PFD_MAIN_PLANE,                 // main layer
			0,                              // reserved
			0, 0, 0                         // layer masks ignored
	};


	int  pixelFormat;

	pixelFormat = ChoosePixelFormat(hDC, &pfd);
	SET_FP_CONTROL_WORD_DEFAULT; // nVidia driver changes the FP consistency on load

	if ( !pixelFormat ) {
		return 0;
	}

	if ( !SetPixelFormat(hDC, pixelFormat, &pfd) ) {
		return 0;
	}

	return 1;
}

static int createContext(int major, int minor, int forward, int debug)
{
	// Delete the old context
	if (glRC) {
		wglDeleteContext(glRC);
		glRC = 0;
	}

	if(!(major || minor || forward || debug)) {
		glRC = wglCreateContext( hDC );
	} else if(WGLEW_ARB_create_context) {
		int flags = (forward?WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB:0) | (debug?WGL_CONTEXT_DEBUG_BIT_ARB:0);
		int attribList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
			WGL_CONTEXT_FLAGS_ARB, flags,
			0, 0,
		};

		glRC = wglCreateContextAttribsARB( hDC, 0, attribList );
	}

	return glRC != 0;
}

static int setupContext()
{
	if ( !wglMakeCurrent( hDC, glRC ) ) {
		wglDeleteContext( glRC );
		glRC = 0;
		return 0;
	}

	return glewInit() == GL_NO_ERROR;
}

#if defined(_FINAL)
	#define FORCE_DEBUG 0
	#define FORCE_NOT_DEBUG 1
#elif defined(_FULLDEBUG)
	#define FORCE_DEBUG 1
	#define FORCE_NOT_DEBUG 0
#else
	#define FORCE_DEBUG 0
	#define FORCE_NOT_DEBUG 0 
#endif

static int setupGL(int reinit)
{
	int forward = 0;
	int debug = (isDevelopmentMode() || FORCE_DEBUG) && (!FORCE_NOT_DEBUG);

	if(!setupPFD()) {
		return 0;
	}

	// initial GLEW setup
	if(!reinit) {
		if(!createContext(0,0,0,0)) {
			return 0;
		}
		if(!setupContext()) {
			return 0;
		}
	}

	// try for a new GL context
	if(createContext(3, 0, forward, debug)) {
		return setupContext();
	}

	// fall back to the old context
	assert(!glRC);
	if(!createContext(0,0,0,0)) {
		return 0;
	}

	return setupContext();
}

void nvidiaBeforeSetupGL()
{
	BOOL b=0;
	DWORD dw=0;
	fNvCplGetDataInt NvCplGetDataInt;
	fNvCplIsExternalPowerConnectorAttached NvCplIsExternalPowerConnectorAttached;
	fNvCplSetDataInt NvCplSetDataInt;
	HINSTANCE hLib = LoadLibrary("NVCPL.dll");

	if (!hLib)
		return;

	NvCplGetDataInt = (fNvCplGetDataInt)GetProcAddress(hLib, "NvCplGetDataInt");
	NvCplIsExternalPowerConnectorAttached = (fNvCplIsExternalPowerConnectorAttached)GetProcAddress(hLib, "NvCplIsExternalPowerConnectorAttached");
	NvCplSetDataInt = (fNvCplSetDataInt)GetProcAddress(hLib, "NvCplSetDataInt");

	if (NvCplIsExternalPowerConnectorAttached && NvCplIsExternalPowerConnectorAttached(&b)) {
		if (!b) {
			Errorf("You don't have your power cable attached to your video card, dummy!");
		}
	}
	// This doesn't work:
	//if (NvCplGetDataInt && NvCplSetDataInt && NvCplGetDataInt(NVCPL_API_CURRENT_AA_VALUE, &dw))
	//{
	//	b = NvCplSetDataInt(NVCPL_API_CURRENT_AA_VALUE, NVCPL_API_AA_METHOD_2X);
	//	NvCplGetDataInt(NVCPL_API_CURRENT_AA_VALUE, &dw);
	//}
	FreeLibrary(hLib);
}

static void windowPostInit(void)
{
	const char* s;

	// Log and record some fundamental GL context strings to the console 
	// and to the caps to aid in resolving configuration problems.
	// These are also recorded in the database with the users configuration information
	s = glGetString(GL_VENDOR);
	printf("OpenGL vendor string: %s\n", s);
	strncpy_s(rdr_caps.gl_vendor, sizeof(rdr_caps.gl_vendor), s, _TRUNCATE );

	s = glGetString(GL_RENDERER);
	printf("OpenGL renderer string: %s\n", s);
	strncpy_s(rdr_caps.gl_renderer, sizeof(rdr_caps.gl_renderer), s, _TRUNCATE );

	s = glGetString(GL_VERSION);
	printf("OpenGL version string: %s\n", s);
	strncpy_s(rdr_caps.gl_version, sizeof(rdr_caps.gl_version), s, _TRUNCATE );

	// create ids for use by graphfps
	if(GLEW_NV_fence) {
		int i;
		glGenFencesNV(MAX_FRAMES, sFenceIDs); CHECKGL;
		for(i=0; i<MAX_FRAMES; i++) {
			glSetFenceNV(sFenceIDs[i], GL_ALL_COMPLETED_NV); CHECKGL;
		}
	}

	// @todo this should be an OpenGL 1.5 core feature so we should
	// be able to test: GLEW_ARB_occlusion_query || GLEW_VERSION_1_5
	// However we have found some drivers for legacy cards that claim
	// to be 1.5 compliant however do not support occlusion.
	// e.g. nVidia 71.89 drivers released 4/14/2005 with a GeForce 2 GTS
	if(GLEW_ARB_occlusion_query /*|| GLEW_VERSION_1_5*/) {
		int i;
		glGenQueriesARB(MAX_FRAMES, sQueryIDs); CHECKGL;
		for(i=0; i<MAX_FRAMES; i++) {
			glBeginQueryARB(GL_SAMPLES_PASSED_ARB, sQueryIDs[i]); CHECKGL;
			glEndQueryARB(GL_SAMPLES_PASSED_ARB); CHECKGL;
		}
	}
}

void windowInitDisplayContextsDirect()
{
	if (!hDC)
	{
		if (systemSpecs.videoCardVendorID == VENDOR_NV)
			nvidiaBeforeSetupGL();
		hDC  = GetDC(hwnd);
		if ( !setupGL(false) ) {
			MessageBox( 0, "Failed to Create OpenGL Rendering Context.",
				"", MB_ICONERROR );
			gfxResetGfxSettings();
			DestroyWindow( hwnd );
			windowExit(0);
		}
		INEXACT_CHECKGL;

		windowPostInit();
	}
	rdrPreserveGammaRampDirect();
}

void windowDestroyDisplayContextsDirect()
{
	WCW_Shutdown();

	rt_cgfxDeInit();

	if (glRC)
	{
		if (rdr_caps.chip) {
			winMakeCurrentDirect();
			pbufDeleteAllDirect();
		}
		wglMakeCurrent(0,0);
		wglDeleteContext(glRC);
		glRC = 0;
	}
	if( hDC )
		ReleaseDC( hwnd, hDC );
	hDC = 0;
}

void windowReleaseDisplayContextsDirect(void)
{
	winMakeCurrentDirect();
	wglMakeCurrentSafeDirect(NULL, NULL, 15);
	if( hDC )
		ReleaseDC( hwnd, hDC );
	hDC = 0;
}

void windowReInitDisplayContextsDirect(void)
{
	if (!hDC)
	{
		hDC  = GetDC(hwnd);
		if ( !setupGL(true) ) {
			MessageBox( 0, "Failed to Create OpenGL Rendering Context.",
				"", MB_ICONERROR );
			gfxResetGfxSettings();
			DestroyWindow( hwnd );
			windowExit(0);
		}
		INEXACT_CHECKGL;

		windowPostInit();
	}
}

extern volatile int frames_buffered;
extern CRITICAL_SECTION frame_check;
extern HANDLE frame_done_signal;

void	windowUpdateDirect()
{
	int framesInProgress = 0;
	U32 curSLI = (rdr_frame_state.curFrame % MAX_FRAMES);
	U8 sliLimit = rdr_frame_state.sliLimit;

	rdrBeginMarker(__FUNCTION__);

	if (isDevelopmentMode())
	{
		HGLRC curr_glc = wglGetCurrentContext();
		assert(curr_glc == glRC);
	}

	#ifdef USE_NVPERFKIT
		if (isDevelopmentMode() && rdr_caps.chip & NV2X)
			NVPerfHeartbeatOGL();
	#endif

	rdrCpuFrameDone();

	rdrBeginMarker("WAIT");

	if(game_state.usenvfence) {
		glSetFenceNV(sFenceIDs[curSLI], GL_ALL_COMPLETED_NV); CHECKGL;
	} else if(rdr_caps.supports_occlusion_query) {
		glBeginQueryARB(GL_SAMPLES_PASSED_ARB, sQueryIDs[curSLI]); CHECKGL;
		glEndQueryARB(GL_SAMPLES_PASSED_ARB); CHECKGL;
	}

	// wait on fence to ensure the wanted frame is finished.
	if(sliLimit) {
		unsigned int u;

		// Force a specific frame to finish
		int index = (curSLI - sliLimit) % MAX_FRAMES;
		if(game_state.usenvfence) {
			glFinishFenceNV(sFenceIDs[index]); CHECKGL;
		} else if(rdr_caps.supports_occlusion_query) {
			GLuint sampleCount;
			glGetQueryObjectuivARB(sQueryIDs[index], GL_QUERY_RESULT_ARB, &sampleCount); CHECKGL;
		}

		// Count unfinished frames
		for(u=0; u < sliLimit; u++) {
			GLuint queryFinished;
			index = (index+1) % MAX_FRAMES;
			if(game_state.usenvfence) {
				queryFinished = glTestFenceNV(sFenceIDs[index]); CHECKGL;
			} else if(rdr_caps.supports_occlusion_query) {
				glGetQueryObjectuivARB(sQueryIDs[index], GL_QUERY_RESULT_AVAILABLE_ARB, &queryFinished); CHECKGL;
			} else {
				break;
			}
			if(!queryFinished) {
				framesInProgress += 1;
			}
		}
	}

	rdrEndMarker();
	
	rdrGpuFrameDone();

	rdrDrawFrameHistory();

	// Clear any OpenGL errors
	gldGetError();

	rdrBeginMarker("SWAP");
	SwapBuffers(hDC);
	rdrEndMarker();

	rdrEndFrameHistory(framesInProgress);

	if(GLEW_GREMEDY_frame_terminator) {
		glFrameTerminatorGREMEDY(); CHECKGL;
	}

	rdrStartFrameHistory();

	InterlockedDecrement(&frames_buffered);
	SetEvent(frame_done_signal);

	rdrEndMarker();
}


//////////////////////////////////////////////////////////////////////////
// Gamma ramp
//////////////////////////////////////////////////////////////////////////

//This is the gamma ramp before the game started.  Restore it when the game finishes.
static WORD preservedRamp[256*3];
static int damageBuffer1 = 0;
static int gammaRampHasBeenPreserved = 0;
static int damageBuffer2 = 0;
static F32 currentGamma = 0;

void rdrNeedGammaResetDirect(void)
{
	currentGamma = -100;
}

void rdrPreserveGammaRampDirect()
{
	if( hDC )
	{
		if( GetDeviceGammaRamp( hDC, preservedRamp ) )
			gammaRampHasBeenPreserved = 1;
	}
}

void rdrRestoreGammaRampDirect()
{
	if( hDC )
	{
		if( gammaRampHasBeenPreserved )
			SetDeviceGammaRamp( hDC, preservedRamp );
	}
}

void rdrSetGammaDirect( float Gamma ) 
{
	if( Gamma > 0.1 && currentGamma != Gamma )  
	{
		WORD ramp[256*3];
		int i;
		F32 adjustedGamma;
		F32 rampVal, rampIdx;//, scale;
		F32 scaledGamma = 0;
		static int lastTweak=0;

		adjustedGamma =  MINMAX( ( Gamma ), 0.3, 3.0 ); 

		for( i = 0 ; i < 256 ; i++ )      
		{	
			//Convert ramp idx to 0.0 to 1.0; 
			rampIdx = (i+1) / 256.0; 

			//Do crazy scaling to get something that looks nice
			//scale = adjustedGamma - 1.0;    
			//scale = scale * (1.0 - rampIdx*rampIdx); 
			//scaledGamma = scale + 1.0;     

			scaledGamma = adjustedGamma;  

			//Apply Gamma Scale   
			rampVal = pow( rampIdx, scaledGamma ); 

			//Scale to fit WORD
			rampVal = rampVal * 65535 + 0.5;
			rampVal = MINMAX( rampVal, 0, 65535 );

			//Apply to R, G, and B ramp
			ramp[i+0] = ramp[i+256] = ramp[i+512] = (WORD)rampVal;
		}

		currentGamma = Gamma;

		// Tweak gamma to get around buggy ATI statemanager
		lastTweak^=1;
		ramp[1]+=lastTweak;

		SetDeviceGammaRamp( hDC, ramp );
	}
}

void rdrSetVSyncDirect(int enable)
{
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(enable?1:0);
}

