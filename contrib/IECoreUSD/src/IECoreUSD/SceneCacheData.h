//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREUSD_SCENE_CACHE_DATA_H
#define IECOREUSD_SCENE_CACHE_DATA_H

#include "IECoreScene/SceneInterface.h"
#include "IECoreScene/SharedSceneInterfaces.h"

#include "IECore/IndexedIO.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/tf/declarePtrs.h"
#include "pxr/base/tf/hashmap.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/vt/value.h"

#include "pxr/pxr.h"

#include "pxr/usd/sdf/abstractData.h"
#include "pxr/usd/sdf/api.h"
#include "pxr/usd/sdf/fileFormat.h"
#include "pxr/usd/sdf/path.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/shared_ptr.hpp"

#include <vector>

// plugins to USD are required to use the internal pxr namespace
// specifically here for the macro TF_DECLARE_WEAK_AND_REF_PTRS
// to be accessible.
PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_WEAK_AND_REF_PTRS(SceneCacheData);

class SceneCacheData : public SdfAbstractData
{
public:
	static SceneCacheDataRefPtr New( SdfFileFormat::FileFormatArguments = {} );
	bool Open( const std::string& filePath );

	/// SdfAbstractData overrides
	bool StreamsData() const override;

	void CreateSpec(const SdfPath& path, SdfSpecType specType) override;
	bool HasSpec(const SdfPath& path) const override;
	void EraseSpec(const SdfPath& path) override;
	void MoveSpec(const SdfPath& oldPath, const SdfPath& newPath) override;
	SdfSpecType GetSpecType(const SdfPath& path) const override;

	bool Has(const SdfPath& path, const TfToken &fieldName, SdfAbstractDataValue* value) const override;
	bool Has(const SdfPath& path, const TfToken& fieldName, VtValue *value = NULL) const override;
	bool HasSpecAndField(const SdfPath &path, const TfToken &fieldName, SdfAbstractDataValue *value, SdfSpecType *specType) const override;

	bool HasSpecAndField(const SdfPath &path, const TfToken &fieldName, VtValue *value, SdfSpecType *specType) const override;

	VtValue Get(const SdfPath& path, const TfToken& fieldName) const override;
	void Set(const SdfPath& path, const TfToken& fieldName, const VtValue & value) override;
	void Set(const SdfPath& path, const TfToken& fieldName, const SdfAbstractDataConstValue& value) override;
	void Erase(const SdfPath& path, const TfToken& fieldName) override;
	std::vector<TfToken> List(const SdfPath& path) const override;

	std::set<double> ListAllTimeSamples() const override;
	
	std::set<double> ListTimeSamplesForPath(const SdfPath& path) const override;

	bool GetBracketingTimeSamples(double time, double* tLower, double* tUpper) const override;

	size_t GetNumTimeSamplesForPath(const SdfPath& path) const override;

	bool GetBracketingTimeSamplesForPath(const SdfPath& path, double time, double* tLower, double* tUpper) const override;

	bool QueryTimeSample(const SdfPath& path, double time, SdfAbstractDataValue *optionalValue) const override;
	bool QueryTimeSample(const SdfPath& path, double time, VtValue *value) const override;

	void SetTimeSample(const SdfPath& path, double time, const VtValue & value) override;

	void EraseTimeSample(const SdfPath& path, double time) override;

protected:
	// SdfAbstractData overrides
	SceneCacheData( SdfFileFormat::FileFormatArguments );
	~SceneCacheData() override;

	void _VisitSpecs(SdfAbstractDataSpecVisitor* visitor) const override;
private:
	const VtValue queryTimeSample(const SdfPath& path, double time) const;

	const VtValue GetSpecTypeAndFieldValue(const SdfPath& path, const TfToken& field, SdfSpecType* specType) const;
	const VtValue GetFieldValue(const SdfPath& path, const TfToken& field, bool loadTimeSampleMap=true) const;

	VtValue* GetMutableFieldValue(const SdfPath& path, const TfToken& field);
	VtValue* GetOrCreateFieldValue(const SdfPath& path, const TfToken& field);

	void loadSceneIntoCache( IECoreScene::ConstSceneInterfacePtr scene );
	void loadPrimVars(const IECoreScene::SceneInterface::Path& currentPath, TfTokenVector& properties, TfToken PrimTypeName);

	void addProperty(
		const SdfPath& primPath,
		const TfToken& attributeName,
		const SdfValueTypeName& typeName,
		bool custom, const SdfVariability & variability,
		const TfToken * defaultValue=nullptr,
		bool defaultValueIsArray=true,
		const TfToken * interpolation=nullptr,
		bool useObjectSample=false
	);

	void addIncludeRelationship(
		const SdfPath& primPath,
		const TfToken& relationshipName,
		const SdfVariability & variability,
		const SdfListOp<SdfPath> & targetPaths,
		const std::vector<SdfPath> & targetChildren
	);

	double m_fps;
	void loadFps();
	double timeToFrame( double time ) const;
	double frameToTime( double frame ) const;

	// Backing storage for a single "spec" -- prim, property, etc.
	typedef std::pair<TfToken, VtValue> FieldValuePair;
	struct SpecData {
		SpecData() : specType(SdfSpecTypeUnknown) {}
		
		SdfSpecType specType;
		std::vector<FieldValuePair> fields;
	};

	// Hashtable storing SpecData.
	typedef SdfPath Key;
	typedef SdfPath::Hash KeyHash;
	typedef TfHashMap<Key, SpecData, KeyHash> HashTable;

	HashTable m_data;

	typedef std::map<std::string, std::vector<SdfPath>> collection;
	collection m_collections;

	void addCollections( SpecData& spec, TfTokenVector& properties, const SdfPath& primPath );
	void addReference( IECoreScene::ConstSceneInterfacePtr scene, SpecData& spec, TfTokenVector& children );
	void addValueClip( SpecData& spec, const VtVec2dArray times, const VtVec2dArray actives, const std::string& assetPath, const std::string& primPath);

	VtValue getTimeSampleMap( const SdfPath& path, const TfToken& field, const VtValue& value ) const;

	const SdfFileFormat::FileFormatArguments m_arguments;
	IECoreScene::ConstSceneInterfacePtr m_scene;
	IECore::IndexedIOPtr m_sceneio;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // IECOREUSD_SCENE_CACHE_DATA_H

