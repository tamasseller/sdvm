#include "1test/TestRunnerExperimental.h"
#include "1test/PrintfOutput.h"

int main(int argc, const char* argv[])
{
	return pet::TestRunner::Experimental::main(argc, argv, &pet::PrintfOutput::instance);
}
