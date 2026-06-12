#pragma once

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
