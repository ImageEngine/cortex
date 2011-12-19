#include "boost/python.hpp"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreRI/DTEXDeepImageWriter.h"

using namespace boost::python;
using namespace IECorePython;

namespace IECoreRI
{

void bindDTEXDeepImageWriter()
{

	RunTimeTypedClass<DTEXDeepImageWriter>()
		.def( init<>() )
		.def( init<const std::string &>() )
		.def( "canWrite", &DTEXDeepImageWriter::canWrite ).staticmethod( "canWrite" )
	;

}

} // namespace IECoreRI

