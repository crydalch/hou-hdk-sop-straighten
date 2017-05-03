/*
	Tool that straightens selected edges.

	IMPORTANT! ------------------------------------------
	-----------------------------------------------------

	Author: 	SNOWFLAKE
	Email:		snowflake@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SNOWFLAKE
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <UT/UT_Interrupt.h>
#include <OP/OP_AutoLockInputs.h>
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_Error.h>
#include <PRM/PRM_Include.h>
#include <GU/GU_EdgeUtils.h>
#include <sys/SYS_Math.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Utility/ParameterAccessing.h>

// this
#include "Parameters.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_Straighten
#define SOP_SmallName			"modeling::straighten::1.0"
#define SOP_Input_Name_0		"Geometry"
#define SOP_Icon_Name			"SOP_Straighten.svg"
#define SOP_Base_Operator		SOP_Node
#define MSS_Selector			GET_SOP_Namespace()::MSS_StraightenSelector

// very important
#define SOP_GroupFieldIndex_0	1

#define UI						GET_SOP_Namespace()::UI
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::input0EdgeGroup_Parameter,

	UI::mainSectionSwitcher_Parameter,	
	UI::uniformDistributionToggle_Parameter,
	UI::uniformDistributionSeparator_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	UI::setMorphToggle_Parameter,
	UI::setMorphSeparator_Parameter,
	UI::morphPowerFloat_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),
	
PARAMETERLIST_End()


bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// is input connected?
	exint is0Connected = getInput(0) == NULL ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */
	
	visibilityState = is0Connected ? 1 : 0; // TODO: do I still need this?	

	/* ---------------------------- Set States --------------------------------------- */
	
	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::setMorphToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::morphPowerFloat_Parameter.getToken(), visibilityState);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_Straighten() { }

SOP_Operator::SOP_Straighten(OP_Network* network, const char* name, OP_Operator* op) : SOP_Base_Operator(network, name, op), _edgeGroupInput0(NULL)
{ op->setIconName(SOP_Icon_Name); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const 
{ return SOP_Input_Name_0; }

OP_ERROR
SOP_Operator::cookInputGroups(OP_Context &context, int alone)
{ return cookInputEdgeGroups(context, this->_edgeGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, 0); }

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

int
SOP_Operator::CallbackSetMorph(void* data, int index, float time, const PRM_Template* tmp)
{
	auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// TODO: figure out why restoreFactoryDefaults() doesn't work
	auto defVal = UI::morphPowerFloat_Parameter.getFactoryDefaults()->getFloat();
	PRM_ACCESS::Set::FloatPRM(me, defVal, UI::morphPowerFloat_Parameter, time);

	return 1;
}

bool 
SOP_Operator::ExtractDataFromEdges(GA_EdgeTData<GA_Edge>& edgedata, UT_AutoInterrupt progress)
{
	edgedata.Clear();

	// extract data from edges
	for (auto edgeIt = this->_edgeGroupInput0->begin(); !edgeIt.atEnd(); ++edgeIt)
	{
		if (progress.wasInterrupted())
		{
			addError(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return false;
		}
		if (!edgeIt->isValid()) continue;
		
		edgedata.AddEdge(edgeIt->p0(), *edgeIt);
		edgedata.AddEdge(edgeIt->p1(), *edgeIt);
	}

	// collect endpoints	
	for (auto data : edgedata.GetExtractedData())
	{
		if (progress.wasInterrupted())
		{
			addError(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return false;
		}
		if (data.second.size() > 1) continue;

		edgedata.AddEndPoint(data.first);		
	}

	return true;
}

bool 
SOP_Operator::FindAllEdgeIslands(GA_EdgeTData<GA_Edge>& edgedata, UT_AutoInterrupt progress)
{
	// make sure nothing is cached
	this->_edgeIslands.clear();
	
	// since we have endpoints,
	// we can make our search faster by going only over them, 
	// instead of looking thru all points of GA_EdgeTData::GetExtractedData()
	for (auto point : edgedata.GetEndPoints())
	{
		if (progress.wasInterrupted())
		{
			addError(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return false;
		}

		// check if island was already discovered
		bool canContinue = true;
		for (auto island : this->_edgeIslands)
		{
			if (progress.wasInterrupted())
			{
				addError(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
				return false;
			}

			if (island.Contains(point))
			{
				canContinue = false;
				break;
			}
		}
		if (!canContinue) continue;

		// we are looking only for islands that have endpoints
		auto edgeIsland = GA_EdgeTIsland<GA_Edge>(EdgeIslandType::OPEN);

		auto success = FindEdgesRecurse(edgedata, point, point, edgeIsland, progress);
		if (!success) return false;

		this->_edgeIslands.push_back(edgeIsland);
	}

	return true;
}

bool
SOP_Operator::FindEdgesRecurse(GA_EdgeTData<GA_Edge>& edgedata, const GA_Offset startoffset, const GA_Offset nextoffset, GA_EdgeTIsland<GA_Edge>& edgeisland, UT_AutoInterrupt progress)
{
	// how many edges?
	auto edges = edgedata.GetExtractedData().at(nextoffset);
	switch (edges.size())
	{
		case 0:
			{
				// this shouldn't ever happen, but who knows?! I don't :)
				std::cout << "WTF?" << std::endl;
			}
			break;
		case 1:
			{ return WhenOneEdge(edgedata, startoffset, nextoffset, edgeisland, progress); }
			break;
		default:
			{ return WhenMoreThanOneEdge(edgedata, startoffset, nextoffset, edgeisland, progress); }
			break;
	}

	return true;
}

bool
SOP_Operator::WhenOneEdge(GA_EdgeTData<GA_Edge>& edgedata, const GA_Offset startoffset, const GA_Offset nextoffset, GA_EdgeTIsland<GA_Edge>& edgeisland, UT_AutoInterrupt progress)
{
	// store ourself
	edgeisland.AddEndPoint(nextoffset);

	// are we at start?
	if (nextoffset == startoffset)
	{
		// grab edge...
		auto edge = *edgedata.GetExtractedData().at(nextoffset).begin();
		edgeisland.AddEdge(edge);

		// ... and process next point
		auto newNextOffset = edge.p0() == nextoffset ? edge.p1() : edge.p0();
		return FindEdgesRecurse(edgedata, startoffset, newNextOffset, edgeisland, progress);
	}

	return true;
}

bool
SOP_Operator::WhenMoreThanOneEdge(GA_EdgeTData<GA_Edge>& edgedata, const GA_Offset startoffset, const GA_Offset nextoffset, GA_EdgeTIsland<GA_Edge>& edgeisland, UT_AutoInterrupt progress)
{
	// store ourself
	edgeisland.AddPoint(nextoffset);

	// grab edges	
	auto edges = edgedata.GetExtractedData().at(nextoffset);
	for (auto edge : edges)
	{
		if (progress.wasInterrupted())
		{
			addError(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return false;
		}

		// find point that != nextoffset and is not stored in edgeisland
		auto newNextOffset = edge.p0() == nextoffset ? edge.p1() : edge.p0();
		if (edgeisland.Contains(newNextOffset)) continue;

		// now we are sure that we didn't visited this branch, so we can add edge...
		edgeisland.AddEdge(edge);

		// ... and process next point
		return FindEdgesRecurse(edgedata, startoffset, newNextOffset, edgeisland, progress);
	}

	return true;
}

OP_ERROR 
SOP_Operator::StraightenEachEdgeIsland(UT_AutoInterrupt progress, fpreal time)
{	
	UT_Map<GA_Offset, UT_Vector3>	originalPositions;
	UT_Vector3Array					positions;
	UT_Map<GA_Offset, UT_Vector3>	edits;

	bool	setMorphState;
	fpreal	morphPowerState;
	bool	setUniformDistributionState;

	PRM_ACCESS::Get::IntPRM(this, setUniformDistributionState, UI::uniformDistributionToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, setMorphState, UI::setMorphToggle_Parameter, time);
	PRM_ACCESS::Get::FloatPRM(this, morphPowerState, UI::morphPowerFloat_Parameter, time);

	// convert from percentage
	morphPowerState = setMorphState ? 0.01 * morphPowerState : 1.0;

#define PROGRESS_ESCAPE(node, message, passedprogress) if (passedprogress.wasInterrupted()) { node->addError(SOP_ErrorCodes::SOP_MESSAGE, message); return error(); }	
	for (auto island : this->_edgeIslands)
	{
		PROGRESS_ESCAPE(this, "Operation interrupted", progress)
					
#ifdef DEBUG_ISLANDS
		island.Report();
#endif // DEBUG_ISLANDS		
		
		// ignore not correct ones
		if (!island.IsValid())
		{
			addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Edge islands with more than 2 endpoints detected.");
			continue;
		}

		// ignore single edge ones
		if (island.GetEdges().size() <= 1) continue;
		
		// store original positions		
		for (auto point : island.GetPoints())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			originalPositions[point] = this->gdp->getPos3(point);
		}

		// get each endpoint position	
		positions.clear();

		for (auto offset : island.GetEndPoints())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			positions.append(gdp->getPos3(offset));
		}
		
		// calculate projection
		const UT_Vector3 projection = positions[0] - positions[1];

		// straighten edges
		edits.clear();

		GUstraightenEdges(edits, *gdp, island.GetEdges(), &projection);
		for (auto edit : edits)
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)			 
			this->gdp->setPos3(edit.first, edit.second);
		}		

		// uniform distribution
		if (setUniformDistributionState)
		{
			edits.clear();		
		
			GUevenlySpaceEdges(edits, *gdp, island.GetEdges());
			for (auto edit : edits)
			{
				PROGRESS_ESCAPE(this, "Operation interrupted", progress)			
				this->gdp->setPos3(edit.first, edit.second);
			}	
		}

		// morph
		if (!setMorphState) continue;
		for (auto offset : island.GetPoints())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			auto newPos = SYSlerp(originalPositions[offset], this->gdp->getPos3(offset), morphPowerState);
			this->gdp->setPos3(offset, newPos);
		}
	}
#undef PROGRESS_ESCAPE

	return error();
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context &context)
{
	DEFAULTS_CookMySop()
		
	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{						
		// group cooking could pass, but we need to be sure that we have any groups specified at all
		auto success = this->_edgeGroupInput0 && !this->_edgeGroupInput0->isEmpty();
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE))
		{
			addWarning(SOP_ERR_BADGROUP);
			return error();
		}		

		// edge selection can contain multiple separate edge island, so before we find them, we need to find their endpoints, so we could have some starting point
		auto edgeData = GA_EdgeTData<GA_Edge>();
		success = ExtractDataFromEdges(edgeData, progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();		

		// once we have endpoints, we can go thru our edge group and split it on edge islands		
		success = FindAllEdgeIslands(edgeData, progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// finally, we can go thru each edge island and calculate and apply straighten
		return StraightenEachEdgeIsland(progress, currentTime);
	}

	return error();
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

MSS_Selector::~MSS_StraightenSelector() { }

MSS_Selector::MSS_StraightenSelector(OP3D_View& viewer, PI_SelectorTemplate& templ) 
: MSS_ReusableSelector(viewer, templ, SOP_SmallName, CONST_EdgeGroupInput0_Name, 0, true) 
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector* 
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ) 
{ return new MSS_Selector((OP3D_View&)viewer, templ); }

const char* 
MSS_Selector::className() const 
{ return "MSS_StraightenSelector"; }

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef PRM_ACCESS
#undef UI

#undef SOP_GroupFieldIndex_0

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Icon_Name
#undef SOP_Input_Name_0
#undef SOP_SmallName
#undef SOP_Operator