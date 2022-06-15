#ifndef APPLICATION_H
#define APPLICATION_H

#include "ConfigReader.h"


namespace Dooky {
	class Application {
	private:
		
	public:
		void Begin(int argc, wchar_t** argv, Config config);
	};
}

#endif