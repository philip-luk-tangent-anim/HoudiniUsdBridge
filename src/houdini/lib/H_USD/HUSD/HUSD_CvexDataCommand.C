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
 */


#include "HUSD_CvexDataCommand.h"
#include <VEX/VEX_GeoCommand.h>

HUSD_CvexDataCommand::HUSD_CvexDataCommand()
    : myVexGeoCommands( new UT_Array<VEX_GeoCommandQueue<HUSD_VEX_PREC>> )
{
}

HUSD_CvexDataCommand::~HUSD_CvexDataCommand()
{
}

void
HUSD_CvexDataCommand::setCommandQueueCount( int count )
{
    myVexGeoCommands->setSize( count );
}

int
HUSD_CvexDataCommand::getCommandQueueCount() const
{
    return myVexGeoCommands->size();
}

VEX_GeoCommandQueue<HUSD_VEX_PREC> &
HUSD_CvexDataCommand::getCommandQueue( int index )
{ 
    return (*myVexGeoCommands)[ index ];
}

