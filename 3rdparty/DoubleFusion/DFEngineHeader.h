#ifndef _H_DFENGINEHEADER_H
#define _H_DFENGINEHEADER_H

/** @file DFEngineHeader.h
 *
 *	(C) Copyright 2004-2007 DoubleFusion LTD. All rights reserved.
 *	DOUBLE FUSION PROPRIETARY INFORMATION
 *
 */

/** \mainpage Double Fusion SDK
 *	<IMG src="dflogo.jpg"><span style='font-size:10.0pt;font-family:Arial'>
 *	Welcome to the Double Fusion SDK. This document	along with the sample code projects
 *  provided with the SDK, contains all the necessery information you need in order to
 *  effectively integrate the DF SDK with your application.
 *
 *	Note that this document is actively updated so make sure to check it whenever you receive
 * 	a new version of the Double Fusion SDK.
 *
 *	Also, we appreciate any feedback you can give us regarding the DF Engine API or the
 *  SDK documentation.
 *	For questions or feedback you can contact us by email at: support@doublefusion.com</span>
 */


#ifndef DF_DONOT_DEFINE_TYPES
#define DF_DONOT_DEFINE_TYPES

#ifdef _WIN32
#pragma once
#endif

#ifdef _WIN32
#define DF_WINDOWS
#ifdef _DEBUG
#define DF_DEBUG
#endif
#elif defined PS2
#define DF_PS2
#elif defined(__CELLOS_LV2__)
#define DF_PS3
#ifdef _DEBUG
#define DF_DEBUG
#endif
#endif

// Basic types

typedef signed char      DF_Int8;
typedef signed short     DF_Int16;
typedef signed int		 DF_Int32;

#ifdef _WIN32
#ifdef _MSC_VER
typedef signed __int64   DF_Int64;
#endif
#elif (defined _LINUX || defined LINUX)
typedef long long	DF_Int64;
#elif defined(DF_PS2) || defined (DF_PS3)
typedef long long DF_Int64;
#else
#error "Unsupported Platform."
#endif

typedef unsigned char      DF_UInt8;
typedef unsigned short     DF_UInt16;
typedef unsigned int	   DF_UInt32;

#ifdef _WIN32
#ifdef _MSC_VER
typedef unsigned __int64   DF_UInt64;
// path and file names are wide char on PC
typedef wchar_t	DF_PathChar;
#endif
#elif (defined _LINUX || defined LINUX)
typedef unsigned long long DF_UInt64;
#elif defined(DF_PS2) || defined (DF_PS3)
typedef unsigned long long DF_UInt64;
// path and file names are ascii on PS
typedef char	DF_PathChar;
#else
#error "Unsupported Platform."
#endif


typedef float  DF_Float32;
typedef double DF_Float64;

typedef unsigned int DF_UInt;

typedef DF_UInt32	DF_Time;

// error code returned from most DF functions
typedef long DF_RESULT;


/**
 *  Return error codes
 */
typedef enum
{
	DF_OK = 0, 										/**< Success */
	DF_ERROR = -1, 									/**< General error */
	DF_ERROR_NOTSTARTED = -2,						/**< Engine not started */
	DF_ERROR_OPENFILE = -3,							/**< Failed to open file */
	DF_ERROR_WRONG_VERSION = -4,					/**< Header does not match engine version */
	DF_ERROR_ALREADY_STARTED = -5,					/**< Engine already started */
	DF_ERROR_INVALID_OBJECT_ID = -6,				/**< Invalid object id */
	DF_ERROR_INVALID_ZONE_ID = -7,					/**< Invalid zone id */
	DF_ERROR_INVALID_ROOT_FOLDER = -8,				/**< Invalid root folder */
	DF_ERROR_NOT_IMPLEMENTED = -9,					/**< method not implemented */
	DF_ERROR_USER_SHUTDOWN = -10,
	DF_ERROR_LOGIN_COMM = -11,
	DF_ERROR_LOGIN_SERVER = -12,
	DF_OK_LOGGING_TO_SERVER = 1,
	DF_ERROR_CREATIVE_DL_COMM = -13,
	DF_ERROR_CREATIVE_DL_SERVER = -14,
	DF_OK_CREATIVE_DOWNLOADING = 2,
	DF_ERROR_TRACKING_COMM = -15,
	DF_ERROR_TRACKING_SERVER = -16,
	DF_OK_TRACKING_SENDING = 3,
	DF_ERROR_TRACKING_NODATA = -17,
	DF_ERROR_TRACKING_NOSESSION = -18,
	DF_ERROR_OUT_OF_MEMORY = -19,					/**< Memory allocation failed */
	DF_ERROR_SOUND_ALREADY_INITIALIZED = -20,		/**< Sound system already initialized */
	DF_ERROR_SOUND_UNKNOWN_TYPE = -21,				/**< Sound system error */
	DF_ERROR_INVALID_PROPERTY = -22,				/**< Invalid engine property */
	DF_ERROR_CANNOT_RELEASE_CREATIVE = -23,
	DF_ERROR_CREATIVE_WAS_RELEASED = -24,			/**< Creative was already released */
	DF_ERROR_EXCEPTION = -25,						/**< An handled exception occurred */
	DF_ERROR_ACTION_NOT_SUPPORTED = -26,			/**< Action not supported */
	DF_ERROR_ALREADY_DOWNLOADING = -27,				/**< Creative are already downloading from server */
	DF_ERROR_TRACKING_OBJECT_HAS_NO_CREATIVES = -28,
	DF_ERROR_INVALID_PARAM = -29,					/**< Invalid parameter */
	DF_ERROR_NOT_PROFILING = -30,					/**< This action cannot be performed when not in profiling mode */
	DF_ERROR_CREATIVE_FILE_NOT_FOUND = -31,			/**< Creative file not found */
	DF_ERROR_ALREADY_IN_ZONE = -32,					/**< Zone already started */
	DF_ERROR_WRONG_THREAD = -33,					/**< Method should only be called from the main thread (from which the Start() method was called) */
	DF_ERROR_INVALID_TEXTURE_LEVEL = -34,			/**< Invalid texture level */
	DF_ERROR_UNSUPPORTED_CREATIVE_TYPE = -35,		/**< Creative file type is unsupported */
} DFErrorCode;


/**
 *  Success return values
 */
#define DF_SUCCESS(x) ((x)>=0)
/**
 *  Error return values
 */
#define DF_FAIL(x) ((x)<0)

typedef unsigned int dfsize_t;

// memory alloc / free handlers prototypes
typedef void* (*DFMemoryAllocHandler)(dfsize_t);
typedef void (*DFMemoryFreeHandler)(void*);

#ifndef NULL
#define NULL 0
#endif

//////////////////////////////////////////////////////////////////
// Enums
//////////////////////////////////////////////////////////////////

/**
 *  Double Fusion Object types
 */
typedef enum
{
	DFT_TEXTURE	 = 0, 	/**< Texture object */
	DFT_3DMODEL	 = 1,	/**< 3d object */
	DFT_SOUND	 = 2,	/**< Sound object */
	DFT_TRACKING = 3	/**< Tracking object */
}DFObjectType;

/**
 *  Event types
 */
typedef enum
{
	DFE_VISIBLE		= 0,	/**< Visible event */
	DFE_COLLISION	= 1,	/**< Collision event */
	DFE_CLICK		= 2,	/**< Click event */
	DFE_PROXIMITY	= 3,	/**< Proximity event */
	DFE_ACTIVE		= 4,	/**< Active event */
	DFE_COUNT		= 5		/**< Count event */
}DFEventType;

/**
 *  Double Fusion Texture Format.
 *  All formats are listed from left to right,
 *  most-significant bit to least-significant bit.
 *  For example, DFTF_A8R8G8B8 is ordered from the most-significant bit channel
 *  A (alpha), to the least-significant bit channel B (blue).
 *  When traversing surface data, the data is stored in memory from least-significant
 *  bit to most-significant bit, which means that the channel order in
 *  memory is from least-significant bit (blue) to most-significant bit (alpha).
 */
typedef enum
{
	  DFTF_R8G8B8		= 0,	/**< R8B8G8 */
	  DFTF_A8R8G8B8		= 1,	/**< DFTF_A8R8G8B8 */
	  DFTF_X8R8G8B8		= 2,	/**< DFTF_X8R8G8B8 */
	  DFTF_B8G8R8		= 3,	/**< DFTF_B8G8R8 */
	  DFTF_A8B8G8R8     = 4,	/**< DFTF_A8B8G8R8 */
	  DFTF_DXT1			= 5,	/**< DFTF_DXT1 */	
	  DFTF_DXT3			= 6,	/**< DFTF_DXT3 */	 
	  DFTF_DXT5			= 7,	/**< DFTF_DXT5 */
	  DFTF_CUSTOM		= 8		/**< DFTF_CUSTOM */
} DFTextureFormat;

/**
 *  Audio modes
 */
typedef enum
{
      DFAM_AUDIO_NONE		= 0,	/**< No Audio */
      DFAM_AUDIO_NORMAL		= 1,	/**< Normal Audio */
      DFAM_AUDIO_3D			= 2		/**< 3D Audio */
} DFAudioMode;

/**
 *  Sound device types
 */
typedef enum
{
	DFS_DIRECT_SOUND = 0	/**< Direct Sound */
} DFSoundDeviceType;

/**
 *  Creative states
 */
typedef enum
{
	DFS_STATIC		= 0,	/**< Static Creative */
	DFS_PLAYING		= 1,	/**< Creative is playing */
	DFS_STOPPED		= 2,	/**< Creative is stopped */
	DFS_FINISHED	= 3,	/**< Creative finished playing */
	DFS_UNKNOWN		= 4,	/**< Unknown creative state */
	DFS_LOADING		= 5,	/**< Streaming creative is still loading. Check state again later */
	DFS_FAILED		= 6		/**< Streaming creative has failed loading */
} DFCreativeState;

/**
 * Working modes for the engine
 */
typedef enum
{
	DFEM_FULL			= 0,		/**< Full working mode */
	DFEM_TRACKING_ONLY	= 1,		/**< Only perform tracking, does not download campaigns
									and does not show already downloaded campaigns */
	DFEM_ADS_ONLY		= 2,		/**< Only downloads campaigns, does not report tracking*/
	DFEM_NONE			= 3			/**< Only shows default creatives, no tracking */
} DFEngineMode;

/**
 *  Object properties
 */
typedef enum
{
	//  Property            // Expected type //
	///////////////////////////////////////////
	DFOP_LOOP				= 0, 	/**< Loop animated creatives DF_UInt8 (0 / 1) */
	DFOP_SOUND_VOLUME		= 1,	/**< Sound volume DF_UInt8 (0 ... 100) */
	DFOP_PLAY_STATE			= 2,  	/**< Creative state (DFCreativeState) */
	DFOP_SOUND_PAN			= 3,	/**< Stereo pan DF_Int8 (-100 ... 100) left to right */
	DFOP_SOUND_3D_PARAMS	= 4		/**< Sound 3d params - type depends on sound system */
} DFObjectProperty;


/**
 *  Object actions
 */
typedef enum
{
	DFOA_START_MOVIE = 0,	/**< Start playing movie on next Update */
	DFOA_PAUSE_MOVIE = 1,	/**< pause movie */
	DFOA_RESET_MOVIE = 2,	/**< Stop and rewind movie */
	DFOA_CLICK_AD    = 3	/**< For clickable ads */
} DFObjectAction;

/**
*  Content delivery mode
*/
typedef enum
{
	DFCDM_MEMORY		= 0,		/**< The object's content comes directly from memory. */
	DFCDM_FILE			= 1			/**< The object's content comes from file on disk. */
} DFContentDeliveryMode;

/**
 *  Flags for DFActionClickParams
 */
#define DFAC_CLICK_CENTER (1L << 0)		/**< Position click on center of ad */

/**
 *  Params for DFOA_CLICK_AD action
 */
typedef struct DFActionClickParams
{
	DF_UInt32 m_nXCoord;	/**< X coordinate (in pixels) where the click has occurred */
	DF_UInt32 m_nYCoord;	/**< Y coordinate (in pixels) where the click has occurred */
	DF_UInt32 m_nFlags;		/**< Flags */
} DFActionClickParams;

//////////////////////////////////////////////////////////////////
// Data structures
//////////////////////////////////////////////////////////////////

/**
 *  Visibility Event parameters
 */
typedef struct DFEventVisibleParams
{
	DF_Float32	m_fVisPercentage; /**< Percentage of the object from game window in the range of [0,100].
								  (i.e if the object bounding rect in screen space is
								  [top: 40, left: 40, bottom: 140, right: 240] and the game window rendering area
								  is 1024x768 piexels then the visible percentage
								  should be [Object's area / Game window area] * 100 =
								  (100 * 200 / 1024*768) which is about 2.54%*/

	DF_Float32	m_fVisAngle;	/**< Angle between the object's NEGATIVE normal and the camera's look at vector 
								in degrees. This parameter relevant just for DFObject of type DFT_TEXTURE.
								If the camera look at vector is (0,0,1) and DFObject with look at vector of (0,0,-1), 
								the angle is simply acos(dot((0,0,1), (0,0,1))) = acos(1) = 0rad = 0deg */

}DFEventVisibleParams;

/**
 *  Memory allocator
 */
typedef struct DFMemoryAllocator
{
	DFMemoryAllocHandler	fMemAllocHandler;		/**< Memory allocation handler */
	DFMemoryFreeHandler		fMemFreeHandler;		/**< Memory free handler*/
	DFMemoryAllocHandler	fTempMemAllocHandler;	/**< Temporary memory allocation handler */
	DFMemoryFreeHandler		fTempMemFreeHandler;	/**< Temporary memory free handler */
} DFMemoryAllocator;

/**
 *  Creative information
 */
typedef struct DFCreativeInfo
{
	bool		m_bIsDefault;	/**< Is this creative a default creative */
	const char* m_pUrl;			/**< URL */
	const char* m_pName;		/**< Name */
	const char* m_pText;		/**< Text */
	const char* m_pID;			/**< Creative ID */
}DFCreativeInfo;

/**
*  Texture information
*/
typedef struct DFTextureInfo
{
	DFTextureFormat	m_eFormat;		/**< Texture format */
	DF_UInt32		m_nWidth;		/**< Texture width in pixels */
	DF_UInt32		m_nHeight;		/**< Texture height in pixels */
	DF_UInt32		m_nPitch;		/**< Texture pitch in bytes */
	bool			m_bUseDynamic;	/**< Indicates texture data might change per frame */
} DFTextureInfo;

/**
* Targeting criterion structure.
* A targeting criterion allows filtering Ads/Tracking by unique definitions.
* i.e Targeting criterion of m_lpszKey = 'age' and m_lpszValues = '20-30' will limit
* the engine to use only creatives that were defined with this targeting criterion.
* Multiple values per key are allowed and are separated by ;
* For example: Value1;Value2;Value3
**/
typedef struct TargetingCriterion
{
	const char*		m_lpszKey;
	const char*		m_lpszValues;
} TargetingCriterion;

#define DFCRITERION_END() {NULL, NULL}

/**
* Start parameters flags
**/
typedef enum 
{
	DFSPF_DOWNLOAD_ADINFO		= (1L << 0),	/**<  Get ads campaign from server upon startup. Connection is blocking */
	DFSPF_ALWAYS_SEND_TRACKING	= (1L << 1),	/**<  Internal use */
	DFSPF_REUSE_INSTANCE_ID		= (1L << 2)		/**<  Internal use */
} DFStartFlags;

/**
* Engine start parameters
**/
typedef struct DFEngineParams
{
	DF_PathChar*				m_pRootDir;			/**<  The absolute path to the game's DF root folder */
	DF_PathChar*				m_pUserRootDir;		/**<  The absolute path to the user's DF root folder, or NULL if it's the same as m_pRootDir */
	DFMemoryAllocator*			m_pMemoryAllocator;	/**<  The memory handlers to use, or NULL for default memory handlers */
	DFEngineMode				m_eEngineMode;		/**<  The mode in which the engine should run */
	char*						m_pGameId;			/**<  For PS use only (Windows users should use NULL) */
	char*						m_pPlayerUniqueId;	/**<  Unique player ID if exists, or NULL otherwise */
	TargetingCriterion*			m_pTargetingRule;	/**<  Optional targeting rule of current game instance.
														  Targeting rule is defined using an array of TargetingCriterion structures.
														  Use DFCRITERION_END to mark the last element of the array*/
	DF_UInt32					m_nFlags;			/**<  Combination of zero or more of DFStartFlags */

} DFEngineParams;

/**
 * Instance tracking information.
 * Note that visibility reporting is not counted if one of the following
 * conditions was met:\n
 * 1. The size didn't reach the minimum size threshold assigned
 *    to this creative (default: 2.0%).\n
 * 2. The angle passed the maximum angle threshold assigned to this creative
 *    (default: 65 degrees).\n
 * 3. The time span passed since the beginning of the current exposure
 *    is less than the minimum bound set for this creative (default: 0.5 second).
 * 4. The IDFObject::UpdateOnEvent method that used to pass the visibility information was not
 *    called in the same frequency that IDFEngine::Update method called.
 */
typedef struct DFInstanceTrackingInfo
{
	// Current frame's visibility details:
	bool		m_bIsVisible;		/**<  Is this instance currently visible */
	DF_Float32	m_fCurFrameSize;	/**<  Current reported size for this instance */
	DF_Float32	m_fCurFrameAngle;	/**<  Current reported angle for this instance */

	// Accumulated information of this instance (since its creation)
	DF_UInt32	m_nExposures;		/**<  Number of exposures */
	DF_Float32	m_fTimeExposed;		/**<  Total time exposed */
	DF_Float32	m_fAvgSize;			/**<  Average reported size */
	DF_Float32	m_fAvgAngle;		/**<  Average reported angle */
	DF_UInt32	m_nInteractions;	/**<  Number of collisions, clicks and counts */

} DFInstanceTrackingInfo;

#endif /*DF_DONOT_DEFINE_TYPES*/

//////////////////////////////////////////////////////////

#ifdef _WIN32
// Used for dll exporting and importing
#define  DLL_EXPORT   extern "C" __declspec( dllexport )
#define  DLL_IMPORT   extern "C" __declspec( dllimport )

// Can't use extern "C" when DLL exporting a class
#define  DLL_CLASS_EXPORT   __declspec( dllexport )
#define  DLL_CLASS_IMPORT   __declspec( dllimport )

// Can't use extern "C" when DLL exporting a global
#define  DLL_GLOBAL_EXPORT   extern __declspec( dllexport )
#define  DLL_GLOBAL_IMPORT   extern __declspec( dllimport )

#define DF_STDMETHODCALLTYPE			__stdcall
#elif (defined _LINUX || defined LINUX)

// Used for dll exporting and importing
#define  DLL_EXPORT   extern "C"
#define  DLL_IMPORT   extern "C"

// Can't use extern "C" when DLL exporting a class
#define  DLL_CLASS_EXPORT
#define  DLL_CLASS_IMPORT

// Can't use extern "C" when DLL exporting a global
#define  DLL_GLOBAL_EXPORT   extern
#define  DLL_GLOBAL_IMPORT   extern

#define DF_STDMETHODCALLTYPE

#elif defined DF_PS2

// Used for dll exporting and importing
#define  DLL_EXPORT   extern "C"
#define  DLL_IMPORT   extern "C"

// Can't use extern "C" when DLL exporting a class
#define  DLL_CLASS_EXPORT
#define  DLL_CLASS_IMPORT

// Can't use extern "C" when DLL exporting a global
#define  DLL_GLOBAL_EXPORT   extern
#define  DLL_GLOBAL_IMPORT   extern

#define DF_STDMETHODCALLTYPE

#elif defined DF_PS3

// Used for dll exporting and importing
#define  DLL_EXPORT   
#define  DLL_IMPORT   

// Can't use extern "C" when DLL exporting a class
#define  DLL_CLASS_EXPORT
#define  DLL_CLASS_IMPORT

// Can't use extern "C" when DLL exporting a global
#define  DLL_GLOBAL_EXPORT   
#define  DLL_GLOBAL_IMPORT

#define DF_STDMETHODCALLTYPE

#else
#error "Unsupported Platform."
#endif



//////////////////////////////////////////////////////////////////
// SDK Version
//////////////////////////////////////////////////////////////////

#define MAJOR_VERSION	3
#define MINOR_VERSION	5

#define DF_ENGINE_SDK_VERSION	((   MAJOR_VERSION   <<24)+(MINOR_VERSION   <<16)+(0    <<8) + 0)



//////////////////////////////////////////////////////////////////
// Interfaces
//////////////////////////////////////////////////////////////////

/* ----------------------------------------------------------------------
 *  Define Classes with C and C++ versions
 *
 *  C++ version is default
 *  define DFENGINE_C_INTERFACE before including this header in order to compile using the C version
 * ---------------------------------------------------------------------- */
#if !defined(__cplusplus) || defined(DFENGINE_C_INTERFACE)
#define DF_DECLARE_INTERFACE(iface)     typedef struct iface { \
										struct iface##Vtbl* lpVtbl; \
										} iface; \
											typedef struct iface##Vtbl iface##Vtbl; \
										struct iface##Vtbl \

#define DF_METHOD_(type,method)			type (DF_STDMETHODCALLTYPE* method)
#define DF_METHOD(method)				DF_RESULT (DF_STDMETHODCALLTYPE* method)
#define DF_THIS_						DF_INTERFACE* This,
#define DF_THIS							DF_INTERFACE* This
#define DF_PURE
#define DF_CONST_PURE
#else
#define DF_DECLARE_INTERFACE(iface)     struct iface
#define DF_METHOD_(type,method)			virtual type DF_STDMETHODCALLTYPE method
#define DF_METHOD(method)				virtual DF_RESULT DF_STDMETHODCALLTYPE method
#define DF_THIS_
#define DF_THIS
#define DF_PURE							= 0
#define DF_CONST_PURE					const = 0
#endif

#ifdef DFENGINE_EXPORTS
	#define DFENGINE_INTERFACE	DLL_EXPORT
#else
	#define DFENGINE_INTERFACE	DLL_IMPORT
#endif

///////////////////////////////////////////////////////////////////////////////
// Purpose: Interface to the DF Object
///////////////////////////////////////////////////////////////////////////////

#undef  DF_INTERFACE
#define DF_INTERFACE IDFObject
/**
 *  Interface to the DF Object.
 */
DF_DECLARE_INTERFACE(IDFObject)
{
	/**
	 *  Get the current creative absolute file name.
	 *	Get the absolute file path of the currently assigned creative
	 *	@param[out] pPath A buffer to receive the creative path
	 *	@param bufsize The buffer size
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(GetAbsoluteLocalFileName)(DF_THIS_ DF_PathChar* pPath, DF_UInt32 bufsize) DF_PURE;

	/**
	 *  Determines if creative data associated with this object has changed from the last frame.
	 *	Objects that display animated creatives or rotate creatives on the fly may need to update the texture information several times a second.
	 *	In order to optimize texture update, use this method to determine if the texture data
	 *  was changed from the last frame and therefore needs to be updated.
	 *	@return true if creative data changed, false otherwise.
	 */
	DF_METHOD_(bool, IsCreativeDataChanged)(DF_THIS) DF_CONST_PURE;

	/**
	*   Update the object on event.
	*	The method is used to inform the DF engine on events such as visibility or interaction
	*	For visibility events, it is important to call this method on EVERY frame that the object is visible.
	*	For a more detailed information about events reporting see the <A HREF="ReportingEvents.htm">Reporting events</a> section.
	*	@see DFEventType for a list of supported event types
	*	@see DFEventVisibleParams for visibility event parameters	
	*	@param event The event type
	*	@param pEventData The event data
	*	@return No return value
	*/
	DF_METHOD_(void, UpdateOnEvent)(DF_THIS_ DFEventType event, void* pEventData) DF_PURE;

	/**
	*   Copy the texture of current frame to the given destination buffer.
	*	@see IDFObject::IsCreativeDataChanged - used to determine if the texture data
	*	have been changed since the last frame.
	*   @param[in,out] pDstBuffer Address of the destination memory buffer.
	*   @param[in] nLevel Texture level to copy.
	*   @param[in] nWidth Width of the destination buffer in pixels: Should be equal to object's texture width.
	*   @param[in] nHeight Height of the destination buffer in pixels: Should be equal to object's texture height.
	*   @param[in] nPitch Pitch of the destination buffer in bytes. Should be equal or bigger than the object's texture pitch.
	*   @param[in] nFlags Optional copy flags.
	*	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	*/
	DF_METHOD(CopyTexture)(DF_THIS_ void* pDstBuffer, DF_UInt32 nLevel, DF_UInt32 nWidth, DF_UInt32 nHeight, DF_UInt32 nPitch, DF_UInt32 nFlags) DF_PURE;

	/**
	*   Get the number of mip levels of the currently assigned texture creative.
	*	@return Level count of the texture.
	*/
	DF_METHOD_(DF_UInt32, GetTextureLevelCount)(DF_THIS) DF_CONST_PURE;

	/**
	*   Get information of the given texture level.
	*   @param[in] nLevel Texture level.
	*	@param[out] pTextureInfo The texture information.
	*	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	*/
	DF_METHOD(GetTextureInfo)(DF_THIS_ DF_UInt32 nLevel, DFTextureInfo* pTextureInfo) DF_CONST_PURE;

	/**
	*   Get the creative data associated with this object.
	*	Use this method only for object that is set to Custom memory format.
	*	@param[out] ppData Pointer to the custom data.
	*   @param[out] pSizeInBytes Will hold size in bytes of the custom data.
	*	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	*/
	DF_METHOD(GetCustomData)(DF_THIS_ void** ppData, DF_UInt32* pSizeInBytes) DF_PURE;

	/**
	 *  Get information about the currently assigned creative.
	 *	The information about the creative can include Advertiser's name, Advertiser's URL and free text describing the creative
	 *	@param[out] pCreativeInfo The creative information
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(GetCreativeInfo)(DF_THIS_ DFCreativeInfo* pCreativeInfo) DF_PURE;


	/**
	*  Get this instance's tracking info.
	*  This info is accumulated since the creation of this instance.
	*  This method will only work when Profiling is enabled.
	*
	*  @param[out] pTrackingInfo The tracking info of this instance
	*  @return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	*/
	DF_METHOD(GetTrackingInfo)(DF_THIS_ DFInstanceTrackingInfo* pTrackingInfo) DF_CONST_PURE;

#ifdef DF_PS2
	DF_METHOD(StartCreativeDownload)(DF_THIS) DF_PURE;
	DF_METHOD(ShutdownCreativeDownload)(DF_THIS) DF_PURE;
	DF_METHOD(CheckCreativeDownloadStatus)(DF_THIS_ DF_RESULT* pStatus) DF_PURE;
#endif

	/**
	 *  Perform an action on the object
	 *	Actions are mostly related to texture objects that display animated creatives (movies).
	 *	You can control movie playback by starting, pausing or resetting movie playback.
	 *	For more information see the <A HREF="UsingVideo.htm">Using Video</a> section.
	 *	@see DFObjectAction - Action types
	 *	@param action The action to perform
	 *	@param params The action parameters
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(PerformAction)(DF_THIS_ DFObjectAction action, const void* params) DF_PURE;

	/**
	 *  Get the object's Id
	 *	The Object Id is a number, automatically generated by the Developer Suite, uniquely identifying the object.
	 *	@return The object's Id
	 */
	DF_METHOD_(const char*, GetId)(DF_THIS) DF_PURE;

	/**
	 *  Get the object's Instance Id
	 *	@return The object's Instance Id
	 */
	DF_METHOD_(DF_UInt32, GetInstanceId)(DF_THIS) DF_PURE;

	/**
	 *  Get the object's type
	 *	@see DFObjectType
	 *	@return The object's type
	 */
	DF_METHOD_(DFObjectType, GetType)(DF_THIS) DF_CONST_PURE;

	/**
	 *  Get the object's name
	 *	@return The object's name
	 */
	DF_METHOD_(const char*, GetName)(DF_THIS) DF_CONST_PURE;

	/**
	 *  Create a new instance for this object.
	 *	Calling this method is identical to calling IDFEngine::CreateDFObject multiple times with the same object name
	 *	@see IDFEngine::CreateDFObject
	 *	@return A pointer to the new object instance
	 */
	DF_METHOD_(IDFObject*, CreateInstance)(DF_THIS) DF_PURE;

	/**
	 *  Release the currently assigned creative.
	 *	Decrease the reference count of the currently assigned creative. If the reference count reaches 0
	 *  the creative is released and removed from memory.
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(ReleaseCurrentCreative)(DF_THIS) DF_PURE;

	/**
	 *  Refresh the selected creative for this object.
	 *  Calling this method assigns a new creative for the object.
	 *	If the creative was not previously in memory it will be loaded from disk.
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(RefreshCurrentCreative)(DF_THIS) DF_PURE;

	/**
	 *  Set an object property
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(SetProperty)(DF_THIS_ DFObjectProperty prop, const void* pValue) DF_PURE;

	/**
	 *  Get an object property
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(GetProperty)(DF_THIS_ DFObjectProperty prop, void* pValue) DF_PURE;

	/**
	*  Set the object's axis aligned bounding box in it's local space.
	*  Thie methoed is used in order to track the object's visibility. For more information please refer to the <A HREF="ReportingEvents.htm">Reporting events</a> section.
	*  @param vBoxMin The minimum extents values of the bounding box.
	*  @param vBoxMax The maximum extents values of the bounding box.
	*/
	DF_METHOD_(void, SetLocalBoundingBox)(DF_THIS_ const float vBoxMin[3], const float vBoxMax[3]) DF_PURE;

	/**
	*  Set the object's look at direction in it's local space.
	*  Thie methoed is used in order to track the object's visibility. For more information please refer to the <A HREF="ReportingEvents.htm">Reporting events</a> section.
	*  @param vLookAt The look at direction of the object in local space.
	*/
	DF_METHOD_(void, SetLocalLookAt)(DF_THIS_ const float vLookAt[3]) DF_PURE;

	/**
	*	Set the current world matrix of this object.
	*  Thie methoed is used in order to track the object's visibility. For more information please refer to the <A HREF="ReportingEvents.htm">Reporting events</a> section.
	*   @param matWorld The world matrix in row major form.
	*/
	DF_METHOD_(void, SetWorldMatrix)(DF_THIS_ const float matWorld[16]) DF_PURE;

	/**
	*  Get the object's content delivery mode.
	*  Object can use either an In Memory or an On Disk delivery mode.
	*  @return The object's delivery mode.
	*/
	DF_METHOD_(DFContentDeliveryMode, GetContentDeliveryMode)(DF_THIS) DF_CONST_PURE;
};

#if !defined(__cplusplus) || defined(DFENGINE_C_INTERFACE)
#define IDFObject_GetAbsoluteLocalFileName(p, a, b) (p)->lpVtbl->GetAbsoluteLocalFileName(p, a, b)
#define IDFObject_IsCreativeDataChanged(p) (p)->lpVtbl->IsCreativeDataChanged(p)
#define IDFObject_UpdateOnEvent(p, a, b) (p)->lpVtbl->UpdateOnEvent(p, a, b)
#define IDFObject_CopyTexture(p, a, b, c, d, e, f) (p)->lpVtbl->CopyTexture(p, a, b, c, d, e, f)
#define IDFObject_GetTextureLevelCount(p) (p)->lpVtbl->GetTextureLevelCount(p)
#define IDFObject_GetTextureInfo(p, a, b) (p)->lpVtbl->GetTextureInfo(p, a, b)
#define IDFObject_GetCustomData(p, a, b) (p)->lpVtbl->GetCustomData(p, a, b)
#define IDFObject_GetCreativeInfo(p, a) (p)->lpVtbl->GetCreativeInfo(p, a)
#define IDFObject_GetTrackingInfo(p, a) (p)->lpVtbl->GetTrackingInfo(p, a)
#define IDFObject_PerformAction(p, a, b) (p)->lpVtbl->PerformAction(p, a, b)
#define IDFObject_GetId(p) (p)->lpVtbl->GetId(p)
#define IDFObject_GetInstanceId(p) (p)->lpVtbl->GetInstanceId(p)
#define IDFObject_GetType(p) (p)->lpVtbl->GetType(p)
#define IDFObject_GetName(p) (p)->lpVtbl->GetName(p)
#define IDFObject_CreateInstance(p) (p)->lpVtbl->CreateInstance(p)
#define IDFObject_ReleaseCurrentCreative(p) (p)->lpVtbl->ReleaseCurrentCreative(p)
#define IDFObject_RefreshCurrentCreative(p) (p)->lpVtbl->RefreshCurrentCreative(p)
#define IDFObject_SetProperty(p, a, b) (p)->lpVtbl->SetProperty(p, a, b)
#define IDFObject_GetProperty(p, a, b) (p)->lpVtbl->GetProperty(p, a, b)
#define IDFObject_SetLocalBoundingBox(p, a, b) (p)->lpVtbl->SetLocalBoundingBox(p, a, b)
#define IDFObject_SetLocalLookAt(p, a) (p)->lpVtbl->SetLocalLookAt(p, a)
#define IDFObject_SetWorldMatrix(p, a) (p)->lpVtbl->SetWorldMatrix(p, a)
#define IDFObject_GetContentDeliveryMode(p) (p)->lpVtbl->GetContentDeliveryMode(p)
#else
#define IDFObject_GetAbsoluteLocalFileName(p, a, b) (p)->GetAbsoluteLocalFileName(a, b)
#define IDFObject_IsCreativeDataChanged(p) (p)->IsCreativeDataChanged()
#define IDFObject_UpdateOnEvent(p, a, b) (p)->UpdateOnEvent(a, b)
#define IDFObject_CopyTexture(p, a, b, c, d, e, f) (p)->CopyTexture(a, b, c, d, e, f)
#define IDFObject_GetTextureLevelCount(p) (p)->GetTextureLevelCount()
#define IDFObject_GetTextureInfo(p, a, b) (p)->GetTextureInfo(a, b)
#define IDFObject_GetCustomData(p, a, b) (p)->GetCustomData(a, b)
#define IDFObject_GetCreativeInfo(p, a) (p)->GetCreativeInfo(a)
#define IDFObject_GetTrackingInfo(p, a) (p)->GetTrackingInfo(a)
#define IDFObject_PerformAction(p, a, b) (p)->PerformAction(a, b)
#define IDFObject_GetId(p) (p)->GetId()
#define IDFObject_GetInstanceId(p) (p)->GetInstanceId()
#define IDFObject_GetType(p) (p)->GetType()
#define IDFObject_GetName(p) (p)->GetName()
#define IDFObject_CreateInstance(p) (p)->CreateInstance()
#define IDFObject_ReleaseCurrentCreative(p) (p)->ReleaseCurrentCreative()
#define IDFObject_RefreshCurrentCreative(p) (p)->RefreshCurrentCreative()
#define IDFObject_SetProperty(p, a, b) (p)->SetProperty(a, b)
#define IDFObject_GetProperty(p, a, b) (p)->GetProperty(a, b)
#define IDFObject_SetLocalBoundingBox(p, a, b) (p)->SetLocalBoundingBox(a, b)
#define IDFObject_SetLocalLookAt(p, a) (p)->SetLocalLookAt(a)
#define IDFObject_SetWorldMatrix(p, a) (p)->SetWorldMatrix(a)
#define IDFObject_GetContentDeliveryMode(p) (p)->GetContentDeliveryMode()
#endif

///////////////////////////////////////////////////////////////////////////////
// Interface to the DF engine
///////////////////////////////////////////////////////////////////////////////


#undef  DF_INTERFACE
#define DF_INTERFACE IDFEngine
/**
 *  Interface to the DF engine.
 */
DF_DECLARE_INTERFACE(IDFEngine)
{
	/**
	 *  Start the DoubleFusion Engine.
	 *	@param nSdkVersion The DF engine version. Should always be DF_ENGINE_SDK_VERSION
	 *	@param pEngineParams The parameters to be transfered to the engine.
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(Start)(DF_THIS_ DF_UInt32 nSdkVersion, const DFEngineParams* pEngineParams) DF_PURE;

	/**
	 *  Shutdown the DoubleFusion Engine.
	 *	@return No return value
	 */
	DF_METHOD_(void, ShutDown)(DF_THIS) DF_PURE;

	/**
	 *  Set the sound system the DF engine will use.
	 *	@param type The sound system to use. Right now only DFS_DirectSound is supported.
	 *	@param pDevice The sound device to use. When using Direct Sound you should pass a pointer to the IDirectSound interface
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(SetSoundSystem)(DF_THIS_ DFSoundDeviceType type, void* pDevice) DF_PURE;

	/**
	 *  Create a DF object.
	 *	Subsequent calls with the same object name will create multiple instances of the same object.
	 *	@param pObjectName The name of the DF object to create
	 *	@param[out] pDFObject A pointer to the created object.
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
#ifdef DF_PS2
	DF_METHOD(CreateDFObject)(DF_THIS_ const char* pObjectName, const char* pObjectId, IDFObject** pDFObject) DF_PURE;
	
#else
	DF_METHOD(CreateDFObject)(DF_THIS_ const char* pObjectName, IDFObject** pDFObject) DF_PURE;
#endif

	/**
	 *  Destroy a DF object.
	 *	@param pDFObject The object to destroy
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(RemoveDFObject)(DF_THIS_ IDFObject* pDFObject) DF_PURE;

	/**
	 *  Destroy all DF Objects in the given zone.
	 *	@param pZoneId The zone name or NULL to destroy all objects in all zones.
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(DestroyAllDfObjects)(DF_THIS_ const char* pZoneId) DF_PURE;

	/**
	 *  Refresh the selected creative for all objects in a zone.
	 *	@param pZoneId The zone name or NULL to refresh all objects in all zones.
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(RefreshCurrentCreatives)(DF_THIS_ const char* pZoneId) DF_PURE;

	/**
	 *  Advance time for the DF Engine.
	 *	Update should be called once per frame.
	 *	All the communication performed by the DF engine (downloading of creatives, sending tracking information) is
	 *  done asynchronously in the Update function.
	 *
	 *	@param fElapsedTime elapsed time in seconds since the last frame.
	 *	@return No return value
	 */
	DF_METHOD_(void, Update)(DF_THIS_ const float fElapsedTime) DF_PURE;

	/**
	 *  Start a zone.
	 *	This function should be called when the player has entered a new zone.
	 *	Usually a zone corresponds to a level or an area in the game.
	 *	Starting a zone also starts the time counter for the specific zone (for zone tracking purposes)
	 *
	 *	@param pZoneId The zone name
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(StartZone)(DF_THIS_ const char* pZoneId) DF_PURE;

	/**
	 *  Leave a zone.
	 *	This function should be called when the player has left a zone.
	 *	Usually a zone corresponds to a level or an area in the game.
	 *	Leaving a zone also stops the time counter for the specific zone (for zone tracking purposes)
	 *
	 *	@param pZoneId The zone name
	 *	@return No return value
	 */
	DF_METHOD_(void, LeaveZone)(DF_THIS_ const char* pZoneId) DF_PURE;

	/**
	 *  Download campaign information from the server
	 *	The method will block until all creatives are downloaded.
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(UpdateContentFromServerBlocking)(DF_THIS) DF_PURE;

	/**
	 *  Get an engine parameter
	 *
	 *	@param pParamName		Parameter name
	 *	@param pParamValue		A pointer to the appropriate type
	 *	@param uAllocatedSize	The allocated size of pParamValue
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(GetEngineParameter)(DF_THIS_ const char* pParamName, void* pParamValue, DF_UInt32 uAllocatedSize) DF_PURE;

	/**
	 *  Set an engine parameter
	 *
	 *	@param pParamName  Parameter name
	 *	@param pParamValue Parameter string value
	 *
	 *	@return If the method succeeds, the return value is DF_OK. If the method fails, the return value is negative.
	 */
	DF_METHOD(SetEngineParameter)(DF_THIS_ const char* pParamName, const char* pParamValue) DF_PURE;

	/**
	*	Set the current view matrix of the camera.
	*   @param matView The view matrix in row major form.
	*/
	DF_METHOD_(void, SetCameraViewMatrix)(DF_THIS_ const float matView[16]) DF_PURE;

	/**
	*	Set the current projection matrix of the camera.
	*   @param matProj The projection matrix in row major form.
	*/
	DF_METHOD_(void, SetCameraProjMatrix)(DF_THIS_ const float matProj[16]) DF_PURE;

#ifdef DF_PS2
	  DF_METHOD(LoginToServer)(DF_THIS) DF_PURE;
	  DF_METHOD(CheckLoginStatus)(DF_THIS_ DF_RESULT* pTrackingStatus) DF_PURE;
	  DF_METHOD(SendTrackingToServer)(DF_THIS) DF_PURE;
	  DF_METHOD(CheckTrackingStatus)(DF_THIS_ DF_RESULT* pTrackingStatus) DF_PURE;
#endif

	  /**
	  *	 Set network download rate limit.
	  *  @param nRate The download limit in KB/Sec (0 for no limit).
	  */
	  DF_METHOD_(void, SetNetworkDownloadLimit)(DF_THIS_ const DF_UInt32 nRate) DF_PURE;
	 
	  /**
	  *	 Set network upload rate limit.
	  *  @param nRate The upload limit in KB/Sec (0 for no limit).
	  */
	  DF_METHOD_(void, SetNetworkUploadLimit)(DF_THIS_ const DF_UInt32 nRate) DF_PURE;

	  /**
	  *	 Enable or disable network communication.
	  *  @param bEnable true to enable network communication or false to disable it.
	  */
	  DF_METHOD_(void, SetNetworkEnable)(DF_THIS_ const bool bEnable) DF_PURE;
};

///////////////////////////////////////////////////////////////////////////////
// C API macros definition
///////////////////////////////////////////////////////////////////////////////
#if !defined(__cplusplus) || defined(DFENGINE_C_INTERFACE)
#define IDFEngine_Start(p, a, b) (p)->lpVtbl->Start(p, a, b)
#define IDFEngine_ShutDown(p) (p)->lpVtbl->ShutDown(p)
#define IDFEngine_SetSoundSystem(p, a, b) (p)->lpVtbl->SetSoundSystem(p, a, b)
#ifdef DF_WINDOWS
#define IDFEngine_CreateDFObject(p, a, b) (p)->lpVtbl->CreateDFObject(p, a, b)
#else
#define IDFEngine_CreateDFObject(p, a, b, c) (p)->lpVtbl->CreateDFObject(p, a, b, c)
#endif
#define IDFEngine_RemoveDFObject(p, a) (p)->lpVtbl->RemoveDFObject(p, a)
#define IDFEngine_DestroyAllDfObjects(p, a) (p)->lpVtbl->DestroyAllDfObjects(p, a)
#define IDFEngine_RefreshCurrentCreatives(p, a) (p)->lpVtbl->RefreshCurrentCreatives(p, a)
#define IDFEngine_Update(p, a) (p)->lpVtbl->Update(p, a)
#define IDFEngine_StartZone(p, a) (p)->lpVtbl->StartZone(p, a)
#define IDFEngine_LeaveZone(p, a) (p)->lpVtbl->LeaveZone(p, a)
#define IDFEngine_UpdateContentFromServerBlocking(p) (p)->lpVtbl->UpdateContentFromServerBlocking(p)
#define IDFEngine_GetEngineParameter(p, a, b, c) (p)->lpVtbl->GetEngineParameter(p, a, b, c)
#define IDFEngine_SetEngineParameter(p, a, b) (p)->lpVtbl->SetEngineParameter(p, a, b)
#define IDFEngine_SetCameraViewMatrix(p, a) (p)->lpVtbl->SetCameraViewMatrix(p, a)
#define IDFEngine_SetCameraProjMatrix(p, a) (p)->lpVtbl->SetCameraProjMatrix(p, a)
#define IDFEngine_SetNetworkDownloadLimit(p, a) (p)->lpVtbl->SetNetworkDownloadLimit(p, a)
#define IDFEngine_SetNetworkUploadLimit(p, a) (p)->lpVtbl->SetNetworkUploadLimit(p, a)
#define IDFEngine_SetNetworkEnable(p, a) (p)->lpVtbl->SetNetworkEnable(p, a)
#ifndef DF_WINDOWS
#define IDFEngine_LoginToServer(p) (p)->lpVtbl->LoginToServer(p)
#define IDFEngine_CheckLoginStatus(p, a) (p)->lpVtbl->CheckLoginStatus(p, a)
#define IDFEngine_SendTrackingToServer(p) (p)->lpVtbl->SendTrackingToServer(p)
#define IDFEngine_CheckTrackingStatus(p, a) (p)->lpVtbl->CheckTrackingStatus(p, a)
#endif
#else	// C++ macros definition
#define IDFEngine_Start(p, a, b) (p)->Start(a, b)
#define IDFEngine_ShutDown(p) (p)->ShutDown()
#define IDFEngine_SetSoundSystem(p, a, b) (p)->SetSoundSystem(a, b)
#ifdef DF_PS2
#define IDFEngine_CreateDFObject(p, a, b, c) (p)->CreateDFObject(a, b, c)
#else
#define IDFEngine_CreateDFObject(p, a, b) (p)->CreateDFObject(a, b)
#endif
#define IDFEngine_RemoveDFObject(p, a) (p)->RemoveDFObject(a)
#define IDFEngine_DestroyAllDfObjects(p, a) (p)->DestroyAllDfObjects(a)
#define IDFEngine_RefreshCurrentCreatives(p, a) (p)->RefreshCurrentCreatives(a)
#define IDFEngine_Update(p, a) (p)->Update(a)
#define IDFEngine_StartZone(p, a) (p)->StartZone(a)
#define IDFEngine_LeaveZone(p, a) (p)->LeaveZone(a)
#define IDFEngine_UpdateContentFromServerBlocking(p) (p)->UpdateContentFromServerBlocking()
#define IDFEngine_GetEngineParameter(p, a, b, c) (p)->GetEngineParameter(a, b, c)
#define IDFEngine_SetEngineParameter(p, a, b) (p)->SetEngineParameter(a, b)
#define IDFEngine_SetCameraViewMatrix(p, a) (p)->SetCameraViewMatrix(a)
#define IDFEngine_SetCameraProjMatrix(p, a) (p)->SetCameraProjMatrix(a)
#define IDFEngine_SetNetworkDownloadLimit(p, a) (p)->SetNetworkDownloadLimit(a)
#define IDFEngine_SetNetworkUploadLimit(p, a) (p)->SetNetworkUploadLimit(a)
#define IDFEngine_SetNetworkEnable(p, a) (p)->SetNetworkEnable(a)
#ifdef DF_PS2
#define IDFEngine_LoginToServer(p) (p)->LoginToServer()
#define IDFEngine_CheckLoginStatus(p, a) (p)->CheckLoginStatus(a)
#define IDFEngine_SendTrackingToServer(p) (p)->SendTrackingToServer()
#define IDFEngine_CheckTrackingStatus(p, a) (p)->CheckTrackingStatus(a)
#endif
#endif


#ifdef DF_STATIC_LIB
IDFEngine* CreateDFEngine();
#else
/**
 *	Create the one and only instance of the DF Engine
 */
DFENGINE_INTERFACE IDFEngine* CreateDFEngine();
#endif

#endif /* _H_DFENGINEHEADER_H */
