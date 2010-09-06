#include "NodePassData.h"
using namespace IECoreHoudini;

NodePassData::NodePassData( OP_Node *node, NodeType type ) :
		mp_node(node),
		m_type(type)
{
}

NodePassData::~NodePassData()
{
}
