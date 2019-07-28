#define PLAYFX_NOT_ATTACHED			0
#define PLAYFX_PREVIOUS				1
#define PLAYFX_NO_TIMEOUT			0.0f

#define PLAYFX_NORMAL				0x00
#define PLAYFX_FROM_AOE				0x01
#define PLAYFX_CONTINUING			0x02
#define PLAYFX_AT_GEO_LOC			0x04
#define PLAYFX_POWER_TYPE(x)		((x) & 0xff)

#define PLAYFX_DONTPITCHTOTARGET	0
#define PLAYFX_PITCHTOTARGET		0x100
#define PLAYFX_PITCH_TO_TARGET(x)	((x) & PLAYFX_PITCHTOTARGET ? 1 : 0)

#define PLAYFX_NO_TINT				0x200
#define PLAYFX_TINT_FLAG(power)		(power_IsTinted(power) ? 0 : PLAYFX_NO_TINT)

#define PLAYFX_IMPORTANT			0x400
#define PLAYFX_IS_IMPORTANT(x)		((x) & PLAYFX_IMPORTANT ? 1 : 0)

#define PLAYFX_REFRESH_TIME			0.5f
// If the toggle fx hasn't heard a refresh command in this long, kill it

