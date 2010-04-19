#ifndef IECOREGL_STATECOMPONENT_INL
#define IECOREGL_STATECOMPONENT_INL

#include "IECoreGL/State.h"

namespace IECoreGL
{

template<typename T>
StateComponent::Description<T>::Description()
{
	IECoreGL::State::registerComponent( T::staticTypeId(), creator );
}

template<typename T>
StateComponentPtr StateComponent::Description<T>::creator()
{
	return new T;
}

} // namespace IECoreGL

#endif // IECOREGL_STATECOMPONENT_INL
