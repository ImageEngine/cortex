#ifndef IECOREGL_DISPLAY_H
#define IECOREGL_DISPLAY_H

#include "IECore/CompoundData.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( FrameBuffer );

class Display : public IECore::RefCounted
{
	
	public :

		/// A copy is taken of parameters
		Display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters );

		void display( ConstFrameBufferPtr frameBuffer ) const;

	private :

		std::string m_name;
		std::string m_type;
		std::string m_data;
		IECore::CompoundDataMap m_parameters;

};

} // namespace IECoreGL

#endif // IECOREGL_DISPLAY_H
