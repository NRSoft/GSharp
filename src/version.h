#ifndef GSHARP_VERSION_H
#define GSHARP_VERSION_H

namespace gsharp{

	//Date Version Types
	static const char GSHARP_DATE[] = "10";
	static const char GSHARP_MONTH[] = "06";
	static const char GSHARP_YEAR[] = "2016";
	static const char GSHARP_UBUNTU_VERSION_STYLE[] =  "16.06";

	//Software Status
	static const char GSHARP_STATUS[] =  "Alpha";
	static const char GSHARP_STATUS_SHORT[] =  "a";

	//Standard Version Type
	static const long GSHARP_MAJOR  = 1;
	static const long GSHARP_MINOR  = 0;
	static const long GSHARP_BUILD  = 61;
	static const long GSHARP_REVISION  = 302;

	//Miscellaneous Version Types
	static const long GSHARP_BUILDS_COUNT  = 302;
	#define GSHARP_RC_FILEVERSION 1,0,61,302
	#define GSHARP_RC_FILEVERSION_STRING "1, 0, 61, 302\0"
	static const char GSHARP_FULLVERSION_STRING [] = "1.0.61.302";

	//These values are to keep track of your versioning state, don't modify them.
	static const long GSHARP_BUILD_HISTORY  = 0;


}
#endif //GSHARP_VERSION_H
