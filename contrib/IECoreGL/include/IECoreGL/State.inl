#ifndef IECOREGL_STATE_INL
#define IECOREGL_STATE_INL

namespace IECoreGL
{

template<typename T>
typename T::Ptr State::get()
{
	return boost::staticPointerCast<T>( get( T::staticTypeId() ) );
}

template<typename T>
typename T::ConstPtr State::get() const
{
	return boost::staticPointerCast<const T>( get( T::staticTypeId() ) );
}

template<typename T>
void State::remove()
{
	remove( T::staticTypeId() );
}

} // namespace IECoreGL

#endif // IECOREGL_STATE_INL
