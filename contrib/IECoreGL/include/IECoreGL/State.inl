#ifndef IECOREGL_STATE_INL
#define IECOREGL_STATE_INL

namespace IECoreGL
{

template<typename T>
typename T::Ptr State::get()
{
	return boost::static_pointer_cast<T>( get( T::staticTypeId() ) );	
}

template<typename T>
typename T::ConstPtr State::get() const
{
	return boost::static_pointer_cast<const T>( get( T::staticTypeId() ) );
}

template<typename T>
void State::remove()
{
	remove( T::staticTypeId() );
}

} // namespace IECoreGL

#endif // IECOREGL_STATE_INL
