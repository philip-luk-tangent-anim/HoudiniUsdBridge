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
 *	Side Effects Software Inc.
 *	123 Front Street West, Suite 1401
 *	Toronto, Ontario
 *      Canada   M5J 2M2
 *	416-504-9876
 *
 * Provides binding remapping of cvex parms to USD prim's attributes.
 */

#ifndef __HUSD_CvexBindingMap__
#define __HUSD_CvexBindingMap__

#include "HUSD_API.h"
#include <UT/UT_StringMap.h>
class OP_Node;

class HUSD_API HUSD_CvexBindingMap 
{
public:
    /// Convenience function for building the map from node's parameters.
    /// The parameters are assumed to have 1-base index (ie, default).
    static HUSD_CvexBindingMap	
		constructBindingsMap( const OP_Node &node,
			const char *bindings_num_parm,
			const char *cvex_parm_name_parm,
			const char *usd_attrib_name_parm, 
			const char *usd_attrib_type_parm, 
			const char *auto_bind_parm );

    /// Add an entry to the map.
    void	addBinding( const UT_StringHolder &parm_name,
			const UT_StringHolder &attrib_name,
			const UT_StringHolder &attrib_type = UT_StringHolder());

    /// Specify if attrib name can be used as parm name.
    void		setDefaultToIdentity( bool do_identity );

    /// Obtain the attribute name for the given cvex parameter name.
    UT_StringHolder	getAttribFromParm( const UT_StringRef &parm ) const;

    /// Obtain the requested attribute type for the given cvex parameter name.
    UT_StringHolder	getAttribTypeFromParm( const UT_StringRef &parm ) const;

private:
    UT_StringMap<UT_StringHolder>	myAttribFromParm;
    UT_StringMap<UT_StringHolder>	myAttribTypeFromParm;
    bool				myDefaultToIdentity = true;
};


#endif

