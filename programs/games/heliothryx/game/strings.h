#ifndef RS_STRINGS_H
#define RS_STRINGS_H

#ifndef RS_KOS
	#include "strings_ru.h" 
#else

	//#include "../lang.h"
	
	#ifdef LANG_RU
		// Russian
		#include "strings_ru.h"
	#elif LANG_SP
		// other languages are not implemented
		#include "strings_en.h"
	#elif LANG_IT
		// other languages are not implemented
		#include "strings_en.h"
	#else
		// default language: English
		#include "strings_en.h"
	#endif
		

#endif



#endif
