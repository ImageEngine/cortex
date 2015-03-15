//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#ifndef IECORE_CURVETRACER_H
#define IECORE_CURVETRACER_H

#include "boost/graph/adjacency_list.hpp"

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/ImagePrimitive.h"

namespace IECore
{

/// The CurveTracer class takes images like those produced by the
/// ImageThinner and outputs a CurvesPrimitive matching the lines.
/// \ingroup imageProcessingGroup
class IECORE_API CurveTracer : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( CurveTracer, Op );

		CurveTracer();
		virtual ~CurveTracer();

		ImagePrimitiveParameter * imageParameter();
		const ImagePrimitiveParameter * imageParameter() const;
		
		StringParameter * channelNameParameter();
		const StringParameter * channelNameParameter() const;
		
		FloatParameter * vertexMergeDistanceParameter();
		const FloatParameter * vertexMergeDistanceParameter() const;
		
		FloatParameter * minimumLengthParameter();
		const FloatParameter * minimumLengthParameter() const;

		BoolParameter * colorParameter();
		const BoolParameter * colorParameter() const;

		enum OutputType
		{
			Linear,
			CatmullRom
		};

		IntParameter * outputTypeParameter();
		const IntParameter * outputTypeParameter() const;

		IntParameter * catmullRomStepParameter();
		const IntParameter * catmullRomStepParameter() const;
		
		IntParameter * outputSpaceParameter();
		const IntParameter * outputSpaceParameter() const;
		
	protected :

		ObjectPtr doOperation( const CompoundObject * operands );
		
	private :
	
		// lots and lots of typedefs for types used to represent the graph of connections.
		typedef std::vector<Imath::V2i> PixelVector;
		typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, boost::property<boost::vertex_name_t, Imath::V2i>, boost::property<boost::edge_name_t, PixelVector> > Graph;
		typedef Graph::vertex_descriptor Vertex;
		typedef Graph::vertex_iterator VertexIterator;
		typedef std::pair<VertexIterator, VertexIterator> VertexIteratorRange;
	
		typedef std::map<Imath::V2i, Vertex> VertexMap;
	
		typedef Graph::edge_descriptor Edge;
		typedef Graph::edge_iterator EdgeIterator;
		typedef std::pair<EdgeIterator, EdgeIterator> EdgeIteratorRange;
		typedef std::pair<Edge, bool> EdgeBool;
	
		typedef Graph::out_edge_iterator OutEdgeIterator;
		typedef std::pair<OutEdgeIterator, OutEdgeIterator> OutEdgeIteratorRange;
	
		typedef Graph::in_edge_iterator InEdgeIterator;
		typedef std::pair<InEdgeIterator, InEdgeIterator> InEdgeIteratorRange;
		
		void buildGraph( const std::vector<float> &channel, const Imath::V2i &size, Graph &graph ) const;
		void removeSpurs( Graph &graph, float minimumLength ) const;
		void mergeVertices( Graph &graph, float mergeDistance ) const;
		CurvesPrimitivePtr buildCurves( Graph &graph, OutputType type, int catmullRomStep ) const;
		void colorCurves( CurvesPrimitive * curves ) const;

};

IE_CORE_DECLAREPTR( CurveTracer );

} // namespace IECore

#endif // IECORE_ENVMAPSAMPLER_H
