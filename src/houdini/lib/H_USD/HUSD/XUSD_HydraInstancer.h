/*
 * Copyright 2019 Side Effects Software Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Produced by:
 *	Side Effects Software Inc
 *	123 Front Street West, Suite 1401
 *	Toronto, Ontario
 *	Canada   M5J 2M2
 *	416-504-9876
 *
 * NAME:	XUSD_HydraInstancer.h (HUSD Library, C++)
 *
 * COMMENTS:	Basic instancer for creating instance transforms.
 *
 */

#ifndef XUSD_HydraInstancer_h
#define XUSD_HydraInstancer_h

#include "HUSD_API.h"
#include <UT/UT_Lock.h>
#include <UT/UT_Map.h>
#include <UT/UT_StringMap.h>
#include <UT/UT_SmallArray.h>
#include <UT/UT_UniquePtr.h>
#include <GT/GT_Transform.h>
#include <GT/GT_TransformArray.h>

#include <pxr/pxr.h>
#include <pxr/imaging/hd/instancer.h>
#include <pxr/imaging/hd/vtBufferSource.h>
#include <pxr/base/tf/hashmap.h>
#include <pxr/base/tf/token.h>

class HUSD_Scene;

PXR_NAMESPACE_OPEN_SCOPE

class HUSD_API XUSD_HydraInstancer : public HdInstancer
{
public:
    XUSD_HydraInstancer(HdSceneDelegate* del,
			SdfPath const& id,
			SdfPath const &parentInstancerId);
    ~XUSD_HydraInstancer() override;

    // Checks the change tracker to determine whether instance primvars are
    // dirty, and if so pulls them. Since primvars can only be pulled once,
    // and are cached, this function is not re-entrant. However, this function
    // is called by ComputeInstanceTransforms, which is called (potentially)
    // by HdMantraMesh::Sync(), which is dispatched in parallel, so it needs
    // to be guarded by _instanceLock.
    //
    // The @c nsegs variable indicates the number of segments/samples required
    // for motion blur.  The function returns the actual number of segments on
    // the instancer.
    //
    // Pulled primvars are cached in _primvarMap.
    int		syncPrimvars(bool recurse, int nsegs=1);

    // Return the number of evaluated motion segments
    int		motionSegments() const
    {
	return SYSmax(xsegments(), psegments());
    }

    // Grab the transforms for this instancer, and flatten it with any parent
    // instancers if 'recurse' is true. syncPrimvars() must be called first.
    VtMatrix4dArray	computeTransforms(const SdfPath    &protoId,
                                bool              recurse,
                                const GfMatrix4d *protoXform,
                                float		  shutter_time=0);

    // Grab the transforms and scene ids for each instance. If 'recurse' is
    // true, flatten both the transforms and ids for nested instancers.
    // syncPrimvars() must be called first.
    VtMatrix4dArray	computeTransformsAndIDs(const SdfPath    &protoId,
                                bool              recurse,
                                const GfMatrix4d *protoXform,
                                int               level,
                                UT_IntArray      &ids,
                                HUSD_Scene       *scene,
                                float		  shutter=0,
                                int               hou_proto_id=-1);

    bool                isResolved() const { return myIsResolved; }
    void                resolved() { myIsResolved = true; }

    // Add all instance prims to the scene tree. This does nothing for point
    // instancers.
    void                resolveInstancePrims();

    UT_StringArray      resolveInstance(int proto_id,
                                        const UT_IntArray &indices,
                                        int instance_level = 0);
    UT_StringArray      resolveInstanceID(HUSD_Scene &scene,
                                          const UT_StringRef &houdini_inst_path,
                                          int instance_idx,
                                          UT_StringHolder &indices,
                                          UT_StringArray *proto_id = nullptr)
                                          const;
    void                addInstanceRef(int id);
    void                removeInstanceRef(int id);
    bool                invalidateInstanceRefs();
    const UT_Map<int,int> &instanceRefs() const;
    void                clearInstanceRefs();

    const UT_StringRef &getCachedResolvedInstance(const UT_StringRef &id_key);
    void                cacheResolvedInstance(const UT_StringRef &id_key,
                                              const UT_StringRef &resolved);

    int                 id() const { return myID; }

    void                removePrototype(const UT_StringRef &proto_path,
                                        int proto_id);
    const UT_StringMap< UT_Map<int,int> > &prototypes() const
                        { return myPrototypes; }
    const UT_Map<int, UT_StringHolder>  &prototypeIDs() const
                        { return myPrototypeID; }

    const VtValue      &primvarValue(const TfToken &name) const;

    void                setIsPointInstancer(bool is_pi)
                        { myIsPointInstancer = is_pi; }
    bool                isPointInstancer() const
                        { return myIsPointInstancer; }

protected:
    class PrimvarMapItem
	: public UT_NonCopyable
    {
	using BufferPtr = UT_UniquePtr<HdVtBufferSource>;
    public:
	PrimvarMapItem()
	    : myBuffers()
	    , myValues()
	    , mySize(0)
	{
	}
	PrimvarMapItem(exint size)
	    : myBuffers(UTmakeUnique<BufferPtr[]>(size))
	    , myValues(UTmakeUnique<VtValue[]>(size))
	    , mySize(size)
	{
	}
	PrimvarMapItem(PrimvarMapItem &&src)
	    : myBuffers(std::move(src.myBuffers))
	    , myValues(std::move(src.myValues))
	    , mySize(src.mySize)
	{
	    src.mySize = 0;
	}
	PrimvarMapItem	&operator=(PrimvarMapItem &&src)
	{
	    myBuffers = std::move(src.myBuffers);
	    myValues = std::move(src.myValues);
	    mySize = src.mySize;
	    src.mySize = 0;
	    return *this;
	}
	~PrimvarMapItem()
	{
	}
	exint			 size() const
        {
            return mySize;
        }
	const HdVtBufferSource	*buffer(exint i) const
	{
	    UT_ASSERT_P(i >= 0 && i < mySize);
	    return myBuffers[i].get();
	}
	const VtValue	&value(exint i) const
	{
	    UT_ASSERT_P(i >= 0 && i < mySize);
	    return myValues[i];
	}
	const HdVtBufferSource	*operator[](exint i) const
        {
            return buffer(i);
        }
	void setValueAndBuffer(exint idx, const VtValue &value, BufferPtr b)
	{
            myValues[idx] = value;
	    myBuffers[idx] = std::move(b);
	}

    private:
	UT_UniquePtr<BufferPtr[]>	myBuffers;
	UT_UniquePtr<VtValue[]>     	myValues;
	exint				mySize;
    };

    /// Given a shutter time and a number of motion segments, return the motion
    /// segment and interpolant.  If seg0 != seg1, then values should be
    /// interpolated using: @code
    ///    val = SYSlerp(primvar[seg0], primvar[seg1], lerp);
    /// @endcode
    void	getSegment(float time, int &seg0, int &seg1, float &lerp,
			    bool for_transform) const;

    const float	*xtimes() const { return myXTimes.data(); }
    int		 xsegments() const { return myXTimes.size(); }
    const float	*ptimes() const { return myPTimes.data(); }
    int		 psegments() const { return myPTimes.size(); }

    // Map of the latest primvar data for this instancer, keyed by
    // primvar name. Primvar values are VtValue, an any-type; they are
    // interpreted at consumption time (here, in ComputeInstanceTransforms).
    UT_Map<TfToken, PrimvarMapItem, TfToken::HashFunctor>	myPrimvarMap;
    UT_SmallArray<float>       	myXTimes;	// USD time samples for xforms
    UT_SmallArray<float>	myPTimes;	// USD time samples for primvars
    UT_SmallArray<GfMatrix4d>	myXforms;	// Transform matrices

    mutable UT_Lock myLock;

private:
    UT_StringHolder findParentInstancer() const;

    VtMatrix4dArray privComputeTransforms(const SdfPath    &prototypeId,
                                          bool              recurse,
                                          const GfMatrix4d *protoXform,
                                          int               level,
                                          UT_StringArray   *instances,
                                          UT_IntArray      *ids,
                                          HUSD_Scene       *scene,
					  float		    shutter_time,
                                          int               hou_proto_id = -1);
    
    UT_StringMap<UT_StringHolder>  myResolvedInstances;
    UT_Map<int,int>                myInstanceRefs;
    UT_StringMap<UT_Map<int,int> > myPrototypes;
    UT_Map<int, UT_StringHolder>   myPrototypeID;
    
    int  myID;
    bool myIsResolved;
    bool  myIsPointInstancer;
};

class XUSD_HydraTransforms : public GT_TransformArray
{
public:
	     XUSD_HydraTransforms() : myDataId(-1) {}
            ~XUSD_HydraTransforms() override {}

    void     setDataId(int64 id)           { myDataId = id; }
    int64    getDataId() const override    { return myDataId; }
private:
    int64 myDataId;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
