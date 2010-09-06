/*
 * NodePassData.h
 *
 *  Created on: 01/09/2010
 *      Author: dan
 */

#ifndef NODEPASSDATA_H_
#define NODEPASSDATA_H_

class OP_Node;

namespace IECoreHoudini
{
	/// This lightweight class is used to pass pointers to
	/// procedurals & ops around the Houdini graph using GU_Details
	/// ATTRIB_MIXED attribute type
	class NodePassData
	{
		public:
			enum NodeType { CORTEX_OPHOLDER=0,
							CORTEX_PROCEDURALHOLDER,
			};

			NodePassData( OP_Node *node, NodeType type );
			~NodePassData();

			const OP_Node *nodePtr() const { return mp_node; }
			const NodeType type() const { return m_type; }

		private:
			OP_Node *mp_node;
			NodeType m_type;
	};
}

#endif /* NODEPASSDATA_H_ */
