#include "Uploaders.h"
using namespace Upp;

/*
	If uploading of file fail then
	it mean that your GetIdentity(...) function is not up to date with mine. Find it and
	replace it by this one :
		
		String GetIdentity(const Renderer *r)
		{
			Http *http = const_cast<Http *>(dynamic_cast<const Http *>(r));
			if(!http)
				throw Exc("invalid POST identity call");
			
			//New
			bool find = false;
			Upp::String s;
			for(const Upp::String& key : http->var.GetKeys()){
				if(key.Find("__identity__") != -1){
					s = http->var.Get(key).ToString();
					find = true;
					break;
				}
			}
			if(find)
				return s;
			s = AsString(Uuid::Create());
			http->SessionSet0("__identity__", s);
			http->var[0] = s;
			return s;
		}

	For more information, see https://www.ultimatepp.org/forums/index.php?t=msg&goto=53798&&srch=skylark#msg_53798

*/


CONSOLE_APP_MAIN
{
	#ifdef _DEBUG
		StdLogSetup(LOG_FILE|LOG_COUT);
		Ini::skylark_log = true;
		
	#endif
	Uploaders().Run();
}
