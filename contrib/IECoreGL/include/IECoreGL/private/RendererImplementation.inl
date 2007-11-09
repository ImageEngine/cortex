#ifndef IECOREGL_RENDERERIMPLEMENTATION_INL
#define IECOREGL_RENDERERIMPLEMENTATION_INL

namespace IECoreGL
{

template <class T>
typename T::Ptr RendererImplementation::getState()
{
	return boost::static_pointer_cast<T>( getState( T::staticTypeId() ) );
}

} // namespace IECoreGL

#endif // IECOREGL_RENDERERIMPLEMENTATION_H
