#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{
	enum class D3DTS { VIEW, PROJ, END };
	enum class STATE { RIGHT, UP, LOOK, POSITION };
	enum class STATIC { STATIC, NONE };
	enum class WINMODE { FULL, WIN };
	enum class POINTS { A, B, C, END};
	enum class LINE { AB, BC, CA, END };

	// IK
	enum class EIKSOLVER_TYPE : unsigned int { TWO_BONE = 0, CCD, FABRIK, END };
	enum class EIKTARGET_MODE : unsigned int { POSITION, POSITION_CLEARANCE, END };
	enum class EALIGN_MODE : unsigned int { NONE, WALL, FLOOR };

	// Fade
	enum class FADE { FADE_OUT, FADE_IN };

	// Morph Target
	enum class KEY_INPTEROLATION : unsigned int { LINEAR = 0, CONSTANT, CUBIC, END };
	enum class KEY_TARGENT_MODE : unsigned int { AUTO = 0, USER, BREAK, END };

	enum class OPEN_RT { DEPTH };

	// Input
	enum class KEYSTATE { DOWN, PRESS, UP, END };
	enum class MOUSEKEYSTATE { LB, RB, WB, END };
	enum class MOUSEMOVESTATE { X, Y, WHEEL, END };
	 
	// Render
	
	enum class TEXTURETYPE { DIFFUSE, NORMAL, MASK, EMISSIVE, END };


	enum class MODELTYPE { NONANIM, ANIM, MAP, ECO, CHARACTER, VA };
	enum class PROTOTYPE { GAMEOBJECT, COMPONENT };
	enum class RENDERGROUP { PRIORITY, SHADOW, OUTLINE_NONCOMPARE, OUTLINE, NONBLEND, STATIC, NONSTATIC, DYNAMIC, WATER, NONLIGHT, EMISSIVE, EFFECT, SFX, BLEND, DISTORTION, POST_SFX, UI, UI_POST, FADE,
	#ifdef _DEBUG  
	RD_DEBUG, 
	#endif  
	END };
	enum class LOADSTATE { NOTLOADED, LOADING, LOADED };
	enum class EVENT { STATIC, NONSTATIC, END};

	enum class SHADER_DEFFERED { DRAW, RD_DEBUG_CSM, RD_DEBUG_SHAODW_MAP, COMBINED, LIGHT, BLOOM, DISTORTION, LUT, FOG, SSAO, DOF, DOF_DEPTH, BLUR, VELOCITY_MAP, MOTION_BLUR, WATER, WEIGHTBLEND};

	enum class SHADER_ENVMAP_DEFFERED { DIRECTIONAL, POINT };

	enum class SFX_TYPE { SSAO, BLOOM, BLUR, DOF, RADIAL, MOTION, SSS, WATER, END};

	enum class SFX_TOGGLE { BLUR = static_cast<int>(SFX_TYPE::BLUR), DOF = static_cast<int>(SFX_TYPE::DOF), RADIAL = static_cast<int>(SFX_TYPE::RADIAL), MOTION = static_cast<int>(SFX_TYPE::MOTION), END };

	// BroadPhase Layer
	enum class BPLAYER { NONE, NON_MOVE, MOVE, DEBRIS, SENSOR, END };
	// Body Shape
	enum class SHAPE { SPHERE, BOX, CAPSULE, CONVEXHULL, MESH };
	// Collide Timing
	enum class COLLIDE_STATE { ENTER, DURING, REMOVE, END};

	enum class EFFECT_TYPE { PARTICLE, MESH, TRAIL, RECT, DECAL, RADIAL, VA, LIGHT, SPECTRUM, END };

	// Sequence
	enum class ITEM_TYPE { ACTION, SCENE, SOUND, SFX, ACTOR, EFFECT, END };
}
#endif // Engine_Enum_h__
