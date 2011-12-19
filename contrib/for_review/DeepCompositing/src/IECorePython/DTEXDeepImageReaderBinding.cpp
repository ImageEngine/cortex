#include "boost/python.hpp"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreRI/DTEXDeepImageReader.h"

using namespace boost::python;
using namespace IECorePython;

namespace IECoreRI
{

void bindDTEXDeepImageReader()
{

	RunTimeTypedClass<DTEXDeepImageReader>()
		.def( init<>() )
		.def( init<const std::string &>() )
		.def( "canRead", &DTEXDeepImageReader::canRead ).staticmethod( "canRead" )
	;

}

} // namespace IECoreRI

