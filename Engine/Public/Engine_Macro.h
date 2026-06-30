#ifndef Engine_Macro_h__
#define Engine_Macro_h__

namespace Engine
{
#define		VTXCNTX		129
#define		VTXCNTZ		129
#define		VTXITV			1

#ifndef			MSG_BOX
#define			MSG_BOX(_message)			MessageBoxW(nullptr, TEXT(_message), L"System Message", MB_OK)
#endif

#define			CRASH(cause) { std::abort();}
#define			ASSERT_CRASH(expr){   if (!(expr)) {CRASH("ASSERT_CRASH");__analysis_assume(expr);}}

#define			NS_BEGIN(NAMESPACE)		namespace NAMESPACE {
#define			NS_END						}

#define			USING(NAMESPACE)	using namespace NAMESPACE;

#define			ENUM_CLASS(e)		static_cast<unsigned int>(e)

#ifdef	ENGINE_EXPORTS
#define ENGINE_DLL		_declspec(dllexport)
#else
#define ENGINE_DLL		_declspec(dllimport)
#endif

#define NO_COPY(CLASSNAME)											\
		private:																	\
		CLASSNAME(const CLASSNAME&) = delete;					\
		CLASSNAME& operator = (const CLASSNAME&)= delete;		

#define DECLARE_SINGLETON(CLASSNAME)							\
		NO_COPY(CLASSNAME)												\
		private:																	\
		static CLASSNAME*	m_pInstance;								\
		public:																	\
		static CLASSNAME*	GetInstance( void );						\
		static void DestroyInstance( void );			

#define IMPLEMENT_SINGLETON(CLASSNAME)						\
		CLASSNAME*	CLASSNAME::m_pInstance = nullptr;			\
		CLASSNAME*	CLASSNAME::GetInstance( void )	{			\
			if(nullptr == m_pInstance) {										\
				m_pInstance = new CLASSNAME;							\
			}																		\
			return m_pInstance;												\
		}																			\
		void CLASSNAME::DestroyInstance( void ) {						\
			if(nullptr != m_pInstance)	{									\
				delete m_pInstance;											\
				m_pInstance = nullptr;										\
			}																		\
		}
}

#endif // Engine_Macro_h__
