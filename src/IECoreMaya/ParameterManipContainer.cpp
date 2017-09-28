
#include "IECoreMaya/ParameterManipContainer.h"

using namespace IECoreMaya;

ParameterManipContainer::ParameterManipContainer()
	: m_label( "" )
{
}

void ParameterManipContainer::setPlug( MPlug &plug )
{
	m_plug = plug;
}

MPlug ParameterManipContainer::getPlug()
{
	return m_plug;
}

void ParameterManipContainer::setLabel( MString &label )
{
	m_label = label;
}

MString ParameterManipContainer::getLabel()
{
	return m_label;
}

