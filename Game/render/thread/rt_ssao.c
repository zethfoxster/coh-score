#define RT_ALLOW_BINDTEXTURE

#include "rt_ssao.h"
#include "rt_cgfx.h"
#include "rt_tune.h"
#include "rt_init.h"
#include "wcw_statemgmt.h"
#include "gfx.h"
#include "gfxSettings.h"
#include "mathutil.h"
#include "tiff.h"
#include "failtext.h"
#include <file.h>
#include "newfeature.h"

#define RT_PRIVATE
#include "rt_pbuffer.h"

#define kSSAO_EffectName "ssao"

// Minimum size depth buffer to use
#define MIN_DEPTH_PIXELS 200000

// Minimum size ssao buffer to use
#define MIN_SSAO_PIXELS 200000

// Minimum size blur buffer to use
#define MIN_BLUR_PIXELS 200000

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;

enum depthReduceEnum {
	depthPoint,
	depthAverage,
	depthMin,
	depthMax,
};

typedef struct ssaoGlobals {
	// quality settings
	int strength;
	int resolution;
	int blur;

	int ambientDownsample;
	int lastParamAmbientDownsample;

	int blurDownsample;
	int lastParamBlurDownsample;

	int depthDownsample;
	int lastParamDepthDownsample;

	bool irradiance;
	bool lastIrradiance;

	int sampleCount;
	int lastSampleCount;

	int depthReduce;

	// postWeight
	float postScale;
	float postPower;
	float irradianceScale;

	GLuint texTarget;
	GLuint lastTexTarget;

	// framebuffer render size
	GLsizei screenWidth;
	GLsizei screenHeight;

	// ssao buffer size
	GLsizei ambientWidth;
	GLsizei ambientHeight;

	// blur buffer size
	GLsizei blurWidth;
	GLsizei blurHeight;

	// depth buffer size
	GLsizei depthWidth;
	GLsizei depthHeight;

	// framebuffer, renderbuffer and texture objects
	int numFbo;
	int curFbo;
	GLuint pushFbo;
	GLuint fbo0;
	GLuint blackTex;
	GLuint whiteTex;
	GLuint colorTex[MAX_SLI];
	GLuint depthTex[MAX_SLI];
	GLuint ambientDepth[MAX_SLI];
	GLuint ambientTex[MAX_SLI];
	GLuint blurDepth[MAX_SLI];
	GLuint blurTex[MAX_SLI];
	GLuint blur2Depth[MAX_SLI];
	GLuint blur2Tex[MAX_SLI];
	GLuint reducedColorTex[MAX_SLI];
	GLuint reducedDepthTex[MAX_SLI];

	// CgFX handles
	CGcontext cgContext;
	CGeffect cgEffect;
	
	CGtechnique color2d;
	CGtechnique depth2d;
	CGtechnique colorDepth2d;
	CGtechnique colorDepthAverage2d;
	CGtechnique colorDepthMin2d;
	CGtechnique colorDepthMax2d;
	CGtechnique colorFog2d;
	CGtechnique colorBlendFog2d;	
	CGtechnique blurFast2d;
	CGtechnique gaussian1d;
	CGtechnique gaussianDepth1d;
	CGtechnique bilateral1d;
	CGtechnique bilateralDepth1d;
	CGtechnique trilateral1d;
	CGtechnique ssaoLow;
	CGtechnique ssaoMedium;
	CGtechnique ssaoHigh;
	CGtechnique ssaoUltra;

	// Cached results from cgGetNamedEffectParameter()
	CGparameter modelViewProjParam;
	CGparameter screenstepParam;
	CGparameter depthTexParam;
	CGparameter depthTexNPOTParam;
	CGparameter colorTexParam;
	CGparameter colorTexNPOTParam;
	CGparameter color2TexParam;
	CGparameter color2TexNPOTParam;

	CGparameter reducedDepthTexParam;
	CGparameter reducedDepthTexNPOTaram;
	CGparameter reducedColorTexParam;
	CGparameter reducedColorTexNPOTaram;
	CGparameter ssaoSizeParam;
	CGparameter randomVectorSizeParam;
	CGparameter randomVectorsParam;
	CGparameter nearFarParam;
	CGparameter nearFarLimitParam;
	CGparameter pixelDepthSpreadParam;
	CGparameter preWeightParam;
	CGparameter postWeightParam;
	CGparameter sampleCountParam;
	CGparameter sampleVectorsParam;
	CGparameter sigmaParam;

	CGparameter newFeatureParam;
	CGparameter fogColorParam;
	CGparameter fogParamParam;
	CGparameter depthInvMatrixParam;
} ssaoGlobals;

static ssaoGlobals ssao;

// Deprecated below here
enum shaderOptimizationEnum {
	optPrecise = 0,
	optNone = 1,
	optFastMath = 2,
	optFastPrecision = 3,
};

const char * shaderOptimizationNames[] = {"Precise", "None", "Fast Math", "Fast Precision", NULL};
static int shaderOptimizationModes[] = {optPrecise, optNone, optFastMath, optFastPrecision};
static int shaderOptimization = optFastMath;

const char * ssaoEdgeNames[] = {"Clamp", "Clamp to Edge", "Mirror", "Clamp to Border", "Repeat", NULL};
static int ssaoEdgeModes[] = {GL_CLAMP, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_CLAMP_TO_BORDER, GL_REPEAT};
static int ssaoEdge = GL_CLAMP_TO_BORDER;

static float border = 0.0f;

enum drawEnum {
	drawOff,
	drawOn,
	drawVertical,
	drawHorizontal,
};

const char * drawNames[] = {"Off", "On", "Half Vertical", "Half Horizontal", NULL};
static int drawModes[] = {drawOff, drawOn, drawVertical, drawHorizontal};
static int draw = drawOn;

const char * drawDepthTestNames[] = {"Never", "Not Equal", "Less", "Less Equal", "Equal", "Greater Equal", "Greater", "Always", NULL};
static int drawDepthTestModes[] = {GL_NEVER, GL_NOTEQUAL, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_GEQUAL, GL_GREATER, GL_ALWAYS};
static int drawDepthTest = GL_GREATER;

static float drawDepth = -1.0f;

static bool enableBlend = true;
static bool enableFog = true;

static float sigma[4] = {2.0f, 1.0f, 0.0003f, 0.0f};

static float nearFarLimit[2] = {4.0f, 10.0f};
static float pixelDepthSpread[2] = {1.0f, 0.005f};
static float preWeight[4] = {0.05f, 0.5f, 1.0f, 0.0001f};

GLuint makeRenderbuffer(GLint internalFormat, GLsizei width, GLsizei height)
{
	GLuint pushRenderbuffer = 0;
	GLuint renderbuffer;
	GLenum error;

	glGenRenderbuffersEXT(1, &renderbuffer); CHECKGL;
	glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &pushRenderbuffer); CHECKGL;
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer); CHECKGL;

	gldGetError();
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, width, height);
	error = gldGetError();

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, pushRenderbuffer); CHECKGL;

	if(error) {
		glDeleteRenderbuffersEXT(1, &renderbuffer); CHECKGL;
		return 0;
	}

	return renderbuffer;
}

GLuint makeTex(GLuint target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * data)
{
	GLuint texture;
	GLenum error;

	glGenTextures(1, &texture); CHECKGL;
	WCW_BindTexture(target, 0, texture);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0); CHECKGL;

	gldGetError();
	glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, data);
	error = gldGetError();

	WCW_BindTexture(target, 0, 0);

	if(error) {
		glDeleteTextures(1, &texture); CHECKGL;
		return 0;
	}

	return texture;
}

void setTexMode(GLuint target, GLuint texture, GLint minFilter, GLint maxFilter, GLint wrapS, GLint wrapT)
{
	WCW_BindTexture(target, 0, texture);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter); CHECKGL;
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, maxFilter); CHECKGL;
	glTexParameterf(target, GL_TEXTURE_WRAP_S, wrapS); CHECKGL;
	glTexParameterf(target, GL_TEXTURE_WRAP_T, wrapT); CHECKGL;
	WCW_BindTexture(target, 0, 0);
}

#define checkFramebufferStatus() _checkFramebufferStatus(__FILE__, __LINE__, __FUNCTION__)
static GLenum _checkFramebufferStatus(const char * file, int line, const char * function)
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); CHECKGL;
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		const char * str = NULL;
		switch(status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: str = "incomplete attachment"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: str = "missing attachment"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: str = "incomplete dimensions"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: str = "incomplete formats"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: str = "incomplete draw buffer"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: str = "incomplete read buffer"; break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT: str = "unsupported"; break;
		}
		printf("glCheckFramebufferStatus = 0x%04x (%s) @ %s:%d:%s\n", status, str, file, line, function);
	}
	return status;
}

static GLenum setupFbo(GLuint colorTexTarget, GLuint colorTex, GLuint depthTexTarget, GLuint depthTex, GLuint depthRenderbuffer, GLsizei width, GLsizei height)
{
	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT); CHECKGL;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &ssao.pushFbo); CHECKGL;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ssao.fbo0); CHECKGL;
	if (depthTex) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, depthTexTarget, depthTex, 0); CHECKGL;
	} else {
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthRenderbuffer); CHECKGL;
	}
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, colorTexTarget, colorTex, 0); CHECKGL;

	glViewport(0, 0, width, height); CHECKGL;
	return checkFramebufferStatus();
}

static void restoreFbo()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ssao.pushFbo); CHECKGL;
	glPopAttrib(); CHECKGL;
	ssao.pushFbo = 0;
}

static void setGameAmbient(U8 strength, U8 resolution, U8 blur)
{
	// @todo make this more thread safe
	game_state.ambientStrength = strength;
	game_state.ambientResolution = resolution;
	game_state.ambientBlur = blur;
	if (strength)
		rdr_caps.features |= GFXF_AMBIENT;		
	else
		rdr_caps.features &= ~GFXF_AMBIENT;
}

static void updateAmbientFeatures()
{
	setGameAmbient(game_state.ambientStrength, game_state.ambientResolution, game_state.ambientBlur);
}

static void disableAmbient()
{
	game_state.ambientStrength = AMBIENT_OFF;
	updateAmbientFeatures();
	// @todo make this more thread safe
	rdr_caps.allowed_features &= ~GFXF_AMBIENT;
}

static void deleteFbos()
{
	ssao.screenWidth = ssao.screenHeight = 0;
	if (ssao.fbo0) {
		glDeleteFramebuffersEXT(1, &ssao.fbo0); CHECKGL;
		ssao.fbo0 = 0;
	}
	if (ssao.ambientDepth[0]) {
		glDeleteRenderbuffersEXT(ssao.numFbo, ssao.ambientDepth); CHECKGL;
		memset(ssao.ambientDepth, 0, sizeof(ssao.ambientDepth));
	}
	if (ssao.blurDepth[0]) {
		glDeleteRenderbuffersEXT(ssao.numFbo, ssao.blurDepth); CHECKGL;
		memset(ssao.blurDepth, 0, sizeof(ssao.blurDepth));
	}
	if (ssao.blackTex) {
		glDeleteTextures(1, &ssao.blackTex); CHECKGL;
		ssao.blackTex = 0;
	}
	if (ssao.whiteTex) {
		glDeleteTextures(1, &ssao.whiteTex); CHECKGL;
		ssao.whiteTex = 0;
	}
	if (ssao.colorTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.colorTex); CHECKGL;
		memset(ssao.colorTex, 0, sizeof(ssao.colorTex));
	}
	if (ssao.depthTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.depthTex); CHECKGL;
		memset(ssao.depthTex, 0, sizeof(ssao.depthTex));
	}
	if (ssao.reducedColorTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.reducedColorTex); CHECKGL;
		memset(ssao.reducedColorTex, 0, sizeof(ssao.reducedColorTex));
	}
	if (ssao.reducedDepthTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.reducedDepthTex); CHECKGL;
		memset(ssao.reducedDepthTex, 0, sizeof(ssao.reducedDepthTex));
	}
	if (ssao.ambientTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.ambientTex); CHECKGL;
		memset(ssao.ambientTex, 0, sizeof(ssao.ambientTex));
	}
	if (ssao.blurTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.blurTex); CHECKGL;
		memset(ssao.blurTex, 0, sizeof(ssao.blurTex));
	}
	if (ssao.blur2Tex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.blur2Tex); CHECKGL;
		memset(ssao.blur2Tex, 0, sizeof(ssao.blur2Tex));
	}
}

static bool initFbos()
{
	const uint32_t black = 0x00;
	const uint32_t white = 0xFF;

	// Color formats for ultra mode ssao
	const GLint colorFormats[] = {
		GL_RGB8,
		GL_RGBA8,
		GL_RGB5,
		GL_RGB5_A1,
		GL_RGB,
		GL_RGBA,
		GL_RGB4,
		GL_RGBA4,
	};
	const int numColorFormats = sizeof(colorFormats)/sizeof(colorFormats[0]);

	// Luminance formats for non ultra mode ssao
	const GLint luminanceFormats[] = {
		GL_LUMINANCE8,
		GL_INTENSITY8,
		GL_LUMINANCE12,
		GL_INTENSITY12,
		GL_LUMINANCE12_ALPHA4,
		GL_LUMINANCE16,
		GL_INTENSITY16,
		GL_LUMINANCE,
		GL_INTENSITY,
		GL_LUMINANCE4,
		GL_INTENSITY4,
	};
	const int numLuminanceFormats = sizeof(luminanceFormats)/sizeof(luminanceFormats[0]);

	// Prefer 24 bit depth, but fallback to 32 or 16
	GLint hqDepthFormats[] = {
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT32,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT16,
	};
	const int numHqDepthFormats = sizeof(hqDepthFormats)/sizeof(hqDepthFormats[0]);

	// Prefer the fastest depth format we can get
	GLint fastDepthFormats[] = {
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT32,
	};
	const int numFastDepthFormats = sizeof(fastDepthFormats)/sizeof(fastDepthFormats[0]);

	bool singleChannel = !ssao.irradiance;
	int pixels = ssao.screenWidth * ssao.screenHeight;

	int i, j, k;
	GLenum ret;
	GLenum depthInternalFormat;
	GLenum internalFormat;

	if (ssao.fbo0) {
		glDeleteFramebuffersEXT(1, &ssao.fbo0); CHECKGL;
	}
	glGenFramebuffersEXT(1, &ssao.fbo0); CHECKGL;

	// Allocate textures
	if (ssao.blackTex) {
		glDeleteTextures(1, &ssao.blackTex); CHECKGL;
	}
	ssao.blackTex = makeTex(ssao.texTarget, GL_LUMINANCE8, 1, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE, &black);
	if(!ssao.blackTex) return false;

	if (ssao.whiteTex) {
		glDeleteTextures(1, &ssao.whiteTex); CHECKGL;
	}
	ssao.whiteTex = makeTex(ssao.texTarget, GL_LUMINANCE8, 1, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE, &white);
	if(!ssao.whiteTex) return false;

	if (ssao.colorTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.colorTex); CHECKGL;
		memset(ssao.colorTex, 0, sizeof(ssao.colorTex));
	}
	for(k=0; k<ssao.numFbo; k++) {
		ssao.colorTex[k] = makeTex(ssao.texTarget, GL_RGBA8, ssao.screenWidth, ssao.screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		if(!ssao.colorTex[k]) break;
	}

	if (ssao.depthTex[0]) {
		glDeleteTextures(ssao.numFbo, ssao.depthTex); CHECKGL;
		memset(ssao.depthTex, 0, sizeof(ssao.depthTex));
	}
	for(k=0; k<ssao.numFbo; k++) {
		ssao.depthTex[k] = makeTex(ssao.texTarget, GL_DEPTH_COMPONENT24, ssao.screenWidth, ssao.screenHeight, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		if(!ssao.depthTex[k]) break;
	}

	// Downsample selection - limit downsampling if the screen resolution is too small
	if(ssao.ambientDownsample > 1 && pixels < MIN_SSAO_PIXELS) {
		ssao.ambientDownsample = 1;
	} else if(ssao.ambientDownsample > 2 && (pixels / 4) < MIN_SSAO_PIXELS) {
		ssao.ambientDownsample = 2;
	}
	ssao.ambientWidth = ssao.screenWidth / ssao.ambientDownsample;
	ssao.ambientHeight = ssao.screenHeight / ssao.ambientDownsample;

	if(ssao.blurDownsample > 1 && pixels < MIN_BLUR_PIXELS) {
		ssao.blurDownsample = 1;
	} else if(ssao.blurDownsample > 2 && (pixels / 4) < MIN_BLUR_PIXELS) {
		ssao.blurDownsample = 2;
	}
	ssao.blurWidth = ssao.screenWidth / ssao.blurDownsample;
	ssao.blurHeight = ssao.screenHeight / ssao.blurDownsample;

	if(ssao.depthDownsample && pixels < MIN_DEPTH_PIXELS) {
		ssao.depthDownsample = 1;
	} else if(ssao.depthDownsample > 2 && (pixels / 4) < MIN_DEPTH_PIXELS) {
		ssao.depthDownsample = 2;
	} else if(ssao.depthDownsample > 4 && (pixels / 16) < MIN_DEPTH_PIXELS) {
		ssao.depthDownsample = 4;
	}
	ssao.depthWidth = ssao.screenWidth / ssao.depthDownsample;
	ssao.depthHeight = ssao.screenHeight / ssao.depthDownsample;

	// Allocate ssao render targets
	ret = GL_NO_ERROR;
	while(1) {
		for(i=0; i<numFastDepthFormats; i++) {
			const int numSsaoFormats = singleChannel ? numLuminanceFormats : numColorFormats;
			const GLenum format = singleChannel ? GL_LUMINANCE : GL_RGBA;

			depthInternalFormat = fastDepthFormats[i];

			if (ssao.ambientDepth[0]) {
				glDeleteRenderbuffersEXT(ssao.numFbo, ssao.ambientDepth); CHECKGL;
				memset(ssao.ambientDepth, 0, sizeof(ssao.ambientDepth));
			}
			for(k=0; k<ssao.numFbo; k++) {
				ssao.ambientDepth[k] = makeRenderbuffer(depthInternalFormat, ssao.ambientWidth, ssao.ambientHeight);
				if (!ssao.ambientDepth[k]) break;
			}
			if(k != ssao.numFbo) continue;

			if (ssao.blurDepth[0]) {
				glDeleteRenderbuffersEXT(ssao.numFbo, ssao.blurDepth); CHECKGL;
				memset(ssao.blurDepth, 0, sizeof(ssao.blurDepth));
			}
			for(k=0; k<ssao.numFbo; k++) {
				ssao.blurDepth[k] = makeRenderbuffer(depthInternalFormat, ssao.blurWidth, ssao.blurHeight);
				if (!ssao.blurDepth[k]) break;
			}
			if(k != ssao.numFbo) continue;

			if (ssao.blur2Depth) {
				glDeleteRenderbuffersEXT(ssao.numFbo, ssao.blur2Depth); CHECKGL;
				memset(ssao.blur2Depth, 0, sizeof(ssao.blur2Depth));
			}
			for(k=0; k<ssao.numFbo; k++) {
				ssao.blur2Depth[k] = makeRenderbuffer(depthInternalFormat, ssao.blurWidth, ssao.ambientHeight);
				if (!ssao.blur2Depth[k]) break;
			}
			if(k != ssao.numFbo) continue;

			for(j=0; j<numSsaoFormats; j++) {
				internalFormat = singleChannel ? luminanceFormats[j] : colorFormats[j];

				if (ssao.ambientTex[0]) {
					glDeleteTextures(ssao.numFbo, ssao.ambientTex); CHECKGL;
					memset(ssao.ambientTex, 0, sizeof(ssao.ambientTex));
				}
				for(k=0; k<ssao.numFbo; k++) {
					ssao.ambientTex[k] = makeTex(ssao.texTarget, internalFormat, ssao.ambientWidth, ssao.ambientHeight, format, GL_UNSIGNED_BYTE, NULL);
					if(!ssao.ambientTex[k]) break;
				}
				if(k != ssao.numFbo) continue;

				if (ssao.blurTex) {
					glDeleteTextures(ssao.numFbo, ssao.blurTex); CHECKGL;
					memset(ssao.blurTex, 0, sizeof(ssao.blurTex));
				}
				for(k=0; k<ssao.numFbo; k++) {
					ssao.blurTex[k] = makeTex(ssao.texTarget, internalFormat, ssao.blurWidth, ssao.blurHeight, format, GL_UNSIGNED_BYTE, NULL);
					if(!ssao.blurTex[k]) break;
				}
				if(k != ssao.numFbo) continue;

				if (ssao.blur2Tex) {
					glDeleteTextures(ssao.numFbo, ssao.blur2Tex); CHECKGL;
					memset(ssao.blur2Tex, 0, sizeof(ssao.blur2Tex));
				}
				for(k=0; k<ssao.numFbo; k++) {
					ssao.blur2Tex[k] = makeTex(ssao.texTarget, internalFormat, ssao.blurWidth, ssao.ambientHeight, format, GL_UNSIGNED_BYTE, NULL);
					if(!ssao.blur2Tex[k]) break;
				}
				if(k != ssao.numFbo) continue;

				for(k=0; k<ssao.numFbo; k++) {
					ret = setupFbo(ssao.texTarget, ssao.ambientTex[k], 0, 0, ssao.ambientDepth[k], ssao.ambientWidth, ssao.ambientHeight);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECKGL;
					restoreFbo();
					if(ret != GL_FRAMEBUFFER_COMPLETE_EXT) break;

					ret = setupFbo(ssao.texTarget, ssao.blurTex[k], 0, 0, ssao.blurDepth[k], ssao.blurWidth, ssao.blurHeight);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECKGL;
					restoreFbo();
					if(ret != GL_FRAMEBUFFER_COMPLETE_EXT) break;

					ret = setupFbo(ssao.texTarget, ssao.blur2Tex[k], 0, 0, ssao.blur2Depth[k], ssao.blurWidth, ssao.ambientHeight);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECKGL;
					restoreFbo();
					if(ret != GL_FRAMEBUFFER_COMPLETE_EXT) break;
				}
				if(k != ssao.numFbo) continue;

				break;
			}
			if(ret == GL_FRAMEBUFFER_COMPLETE_EXT) break;
		}
		if(ret == GL_FRAMEBUFFER_COMPLETE_EXT) break;

		// Try again with RGB formats
		if(singleChannel) {
			singleChannel = false;
			continue;
		}

		break;
	}
	if(ret != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("Could not find a valid rendertarget combination for the ssao buffers\n");
		return false;
	}
	printf("Found a valid rendertarget combination for the ssao buffers (color: %#04x / depth: %#04x)\n", internalFormat, depthInternalFormat);

	// Allocate reduced size render targets
	if (ssao.depthDownsample > 1) {
		ret = GL_NO_ERROR;
		for(i=0; i<numColorFormats; i++) {
			internalFormat = colorFormats[i];

			if (ssao.reducedColorTex[0]) {
				glDeleteTextures(ssao.numFbo, ssao.reducedColorTex); CHECKGL;
			}
			for(k=0; k<ssao.numFbo; k++) {
				ssao.reducedColorTex[k] = makeTex(ssao.texTarget, internalFormat, ssao.depthWidth, ssao.depthHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				if(!ssao.reducedColorTex[k]) break;
			}
			if(k != ssao.numFbo) continue;

			for(j=0; j<numHqDepthFormats; j++) {
				depthInternalFormat = hqDepthFormats[j];

				if (ssao.reducedDepthTex) {
					glDeleteTextures(ssao.numFbo, ssao.reducedDepthTex); CHECKGL;
				}
				for(k=0; k<ssao.numFbo; k++) {
					ssao.reducedDepthTex[k] = makeTex(ssao.texTarget, depthInternalFormat, ssao.depthWidth, ssao.depthHeight, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
					if(!ssao.reducedDepthTex[k]) break;
				}
				if(k != ssao.numFbo) continue;

				for(k=0; k<ssao.numFbo; k++) {
					ret = setupFbo(ssao.texTarget, ssao.reducedColorTex[k], ssao.texTarget, ssao.reducedDepthTex[k], 0, ssao.depthWidth, ssao.depthHeight);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECKGL;
					restoreFbo();
					if(ret != GL_FRAMEBUFFER_COMPLETE_EXT) break;
				}
				if(k != ssao.numFbo) continue;
	
				break;
			}
			if(ret == GL_FRAMEBUFFER_COMPLETE_EXT) break;
		}
		if(ret != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("Could not find a valid rendertarget combination for the reduced buffers\n");
			return false;
		}
		printf("Found a valid rendertarget combination for the reduced buffers (color: %#04x / depth: %#04x)\n", internalFormat, depthInternalFormat);
	}

	return true;
}

static float frand(float rmin, float rmax)
{
	float range = rmax - rmin;
	return(((float)rand() * range) / (float)RAND_MAX) + rmin;
}


static void frandpair(float * a, float * b)
{
	float x1, x2, w;
	do {
		x1 = frand(0.0f, 2.0f) - 1.0f;
		x2 = frand(0.0f, 2.0f) - 1.0f;
		w = x1 * x1 + x2 * x2;
	} while ( w == 0.0f || w >= 1.0f );

	w = sqrtf((-2.0f*logf(w)) / w);
	*a = x1 * w;
	*b = x2 * w;
}

static int randpoisson(int lambda)
{
	const float enl = expf(-lambda);
	float prod = 1.0f;
	int x = -1;

	do {
		x++;
		prod *= frand(0,1);
	} while (prod > enl);

	return x;
}

static float frandinvpoisson(int lambda, float min, float max)
{
	return (max-min)/(randpoisson(lambda)+1) + min;
}

static void vec3randpair(Vec3 * a, Vec3 * b, int lambda)
{
	float sa, sb;

	do {
		frandpair(*a+0, *b+0);
		frandpair(*a+1, *b+1);
		frandpair(*a+2, *b+2);
		sa = lengthVec3Squared(*a);
		sb = lengthVec3Squared(*b);
	} while (sa == 0.0f || sb == 0.0f || sa > 1.0f || sb > 1.0f);

	sa = 1.0f / sqrtf(sa);
	sb = 1.0f / sqrtf(sb);

	scaleVec3(*a, sa, *a);
	scaleVec3(*b, sb, *b);
}

static void vec3applyforce(Vec3 * vectors, int size, int iterations)
{
	while(iterations-- > 0) {
		int i;
		for (i=0; i<size; i++) {
			int j;
			Vec3 delta = {0,0,0};
			Vec3 * v = vectors + i;

			for (j=0; j<size; j++) {
				Vec3 dist;
				float d;

				subVec3(*v, vectors[j], dist);
				d = dotVec3(dist, dist);
				if(d != 0.0f) {
					d = 0.5f / d;
					scaleVec3(dist, d, dist);
					addVec3(delta, dist, delta);
				}
			}

			addVec3(*v, delta, *v);
			normalVec3(*v);
		}
	}
}

static GLuint sphereTex = 0;
#define maxSphereSize 256
static int sphereSize = 4;
static int sphereSeed = 0;
static float sphereMinMax[2] = {0.5,1.0};
static int sphereLambda = 2;
static int sphereIterations = 2;
static uint8_t sphereData[maxSphereSize*maxSphereSize][3];

static void setupSamplingSphere()
{
	int i;
	Vec3 vectors[maxSphereSize*maxSphereSize];
	int sphereSizeSq = sphereSize*sphereSize;

	if (sphereTex) {
		glDeleteTextures(1, &sphereTex); CHECKGL;
	}

	srand(sphereSeed);

	// Start with non-uniform random gaussian sphere data
	for (i=0; i<sphereSizeSq; i+=2) {
		Vec3 * a = vectors + i;
		Vec3 * b = vectors + i + 1;
		vec3randpair(a, b, sphereLambda);
	}

	// Apply a few iterations of force between the points to push neighboring particles away
	for(i=0; i<sphereSizeSq; i+=32) {
		vec3applyforce(vectors + i, MIN(32, sphereSizeSq), sphereIterations);
	}

	// Apply inverse poisson distribution to the sphere lengths
	if(sphereLambda) {
		for (i=0; i<sphereSizeSq; i++) {
			float s = frandinvpoisson(sphereLambda, sphereMinMax[0], sphereMinMax[1]) / lengthVec3(vectors[i]);
			scaleVec3(vectors[i], s, vectors[i]);
		}
	}

	// Convert to unsigned integer data
	for (i=0; i<sphereSizeSq; i++) {
		Vec3 * v = vectors + i;
		uint8_t * u = sphereData[i];
		
		u[0] = (uint8_t)(((*v)[0] + 1.0) * 127.5);
		u[1] = (uint8_t)(((*v)[1] + 1.0) * 127.5);
		u[2] = (uint8_t)(((*v)[2] + 1.0) * 127.5);
	}

	sphereTex = makeTex(GL_TEXTURE_2D, GL_RGBA8, sphereSize, sphereSize, GL_RGB, GL_UNSIGNED_BYTE, sphereData);
	setTexMode(GL_TEXTURE_2D, sphereTex, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
}

static void writeSphere()
{
	int i;
	FILE * f = fopen("sphere.csv", "w");
	for (i=0; i<sphereSize; i++) {
		int j;
		for (j=0; j<sphereSize; j++) {
			uint8_t * u = sphereData[i*sphereSize + j];
			Vec3 v;
			v[0] = (((float)u[0])/127.5-1.0);
			v[1] = (((float)u[1])/127.5-1.0);
			v[2] = (((float)u[2])/127.5-1.0);
			fprintf(f, "%d\t%d\t%d\t%g\n", u[0], u[1], u[2], lengthVec3(v));
		}
	}
	fclose(f);
}

static void plotSphere()
{
	FILE * f = fopen("sphere.plt", "w");
	fprintf(f, "set xrange [0:255]\n");
	fprintf(f, "set yrange [0:255]\n");
	fprintf(f, "set zrange [0:255]\n");
	fprintf(f, "splot 'sphere.csv' using 1:2:3\n");
	fclose(f);

	writeSphere();

	system("c:/work/gnuplot/bin/wgnuplot_pipes.exe sphere.plt -");
}

static void plotSphereLength()
{
	FILE * f = fopen("spherelength.plt", "w");
	fprintf(f, "plot 'sphere.csv' using 4\n");
	fclose(f);

	writeSphere();

	system("c:/work/gnuplot/bin/wgnuplot_pipes.exe spherelength.plt -");
}

static void vec3unitrand(Vec3 * v)
{
	float l;
	do {
		(*v)[0] = frand(-1,1);
		(*v)[1] = frand(-1,1);
		(*v)[2] = frand(-1,1);
		l = lengthVec3Squared(*v);
	} while (l == 0.0f || l > 1.0f);
	l = 1.0f / sqrtf(l);
	scaleVec3(*v, l, *v);
}

#define sampleVectorSize 64
static Vec3 sampleVectors[sampleVectorSize];
static int sampleSeed = 35;
static float sampleMinMax[2] = {0.05,1.0};
static int sampleLambda = 8;
static int sampleIterations = 100;

static void setupSampleVectors()
{
	int i;

	srand(sampleSeed);

	// Start with a set of random points on the sphere
	for (i = 0; i < ssao.sampleCount; i++) {
		vec3unitrand(sampleVectors + i);
	}

	// Apply force to the points to minimize the distances between them
	vec3applyforce(sampleVectors, ssao.sampleCount, sampleIterations);

	// Spread the points out over the range of the sphere
	if(sampleLambda > 1) {
		for (i=0; i<ssao.sampleCount; i++) {
			float range = sampleMinMax[1] - sampleMinMax[0];
			float mod = (float)(i%sampleLambda)/(float)(sampleLambda-1);
			float s = (range * mod + sampleMinMax[0]) / lengthVec3(sampleVectors[i]);
			scaleVec3(sampleVectors[i], s, sampleVectors[i]);
		}
	}
}

static void writeSample()
{
	int i;
	FILE * f = fopen("sample.csv", "w");
	for (i=0; i<ssao.sampleCount; i++) {
		Vec3 * v = sampleVectors + i;
		fprintf(f, "%g\t%g\t%g\t%g\n", (*v)[0], (*v)[1], (*v)[2], lengthVec3(*v));
	}
	fclose(f);
}

static void plotSample()
{
	FILE * f = fopen("sample.plt", "w");
	fprintf(f, "set xrange [-1:1]\n");
	fprintf(f, "set yrange [-1:1]\n");
	fprintf(f, "set zrange [-1:1]\n");
	fprintf(f, "splot 'sample.csv' using 1:2:3\n");
	fclose(f);

	writeSample();

	system("c:/work/gnuplot/bin/wgnuplot_pipes.exe sample.plt -");
}

static void plotSampleLength()
{
	FILE * f = fopen("samplelength.plt", "w");
	fprintf(f, "plot 'sample.csv' using 4\n");
	fclose(f);

	writeSample();

	system("c:/work/gnuplot/bin/wgnuplot_pipes.exe samplelength.plt -");
}

static void drawQuadShader(CGtechnique technique, float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, float z)
{	
	CGpass pass = cgGetFirstPass(technique);
	CGprofile vertexProfile = cgGetProgramProfile(cgGetPassProgram(pass, CG_VERTEX_DOMAIN));
	CGprofile fragmentProfile = cgGetProgramProfile(cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN));
	cgGLEnableProfile(vertexProfile);
	cgGLEnableProfile(fragmentProfile);
	rt_cgfxSetRenderPass(technique, pass);
	glBegin(GL_QUADS);
	glTexCoord2f(u1, v1);
	glVertex3f(x1, y1, z);
	glTexCoord2f(u1, v2);
	glVertex3f(x1, y2, z);
	glTexCoord2f(u2, v2);
	glVertex3f(x2, y2, z);
	glTexCoord2f(u2, v1);
	glVertex3f(x2, y1, z);
	glEnd(); CHECKGL;
	rt_cgfxUnsetRenderPass(technique, pass);
	cgGLDisableProfile(fragmentProfile);
	cgGLDisableProfile(vertexProfile);
}

static void drawFullscreenShader(CGtechnique technique, bool firstUsageInFrame)
{
	if(firstUsageInFrame && rdr_frame_state.sliClear) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); CHECKGL;
	}
	drawQuadShader(technique, -1, -1, 1, 1, 0, 0, 1, 1, 0);
}

static const char * getShaderOptimizationArg()
{
	const char * opt = NULL;
	switch(shaderOptimization) {
		case optPrecise: opt = "-bestprecision"; break;
		case optNone: opt = "-nofastmath"; break;
		case optFastMath: opt = "-fastmath"; break;
		case optFastPrecision: opt = "-fastprecision"; break;
	};
	return opt;
}

static void reloadShaders()
{
	const char * args[] = {
		"-strict",
		"-profileopts",
		"MaxInstructions=2048,NumInstructionSlots=4096,NumMathInstructionSlots=4096,MaxLocalParams=1024,NumTemps=512",
		getShaderOptimizationArg(),
		(ssao.lastTexTarget == GL_TEXTURE_RECTANGLE_ARB) ? "-DUSE_TEX_RECT=1" : "-DUSE_TEX_RECT=0",
		NULL
	};

	ssao.cgEffect = rt_cgfxGetEffect( kSSAO_EffectName, args, true );
	{
		bool bWarningOpt = rt_cgfxGetDisplayWarnings();
		if ( ! game_state.shaderInitLogging ) {
			rt_cgfxSetDisplayWarnings(false);
		}
		ssao.color2d = rt_cgfxGetTechnique( ssao.cgEffect, "color2d" );
		ssao.depth2d = rt_cgfxGetTechnique( ssao.cgEffect, "depth2d" );
		ssao.colorDepth2d = rt_cgfxGetTechnique( ssao.cgEffect, "colorDepth2d" );
		ssao.colorDepthAverage2d = rt_cgfxGetTechnique( ssao.cgEffect, "colorDepthAverage2d" );
		ssao.colorDepthMin2d = rt_cgfxGetTechnique( ssao.cgEffect, "colorDepthMin2d" );
		ssao.colorDepthMax2d = rt_cgfxGetTechnique( ssao.cgEffect, "colorDepthMax2d" );
		ssao.colorFog2d = rt_cgfxGetTechnique( ssao.cgEffect, "colorFog2d" );
		ssao.colorBlendFog2d = rt_cgfxGetTechnique( ssao.cgEffect, "colorBlendFog2d" );
		ssao.blurFast2d = rt_cgfxGetTechnique( ssao.cgEffect, "blurFast2d" );
		ssao.gaussian1d = rt_cgfxGetTechnique( ssao.cgEffect, "gaussian1d" );
		ssao.gaussianDepth1d = rt_cgfxGetTechnique( ssao.cgEffect, "gaussianDepth1d" );
		ssao.bilateral1d = rt_cgfxGetTechnique( ssao.cgEffect, "bilateral1d" );
		ssao.bilateralDepth1d = rt_cgfxGetTechnique( ssao.cgEffect, "bilateralDepth1d" );
		ssao.trilateral1d = rt_cgfxGetTechnique( ssao.cgEffect, "trilateral1d" );
		ssao.ssaoLow = rt_cgfxGetTechnique( ssao.cgEffect, "ssaoLow" );
		ssao.ssaoMedium = rt_cgfxGetTechnique( ssao.cgEffect, "ssaoMedium" );
		ssao.ssaoHigh = rt_cgfxGetTechnique( ssao.cgEffect, "ssaoHigh" );
		ssao.ssaoUltra = rt_cgfxGetTechnique( ssao.cgEffect, "ssaoUltra" );
		
		rt_cgfxSetDisplayWarnings(bWarningOpt);
	}

	ssao.modelViewProjParam = cgGetNamedEffectParameter(ssao.cgEffect, "ModelViewProj");
	ssao.screenstepParam = cgGetNamedEffectParameter(ssao.cgEffect, "screenstep");
	ssao.depthTexParam = cgGetNamedEffectParameter(ssao.cgEffect, "depthTex");
	ssao.depthTexNPOTParam = cgGetNamedEffectParameter(ssao.cgEffect, "depthTexNPOT");
	ssao.colorTexParam = cgGetNamedEffectParameter(ssao.cgEffect, "colorTex");
	ssao.colorTexNPOTParam = cgGetNamedEffectParameter(ssao.cgEffect, "colorTexNPOT");
	ssao.color2TexParam = cgGetNamedEffectParameter(ssao.cgEffect, "color2Tex");
	ssao.color2TexNPOTParam = cgGetNamedEffectParameter(ssao.cgEffect, "color2TexNPOT");

	ssao.reducedDepthTexParam = cgGetNamedEffectParameter(ssao.cgEffect, "reducedDepthTex");
	ssao.reducedDepthTexNPOTaram = cgGetNamedEffectParameter(ssao.cgEffect, "reducedDepthTexNPOT");
	ssao.reducedColorTexParam = cgGetNamedEffectParameter(ssao.cgEffect, "reducedColorTex");
	ssao.reducedColorTexNPOTaram = cgGetNamedEffectParameter(ssao.cgEffect, "reducedColorTexNPOT");
	ssao.ssaoSizeParam = cgGetNamedEffectParameter(ssao.cgEffect, "ssaoSize");
	ssao.randomVectorSizeParam = cgGetNamedEffectParameter(ssao.cgEffect, "randomVectorSize");
	ssao.randomVectorsParam = cgGetNamedEffectParameter(ssao.cgEffect, "randomVectors");
	ssao.nearFarParam = cgGetNamedEffectParameter(ssao.cgEffect, "nearFar");
	ssao.nearFarLimitParam = cgGetNamedEffectParameter(ssao.cgEffect, "nearFarLimit");
	ssao.pixelDepthSpreadParam = cgGetNamedEffectParameter(ssao.cgEffect, "pixelDepthSpread");
	ssao.preWeightParam = cgGetNamedEffectParameter(ssao.cgEffect, "preWeight");
	ssao.postWeightParam = cgGetNamedEffectParameter(ssao.cgEffect, "postWeight");
	ssao.sampleCountParam = cgGetNamedEffectParameter(ssao.cgEffect, "sampleCount");
	ssao.sampleVectorsParam = cgGetNamedEffectParameter(ssao.cgEffect, "sampleVectors");
	ssao.sigmaParam = cgGetNamedEffectParameter(ssao.cgEffect, "sigma");

	ssao.newFeatureParam = cgGetNamedEffectParameter(ssao.cgEffect, "newFeature");
	ssao.fogColorParam = cgGetNamedEffectParameter(ssao.cgEffect, "fogColor");
	ssao.fogParamParam = cgGetNamedEffectParameter(ssao.cgEffect, "fogParam");
	ssao.depthInvMatrixParam = cgGetNamedEffectParameter(ssao.cgEffect, "depthInvMatrix");
}

static void drawAmbient()
{
	const FogContext *old_fog_context;
	FogContext dummy_fog_context;

	// Reset WCW_statemgmt's cached vertex/fragment shaders
	WCW_ResetState_CgFx();

	// Save the fog state because WCW_ResetState() will lose the old fog min/max values
	old_fog_context = WCW_GetFogContext();
	dummy_fog_context = *old_fog_context;
	WCW_SetFogContext(&dummy_fog_context);

	// Reset rt_cgfx's cached vertex/fragmet shaders
	if ( rt_cgGetCgShaderMode() )
	{
		rt_cgResetProgram(kShaderPgmType_FRAGMENT);
		rt_cgResetProgram(kShaderPgmType_VERTEX);
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;

	glViewport(0, 0, ssao.screenWidth, ssao.screenHeight); CHECKGL;
	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;

	glMatrixMode(GL_PROJECTION); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glOrtho(-1, 1, -1, 1, 0, 1); CHECKGL;

	glDisable(GL_DEPTH_TEST); CHECKGL;
	glDepthMask(GL_FALSE); CHECKGL;
	WCW_DisableBlend();
	glDisable(GL_ALPHA_TEST); CHECKGL;

	cgGLSetManageTextureParameters(ssao.cgContext, CG_TRUE);

	// Copy color and depth
	if (ssao.irradiance || enableFog) {
		winCopyFrameBufferDirect(ssao.texTarget, ssao.colorTex[ssao.curFbo], ssao.depthTex[ssao.curFbo], 0, 0, ssao.screenWidth, ssao.screenHeight, 0, false, false);
	} else {
		winCopyFrameBufferDirect(ssao.texTarget, 0, ssao.depthTex[ssao.curFbo], 0, 0, ssao.screenWidth, ssao.screenHeight, 0, false, false);
	}

	// Downsample depth
	if (ssao.depthDownsample > 1) {
		CGtechnique technique = NULL;
		float screenstep[4] = {1.0f / (float)ssao.screenWidth, 1.0f / (float)ssao.screenHeight, 0.0f, 0.0f};
	
		switch (ssao.depthReduce) {
			case depthPoint: technique = ssao.irradiance ? ssao.colorDepth2d : ssao.depth2d; break;
			case depthAverage: technique = ssao.colorDepthAverage2d; break;
			case depthMin: technique = ssao.colorDepthMin2d; break;
			case depthMax: technique = ssao.colorDepthMax2d; break;
		}

		cgGLSetStateMatrixParameter(ssao.modelViewProjParam,
					    CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);

		cgGLSetParameter4fv(ssao.screenstepParam, screenstep);

		setTexMode(ssao.texTarget, ssao.depthTex[ssao.curFbo], GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		cgGLSetupSampler(ssao.depthTexParam, ssao.depthTex[ssao.curFbo]);
		cgGLSetParameter2f(ssao.depthTexNPOTParam, ssao.screenWidth, ssao.screenHeight);

		if (ssao.irradiance) {
			setTexMode(ssao.texTarget, ssao.colorTex[ssao.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(ssao.colorTexParam, ssao.colorTex[ssao.curFbo]);
			cgGLSetParameter2f(ssao.colorTexNPOTParam, ssao.screenWidth, ssao.screenHeight);
		} else {
			if (ssao.depthReduce != depthPoint) {
				cgGLSetupSampler(ssao.colorTexParam, ssao.whiteTex);
			}
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); CHECKGL;
		}

		glEnable(GL_DEPTH_TEST); CHECKGL;
		glDepthFunc(GL_ALWAYS); CHECKGL;
		glDepthMask(GL_TRUE); CHECKGL;

		setupFbo(ssao.texTarget, ssao.reducedColorTex[ssao.curFbo], ssao.texTarget, ssao.reducedDepthTex[ssao.curFbo], 0, ssao.depthWidth, ssao.depthHeight);
		drawFullscreenShader(technique, true);
		restoreFbo();

		glDisable(GL_DEPTH_TEST); CHECKGL;
		glDepthMask(GL_FALSE); CHECKGL;
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); CHECKGL;
	}

	// Calculate SSAO
	if (ssao.strength) {
		GLfloat borderValues[4] = {border, border, border, border};
		CGtechnique technique = NULL;
		switch (ssao.strength) {
			case AMBIENT_LOW: technique = ssao.ssaoLow; break;
			case AMBIENT_MEDIUM: technique = ssao.ssaoMedium; break;
			case AMBIENT_HIGH: technique = ssao.ssaoHigh; break;
			case AMBIENT_ULTRA: technique = ssao.ssaoUltra; break;
		}

		WCW_BindTexture(ssao.texTarget, 0, (ssao.depthDownsample > 1) ? ssao.reducedDepthTex[ssao.curFbo] : ssao.depthTex[ssao.curFbo]);
		glTexParameterfv(ssao.texTarget, GL_TEXTURE_BORDER_COLOR, borderValues); CHECKGL;
		WCW_BindTexture(ssao.texTarget, 0, 0);

		cgGLSetStateMatrixParameter(ssao.modelViewProjParam,
					    CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

		setTexMode(ssao.texTarget, ssao.depthTex[ssao.curFbo], GL_NEAREST, GL_NEAREST, ssaoEdge, ssaoEdge);
		cgGLSetupSampler(ssao.depthTexParam, ssao.depthTex[ssao.curFbo]);
		cgGLSetParameter2f(ssao.depthTexNPOTParam, ssao.screenWidth, ssao.screenHeight);

		setTexMode(ssao.texTarget, ssao.reducedDepthTex[ssao.curFbo], GL_NEAREST, GL_NEAREST, ssaoEdge, ssaoEdge);
		cgGLSetupSampler(ssao.reducedDepthTexParam, (ssao.depthDownsample > 1) ? ssao.reducedDepthTex[ssao.curFbo] : ssao.depthTex[ssao.curFbo]);
		cgGLSetParameter2f(ssao.reducedDepthTexNPOTaram, ssao.depthWidth, ssao.depthHeight);

		setTexMode(ssao.texTarget, (ssao.depthDownsample > 1) ? ssao.reducedColorTex[ssao.curFbo] : ssao.colorTex[ssao.curFbo],
				   GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		cgGLSetupSampler(ssao.reducedColorTexParam,
						 ssao.irradiance ? ((ssao.depthDownsample > 1) ? ssao.reducedColorTex[ssao.curFbo] : ssao.colorTex[ssao.curFbo]) : ssao.blackTex);
		cgGLSetParameter2f(ssao.reducedColorTexNPOTaram, ssao.depthWidth, ssao.depthHeight);

		cgGLSetParameter2f(ssao.ssaoSizeParam, ssao.ambientWidth, ssao.ambientHeight);
		cgGLSetParameter1f(ssao.randomVectorSizeParam, (float)sphereSize);
		cgGLSetupSampler(ssao.randomVectorsParam, sphereTex);
		cgGLSetParameter2f(ssao.nearFarParam, game_state.nearz, game_state.farz);
		cgGLSetParameter2fv(ssao.nearFarLimitParam, nearFarLimit);

		cgGLSetParameter2fv(ssao.pixelDepthSpreadParam, pixelDepthSpread);
		cgGLSetParameter4fv(ssao.preWeightParam, preWeight);
		cgGLSetParameter4f(ssao.postWeightParam, ssao.postScale, ssao.postPower, 1.0f, ssao.irradianceScale);
		cgGLSetParameter1f(ssao.sampleCountParam, ssao.sampleCount);
		cgGLSetParameterArray3f(ssao.sampleVectorsParam, 0, ssao.sampleCount, (const float *)sampleVectors);

		setupFbo(ssao.texTarget, ssao.ambientTex[ssao.curFbo], 0, 0, ssao.ambientDepth[ssao.curFbo], ssao.ambientWidth, ssao.ambientHeight);
		drawFullscreenShader(technique, true);
		restoreFbo();
	}

	// Blur ssao
	if (ssao.blur != AMBIENT_NO_BLUR) {
		const CGtechnique blurPrograms[] = {
			NULL,
			ssao.blurFast2d,
			ssao.gaussian1d,
			ssao.bilateral1d,
			ssao.gaussianDepth1d,
			ssao.bilateralDepth1d,
			ssao.trilateral1d,
		};
		CGtechnique technique = technique = blurPrograms[ssao.blur];
		float screenstep[4] = {1.0f / (float)ssao.ambientWidth, 1.0f / (float)ssao.ambientHeight, 0.0f, 0.0f};

		STATIC_ASSERT(ARRAY_SIZE(blurPrograms) == AMBIENT_BLUR_MAX);

		if(ssao.blur == AMBIENT_FAST_BLUR) {
			screenstep[2] = 0.5f / (float)ssao.ambientWidth;
			screenstep[3] = 0.5f / (float)ssao.ambientHeight;
		}

		cgGLSetStateMatrixParameter(ssao.modelViewProjParam,
					    CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);

		setTexMode(ssao.texTarget, ssao.ambientTex[ssao.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		cgGLSetupSampler(ssao.colorTexParam, ssao.ambientTex[ssao.curFbo]);
		cgGLSetParameter2f(ssao.colorTexNPOTParam, ssao.ambientWidth, ssao.ambientHeight);

		if (ssao.blur == AMBIENT_GAUSSIAN_DEPTH || ssao.blur == AMBIENT_BILATERAL_DEPTH || ssao.blur == AMBIENT_TRILATERAL) {
			setTexMode(ssao.texTarget, ssao.depthTex[ssao.curFbo], GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(ssao.depthTexParam, ssao.depthTex[ssao.curFbo]);
			cgGLSetParameter2f(ssao.depthTexNPOTParam, ssao.screenWidth, ssao.screenHeight);
		}

		cgGLSetParameter4fv(ssao.sigmaParam, sigma);

		if(ssao.blur == AMBIENT_FAST_BLUR) {
			cgGLSetParameter4fv(ssao.screenstepParam, screenstep);

			setupFbo(ssao.texTarget, ssao.blurTex[ssao.curFbo], 0, 0, ssao.blurDepth[ssao.curFbo], ssao.blurWidth, ssao.blurHeight);
			drawFullscreenShader(technique, true);
			restoreFbo();
		} else  {
			float screenstepWidth[4] = {screenstep[0], 0.0f, screenstep[2], 0.0f};
			float screenstepHeight[4] = {0.0f, screenstep[1], 0.0f, screenstep[3]};

			cgGLSetParameter4fv(ssao.screenstepParam, screenstepWidth);

			setupFbo(ssao.texTarget, ssao.blur2Tex[ssao.curFbo], 0, 0, ssao.blur2Depth[ssao.curFbo], ssao.blurWidth, ssao.ambientHeight);
			drawFullscreenShader(technique, true);
			restoreFbo();

			setTexMode(ssao.texTarget, ssao.blur2Tex[ssao.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(ssao.colorTexParam, ssao.blur2Tex[ssao.curFbo]);
			cgGLSetParameter2f(ssao.colorTexNPOTParam, ssao.blurWidth, ssao.ambientHeight);

			cgGLSetParameter4fv(ssao.screenstepParam, screenstepHeight);

			setupFbo(ssao.texTarget, ssao.blurTex[ssao.curFbo], 0, 0, ssao.blurDepth[ssao.curFbo], ssao.blurWidth, ssao.blurHeight);
			drawFullscreenShader(technique, true);
			restoreFbo();
		}
	}

	// Display results
	if (draw) {
		CGtechnique technique = enableFog ? (enableBlend ? ssao.colorBlendFog2d : ssao.colorFog2d) : ssao.color2d;

		Vec4 newFeature;
		getNewFeatureForCurWindow(&newFeature, NEWFEATURE_SSAO);

		cgGLSetStateMatrixParameter(ssao.modelViewProjParam,
					    CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);

		setTexMode(ssao.texTarget, ssao.ambientTex[ssao.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		setTexMode(ssao.texTarget, ssao.blurTex[ssao.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		cgGLSetupSampler(ssao.colorTexParam, ssao.strength ? (ssao.blur ? ssao.blurTex[ssao.curFbo] : ssao.ambientTex[ssao.curFbo]) : ssao.whiteTex);
		cgGLSetParameter2f(ssao.colorTexNPOTParam, ssao.blur ? ssao.blurWidth : ssao.ambientWidth, ssao.blur ? ssao.blurHeight : ssao.ambientHeight);
		cgGLSetParameter4fv(ssao.newFeatureParam, newFeature);

		if (enableFog) {
			GLint fogDistanceMode;
			float fogColor[4];
			GLint fogMode;
			float fogDensity;
			float fogParam[4];
			Mat44 depthInvMatrix;

			// @todo This could be slow, replace me...
#if 0
			glGetFloatv(GL_FOG_COLOR, fogColor); CHECKGL;
			glGetIntegerv(GL_FOG_MODE, &fogMode); CHECKGL;
			glGetFloatv(GL_FOG_DENSITY, &fogDensity); CHECKGL;
			glGetFloatv(GL_FOG_START, &fogParam[0]); CHECKGL;
			glGetFloatv(GL_FOG_END, &fogParam[1]); CHECKGL;

			gldGetError();
			glGetIntegerv(GL_FOG_DISTANCE_MODE_NV, &fogDistanceMode);
			if(gldGetError()) fogDistanceMode = 0;
#else
			const FogContext *fog_context = WCW_GetFogContext();
			copyVec3(fog_context->fog_color, fogColor);
			fogDensity = fog_context->curr_fog[0];
			fogParam[0] = fog_context->curr_fog[1];
			fogParam[1] = fog_context->curr_fog[2];
			fogMode = fog_context->curr_fog[3];
			// rdrSetFogDirect() in rt_prim.c sets GL_FOG_DISTANCE_MODE_NV to GL_EYE_RADIAL_NV if (rdr_caps.chip & NV1X)
			fogDistanceMode = (rdr_caps.chip & NV1X) ? GL_EYE_RADIAL_NV : 0;
#endif

			fogParam[2] = 1.0f / (fogParam[1] - fogParam[0]);
			fogParam[3] = (fogDistanceMode == GL_EYE_RADIAL_NV) ? 1.0f : 0.0f;

			cgGLSetParameter4fv(ssao.fogColorParam, fogColor);
			cgGLSetParameter4fv(ssao.fogParamParam, fogParam);

			invertMat44Copy(rdr_view_state.projectionMatrix, depthInvMatrix);
			cgGLSetMatrixParameterfc(ssao.depthInvMatrixParam, (const float *)depthInvMatrix);

			setTexMode(ssao.texTarget, ssao.depthTex[ssao.curFbo], GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(ssao.depthTexParam, ssao.depthTex[ssao.curFbo]);
			cgGLSetParameter2f(ssao.depthTexNPOTParam, ssao.screenWidth, ssao.screenHeight);

			if (enableBlend) {
				setTexMode(ssao.texTarget, ssao.colorTex[ssao.curFbo], GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
				cgGLSetupSampler(ssao.color2TexParam, ssao.colorTex[ssao.curFbo]);
				cgGLSetParameter2f(ssao.color2TexNPOTParam, ssao.screenWidth, ssao.screenHeight);
			}
		} else if (enableBlend) {
			WCW_BlendFunc(GL_DST_COLOR, GL_ZERO);
		}

		// Preserve the existing alpha buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;

		// Enable depth compare to avoid drawing ssao on the sky
		glEnable(GL_DEPTH_TEST); CHECKGL;
		glDepthFunc(drawDepthTest); CHECKGL;

		if (draw == drawVertical) {
			drawQuadShader(technique, 0, -1, 1, 1, 0.5, 0, 1, 1, drawDepth);
		} else if (draw == drawHorizontal) {
			drawQuadShader(technique, -1, 0, 1, 1, 0, 0.5, 1, 1, drawDepth);
		} else {
			drawQuadShader(technique, -1, -1, 1, 1, 0, 0, 1, 1, drawDepth);
		}
	}

	cgGLSetManageTextureParameters(ssao.cgContext, CG_FALSE);

	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glPopMatrix(); CHECKGL;

	glMatrixMode(GL_PROJECTION); CHECKGL;
	glPopMatrix(); CHECKGL;

	glPopAttrib(); CHECKGL;
	WCW_FetchGLState();
	WCW_ResetState();

	// Restore the old fog state
	WCW_SetFogContext((FogContext*)old_fog_context);

	if(game_state.renderdump) {
		void * depth, * reducedDepth;

		depth = malloc(ssao.screenWidth * ssao.screenHeight * 4);
		reducedDepth = malloc(ssao.depthWidth * ssao.depthHeight * 4);

		WCW_BindTexture(ssao.texTarget, 0, ssao.depthTex[ssao.curFbo]);
		glGetTexImage(ssao.texTarget, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, depth); CHECKGL;

		WCW_BindTexture(ssao.texTarget, 0, ssao.reducedDepthTex[ssao.curFbo]);
		glGetTexImage(ssao.texTarget, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, reducedDepth); CHECKGL;

		tiffSave("C:/ssao_screen_depth.tiff", depth, ssao.screenWidth, ssao.screenHeight, 1, 4, 1, 0, 0);
		tiffSave("C:/ssao_reduced_depth.tiff", depth, ssao.depthWidth, ssao.depthHeight, 1, 4, 1, 0, 0);

		free(depth);
		free(reducedDepth);
	}

}

// Debug variables
static void registerDebugVariables()
{
	static const int strengthModes[] = {AMBIENT_OFF, AMBIENT_LOW, AMBIENT_MEDIUM, AMBIENT_HIGH, AMBIENT_ULTRA};
	static const char * strengthNames[] = {"Off", "Low", "Medium", "High", "Ultra", NULL};

	static const int resolutionModes[] = {AMBIENT_RES_HIGH_PERFORMANCE, AMBIENT_RES_PERFORMANCE, AMBIENT_RES_QUALITY, AMBIENT_RES_HIGH_QUALITY, AMBIENT_RES_SUPER_HIGH_QUALITY};
	static const char * resolutionNames[] = {"2 / 2 / 4", "2 / 2 / 2", "2 / 1 / 2", "1 / 1 /2", "1 / 1 / 1", NULL};

	static int blurModes[] = {AMBIENT_NO_BLUR, AMBIENT_FAST_BLUR, AMBIENT_GAUSSIAN, AMBIENT_BILATERAL,
		AMBIENT_GAUSSIAN_DEPTH, AMBIENT_BILATERAL_DEPTH, AMBIENT_TRILATERAL};
	static const char * blurNames[] = {
		"None",
		"2D Fast Blur",
		"Two Pass 1D Gaussian",
		"Two Pass 1D Bilateral",
		"Two Pass 1D Gaussian Depth",
		"Two Pass 1D Bilateral Depth",
		"Two Pass 1D Trilateral",
		NULL,
	};

	static const int depthReduceModes[] = {depthPoint, depthAverage, depthMin, depthMax};
	static const char * depthReduceNames[] = {"Point", "Average", "Min", "Max", NULL};

	static const int texTargetModes[] = {GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB};
	static const char * texTargetNames[] = {"2D", "Rect", NULL};

	tunePushMenu("SSAO");

	tuneEnum("Strength", &game_state.ambientStrength, strengthModes, strengthNames, &updateAmbientFeatures);
	tuneEnum("Resolution", &game_state.ambientResolution, resolutionModes, resolutionNames, NULL);
	tuneEnum("Blur", &game_state.ambientBlur, blurModes, blurNames, NULL);

	tuneEnum("Draw", &draw, drawModes, drawNames, NULL);
	tuneBoolean("Enable Blend", &enableBlend, NULL);
	tuneBoolean("Enable Fog", &enableFog, NULL);

	tuneEnum("Shader Optimization", &shaderOptimization, shaderOptimizationModes, shaderOptimizationNames, &reloadShaders);
	tuneCallback("Force Shader Reload", &reloadShaders);

	tuneEnum("Texture Target", &ssao.texTarget, texTargetModes, texTargetNames, NULL);

	tuneEnum("Draw Depth Test", &drawDepthTest, drawDepthTestModes, drawDepthTestNames, NULL);
	tuneFloat("Draw Depth", &drawDepth, 0.00001f, -1.0f, 1.0f, NULL);

	tuneEnum("Depth Reduction", &ssao.depthReduce, depthReduceModes, depthReduceNames, NULL);
	tuneInteger("Actual SSAO Downsample", &ssao.ambientDownsample, 0, LONG_MIN, LONG_MAX, NULL);
	tuneInteger("Actual Blur Downsample", &ssao.blurDownsample, 0, LONG_MIN, LONG_MAX, NULL);
	tuneInteger("Actual Depth Downsample", &ssao.depthDownsample, 0, LONG_MIN, LONG_MAX, NULL);
	
	tuneEnum("Screen Edge", &ssaoEdge, ssaoEdgeModes, ssaoEdgeNames, NULL);
	tuneFloat("Border Value", &border, 0.01f, 0.0f, 1.0f, NULL);

	tunePushMenu("Blur Sigma");

	tuneFloat("Spatial", &sigma[0], 0.001f, 0.0f, 10.0f, NULL);
	tuneFloat("Intensity", &sigma[1], 0.001f, 0.0f, 10.0f, NULL);
	tuneFloat("Depth", &sigma[2], 0.00001f, 0.0f, 1.0f, NULL);

	tunePopMenu();

	tunePushMenu("Occlusion Function");

	tuneFloat("Actual Near Plane", &game_state.nearz, 0.0f, FLT_MIN, FLT_MAX, NULL);
	tuneFloat("Actual Far Plane", &game_state.farz, 0.0f, FLT_MIN, FLT_MAX, NULL);
	tuneFloat("Near Limit", &nearFarLimit[0], 0.1f, 0.0f, 100.0f, NULL);
	tuneFloat("Far Limit", &nearFarLimit[1], 0.1f, 0.0f, 100.0f, NULL);

	tuneFloat("Pixel Spread", &pixelDepthSpread[0], 0.01f, 0.0f, 10.0f, NULL);
	tuneFloat("Depth Spread", &pixelDepthSpread[1], 0.0001f, 0.0f, 1.0f, NULL);
	tuneFloat("Pre Depth Scale", &preWeight[0], 0.001f, 0.0f, 1.0f, NULL);
	tuneFloat("Pre Adjust", &preWeight[1], 0.01f, 0.0f, 2.0f, NULL);
	tuneFloat("Pre Scale", &preWeight[2], 0.01f, 0.0f, 2.0f, NULL);
	tuneFloat("Pre Inverse", &preWeight[3], 0.00001f, 0.0f, 2.0f, NULL);
	tuneFloat("Post Scale", &ssao.postScale, 0.001f, 0.0f, 4.0f, NULL);

	tuneFloat("Post Power", &scene_info.ambientOcclusionWeight, 0.001f, 0.0f, 4.0f, NULL);
	tuneFloat("Actual Post Power", &ssao.postPower, 0.0f, FLT_MIN, FLT_MAX, NULL);

	tuneFloat("Fake Irradiance Scale", &ssao.irradianceScale, 0.001f, -1.0f, 10.0f, NULL);
	tuneBoolean("Fake Irradiance", &ssao.irradiance, NULL);

	tunePopMenu();

	tunePushMenu("Samples");

	tuneInteger("Sample Count", &ssao.sampleCount, 1, 1, sampleVectorSize, NULL);
	tuneInteger("Sample Random Seed", &sampleSeed, 1, 0, 65535, &setupSampleVectors);
	tuneFloat("Sample Min", &sampleMinMax[0], 0.01f, 0.0f, 1.0f, &setupSampleVectors);
	tuneFloat("Sample Max", &sampleMinMax[1], 0.01f, 0.0f, 1.0f, &setupSampleVectors);
	tuneInteger("Sample Lambda", &sampleLambda, 8, 0, 64, &setupSampleVectors);
	tuneInteger("Sample Iterations", &sampleIterations, 1, 0, 100, &setupSampleVectors);
	tuneCallback("Sample Plot", &plotSample);
	tuneCallback("Sample Vector Length Plot", &plotSampleLength);

	tuneInteger("Sphere Size", &sphereSize, 1, 0, maxSphereSize, &setupSamplingSphere);
	tuneInteger("Sphere Random Seed", &sphereSeed, 1, 0, 65535, &setupSamplingSphere);
	tuneFloat("Sphere Min", &sphereMinMax[0], 0.01f, 0.0f, 1.0f, &setupSamplingSphere);
	tuneFloat("Sphere Max", &sphereMinMax[1], 0.01f, 0.0f, 1.0f, &setupSamplingSphere);
	tuneInteger("Sphere Lambda", &sphereLambda, 1, 0, 100, &setupSamplingSphere);
	tuneInteger("Sphere Iterations", &sphereIterations, 1, 0, 100, &setupSamplingSphere);
	tuneCallback("Sphere Plot", &plotSphere);
	tuneCallback("Sphere Vector Length Plot", &plotSphereLength);

	tunePopMenu();

	tunePopMenu();
}

void rdrAmbientSetParamsDirect(RdrSsaoParams * params)
{	
	static bool ssaoInit = false;
	if (!ssaoInit) {
		if( params->strength == AMBIENT_OFF )
			return; // exiting game before ever initialized (happens with unsupported video cards)
		memset(&ssao, 0, sizeof(ssao));
		ssao.depthReduce = depthAverage;
		ssao.postScale = 2.0f;
		ssao.postPower = 1.0f;
		ssao.irradianceScale = 0.4f;
		ssao.texTarget = GL_TEXTURE_2D;

		// If non power of two support for tex2D is not reliable for this card, use texRECT instead
		if(!rdr_caps.supports_arb_np2 && rdr_caps.supports_arb_rect) {
			ssao.texTarget = GL_TEXTURE_RECTANGLE_ARB;
		}

		ssaoInit = true;
	}

	rdrBeginMarker(__FUNCTION__);

	ssao.strength = params->strength;
	ssao.blur = params->blur;
	ssao.postPower = params->sceneAmbientOcclusionWeight;
	ssao.curFbo = rdr_frame_state.numFBOs ? rdr_frame_state.curFrame % rdr_frame_state.numFBOs : 0;

	// If the option was turned off, delete the FBOs
	if (ssao.strength == AMBIENT_OFF) {
		deleteFbos();
		WCW_SetFogDeferring(0);
		rdrEndMarker();
		return;
	}

	// Force the sample count and irradiance features to match the strength setting
	switch (ssao.strength) {
		case AMBIENT_LOW: ssao.sampleCount = 16; ssao.irradiance = false; break;
		case AMBIENT_MEDIUM: ssao.sampleCount = 32; ssao.irradiance = false; break;
		case AMBIENT_HIGH: ssao.sampleCount = 64; ssao.irradiance = false; break;
		case AMBIENT_ULTRA: ssao.sampleCount = 64; ssao.irradiance = true; break;
		default: break;
	}

	// Update the sample constants if it has changed
	if (ssao.lastSampleCount != ssao.sampleCount) {
		setupSampleVectors();
		ssao.lastSampleCount = ssao.sampleCount;
	}

	// Update the FBOs if they need to be rebuilt
	if (params->width != ssao.screenWidth ||
		params->height != ssao.screenHeight ||
		params->ambientDownsample != ssao.lastParamAmbientDownsample ||
		params->blurDownsample != ssao.lastParamBlurDownsample ||
		params->depthDownsample != ssao.lastParamDepthDownsample ||
		ssao.irradiance != ssao.lastIrradiance ||
		ssao.texTarget != ssao.lastTexTarget ||
		rdr_frame_state.numFBOs != ssao.numFbo)
	{
		bool ret;

		if(ssao.lastTexTarget != ssao.texTarget) {
			// Reload the shaders
			ssao.cgContext = 0;
		}

		if(ssao.numFbo != rdr_frame_state.numFBOs) {
			deleteFbos();
		}

		ssao.numFbo = rdr_frame_state.numFBOs;
		ssao.screenWidth = params->width;
		ssao.screenHeight = params->height;
		ssao.lastParamAmbientDownsample = ssao.ambientDownsample = params->ambientDownsample;
		ssao.lastParamBlurDownsample = ssao.blurDownsample = params->blurDownsample;
		ssao.lastParamDepthDownsample = ssao.depthDownsample = params->depthDownsample;
		ssao.lastIrradiance = ssao.irradiance;
		ssao.lastTexTarget = ssao.texTarget;

		ret = initFbos();

		if(!ret && (ssao.texTarget == GL_TEXTURE_2D) && rdr_caps.supports_arb_rect) {
			ssao.lastTexTarget = ssao.texTarget = GL_TEXTURE_RECTANGLE_ARB;
			printf("initFbos failed, trying ARB_texture_rectangle\n");
			ret = initFbos();
		}

		if(!ret) {
			printf("initFbos failed, disabling SSAO\n");
			disableAmbient();
			deleteFbos();
		}
	}

	// Load the shaders if they have not been loaded yet
	if (!ssao.cgContext) {
		ssao.cgContext = rt_cgfxGetGlobalContext();

		reloadShaders();
		setupSamplingSphere();
	}

	if(isDevelopmentOrQAMode()) {
		registerDebugVariables();
	}

	WCW_SetFogDeferring(1);
	rdrEndMarker();
}

void rdrAmbientRenderDirect()
{
	rdrBeginMarker(__FUNCTION__);

	WCW_SetFogDeferring(0);

	if(ssao.strength) {
		drawAmbient();
	}

	rdrEndMarker();
	
	rdrNewFeature();
}

void rdrAmbientRenderDebugDirect()
{
}

void rdrHandleEffectReloadDirect( const char* szEffectName )
{
	if ( stricmp( szEffectName, kSSAO_EffectName ) == 0 )
	{
		reloadShaders();
	}
}

// TODO demo hack
#include "newfeature.c"
