//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/SceneAlgo.h"

#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/SceneInterface.h"

#include "tbb/task.h"

#include <atomic>

using namespace IECore;
using namespace IECoreScene;

namespace
{

template<typename LocationFn>
class Task : public tbb::task
{

	public :

		Task(
			const SceneInterface *src, SceneInterface *dst, LocationFn &locationFn, double time, unsigned int flags
		) : m_src( src ), m_dst( dst ), m_locationFn( locationFn ), m_time( time ), m_flags( flags )
		{
		}

		~Task() override
		{
		}

		task *execute() override
		{
			m_locationFn( m_src, m_dst, m_time, m_flags );

			SceneInterface::NameList childNames;
			m_src->childNames( childNames );

			set_ref_count( 1 + childNames.size() );

			std::vector<SceneInterfacePtr> childSceneInterfaces;
			childSceneInterfaces.reserve( childNames.size() );

			std::vector<ConstSceneInterfacePtr> srcChildSceneInterfaces;
			srcChildSceneInterfaces.reserve( childNames.size() );

			for( const auto &childName : childNames )
			{
				SceneInterfacePtr dstChild = m_dst ? m_dst->child( childName, SceneInterface::CreateIfMissing ) : nullptr;
				if( dstChild )
				{
					childSceneInterfaces.push_back( dstChild );
				}

				ConstSceneInterfacePtr srcChild = m_src->child( childName );
				srcChildSceneInterfaces.push_back( srcChild );

				Task *t = new( allocate_child() ) Task( srcChild.get(), dstChild.get(), m_locationFn, m_time, m_flags );
				spawn( *t );
			}
			wait_for_all();

			return nullptr;
		}

	private :

		const SceneInterface *m_src;
		SceneInterface *m_dst;
		LocationFn &m_locationFn;
		double m_time;
		unsigned int m_flags;

};

template<typename T>
struct CopyInfo
{
	CopyInfo() : polygonCount( 0 ), curveCount( 0 ), pointCount( 0 ), attributeCount( 0 ), tagCount( 0 ), setCount( 0 )
	{
	}

	T polygonCount;
	T curveCount;
	T pointCount;

	T attributeCount;
	T tagCount;
	T setCount;
};

CopyInfo<size_t> handleLocation( const SceneInterface *src, SceneInterface *dst, double time, unsigned int flags )
{
	SceneInterface::Path path;
	src->path( path );
	bool isRoot = path.empty();
	CopyInfo<size_t> copyInfo;

	if( flags & SceneAlgo::Bounds )
	{
		auto bound = src->readBound( time );
		if( dst )
		{
			dst->writeBound( bound, time );
		}
	}

	if( flags & SceneAlgo::Transforms )
	{
		IECore::ConstDataPtr transform = src->readTransform( time );
		if( dst && !isRoot )
		{
			dst->writeTransform( transform.get(), time );
		}
	}

	if( flags & SceneAlgo::Attributes )
	{
		SceneInterface::NameList attributeNames;
		src->attributeNames( attributeNames );

		copyInfo.attributeCount += attributeNames.size();
		for( const auto &attributeName : attributeNames )
		{
			IECore::ConstObjectPtr attr = src->readAttribute( attributeName, time );
			if( dst )
			{
				dst->writeAttribute( attributeName, attr.get(), time );
			}
		}
	}

	if( flags & SceneAlgo::Tags )
	{
		SceneInterface::NameList tags;
		src->readTags( tags );
		copyInfo.tagCount += tags.size();
		if( dst )
		{
			dst->writeTags( tags );
		}
	}

	if( flags & SceneAlgo::Sets && isRoot )
	{
		SceneInterface::NameList setNames = src->setNames();
		copyInfo.setCount += setNames.size();
		for( const auto &setName : setNames )
		{
			PathMatcher set = src->readSet( setName );
			if( dst )
			{
				dst->writeSet( setName, set );
			}
		}
	}

	if( flags & SceneAlgo::Objects && src->hasObject() )
	{
		IECore::ConstObjectPtr obj = src->readObject( time );

		if( IECoreScene::MeshPrimitive::ConstPtr mesh = IECore::runTimeCast<const IECoreScene::MeshPrimitive>( obj ) )
		{
			copyInfo.polygonCount += mesh->numFaces();
		}
		else if( IECoreScene::CurvesPrimitive::ConstPtr curves = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>( obj ) )
		{
			copyInfo.curveCount += curves->numCurves();
		}
		else if( IECoreScene::PointsPrimitive::ConstPtr points = IECore::runTimeCast<const IECoreScene::PointsPrimitive>( obj ) )
		{
			copyInfo.pointCount += points->getNumPoints();
		}
		if( dst )
		{
			dst->writeObject( obj.get(), time );
		}
	}

	return copyInfo;

}

template<typename LocationFn>
void copy( const SceneInterface *src, SceneInterface *dst, double time, unsigned int flags, LocationFn &locationFn )
{
	locationFn( src, dst, time, flags );

	SceneInterface::NameList childNames;
	src->childNames( childNames );

	for( const auto &childName : childNames )
	{
		SceneInterfacePtr dstChild = dst->child( childName, SceneInterface::CreateIfMissing );
		::copy( src->child( childName ).get(), dstChild.get(), time, flags, locationFn );
	}
}

} // namespace

namespace IECoreScene
{

namespace SceneAlgo
{

SceneStats parallelReadAll( const SceneInterface *src, int startFrame, int endFrame, float frameRate, unsigned int flags )
{
	std::atomic<size_t> locationCount( 0 );
	::CopyInfo<std::atomic<size_t> > copyInfos;

	auto locationFn = [&locationCount, &copyInfos]( const SceneInterface *src, SceneInterface *dst, double time, unsigned int flags )
	{
		locationCount++;
		::CopyInfo<size_t> copyInfo = ::handleLocation( src, nullptr, time, flags );

		copyInfos.polygonCount += copyInfo.polygonCount;
		copyInfos.tagCount += copyInfo.tagCount;
		copyInfos.attributeCount += copyInfo.attributeCount;
		copyInfos.curveCount += copyInfo.curveCount;
		copyInfos.pointCount += copyInfo.pointCount;
	};

	for( int f = startFrame; f <= endFrame; ++f )
	{
		double time = f / frameRate;
		Task<decltype( locationFn )> *task = new( tbb::task::allocate_root() ) Task<decltype( locationFn )>( src, nullptr, locationFn, time, flags );
		tbb::task::spawn_root_and_wait( *task );
	}

	SceneStats stats;
	stats["locations"] = locationCount;
	stats["polygons"] = copyInfos.polygonCount;
	stats["curves"] = copyInfos.curveCount;
	stats["points"] = copyInfos.pointCount;
	stats["tags"] = copyInfos.tagCount;
	stats["sets"] = copyInfos.setCount;
	stats["attributes"] = copyInfos.attributeCount;
	return stats;
}

void copy( const SceneInterface *src, SceneInterface *dst, int startFrame, int endFrame, float frameRate, unsigned int flags )
{
	for( int f = startFrame; f <= endFrame; ++f )
	{
		double time = f / frameRate;
		// disable copying tags for all frame apart from the first.
		if( f != startFrame )
		{
			flags &= ~Tags;
		}

		::copy( src, dst, time, flags, ::handleLocation );
	}
}

} // SceneAlgo

} // IECoreScene