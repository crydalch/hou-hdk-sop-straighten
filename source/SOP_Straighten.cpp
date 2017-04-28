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
#include <SOP/Macros_ParameterList.h>
#include <Utility_ParameterAccessing.h>

// this
#include "Parameters.h"
#include "GA_EdgeIsland.h"

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
	UI::straightenPowerFloat_Parameter,
	UI::uniformDistributionToggle_Parameter,
	UI::uniformDistributionSeparator_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

	UI::debugSectionSwitcher_Parameter,
	UI::reportEdgeIslandsToggle_Parameter,

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

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

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

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

bool 
SOP_Operator::FindAllEndPoints(UT_AutoInterrupt progress)
{
	// make sure nothing is cached
	this->_edgeExtractedData.clear();
	this->_endPoints.clear();

	// extract data from edges
	for (auto edgeIt = this->_edgeGroupInput0->begin(); !edgeIt.atEnd(); ++edgeIt)
	{
		if (progress.wasInterrupted()) return false;
		if (!edgeIt->isValid()) continue;

		this->_edgeExtractedData[edgeIt->p0()].insert(*edgeIt);
		this->_edgeExtractedData[edgeIt->p1()].insert(*edgeIt);
	}
	
	// collect endpoints	
	for (auto data : this->_edgeExtractedData)
	{
		if (progress.wasInterrupted()) return false;
		if (data.second.size() > 1) continue;
		
		this->_endPoints.append(data.first);
	}
	
	return true;
}

bool 
SOP_Operator::FindAllEdgeIslands(UT_AutoInterrupt progress)
{	
	// make sure nothing is cached
	this->_edgeIslands.clear();

	// find islands
	for (auto point : this->_endPoints)
	{
		if (progress.wasInterrupted()) return false;

		// check if island was already discovered
		bool canContinue = true;
		for (auto island : this->_edgeIslands)
		{
			if (island.Contains(point))
			{
				canContinue = false;
				break;
			}
		}		
		if (!canContinue) continue;

		// if not, play Vangelis - "Conquest of Paradise" and get it!
		auto edgeIsland = GA_EdgeIsland();		
		FindEdgesRecurse(point, point, edgeIsland, progress);	
		this->_edgeIslands.push_back(edgeIsland);			
	}

	return true;
}

void 
SOP_Operator::FindEdgesRecurse(const GA_Offset startoffset, const GA_Offset nextoffset, GA_EdgeIsland& edgeisland, UT_AutoInterrupt progress)
{
	// how many edges?
	auto edges = this->_edgeExtractedData[nextoffset];
	switch (edges.size())
	{
		case 0:
			{
				// this shouldn't ever happen, but who knows?! I don't :)
				std::cout << "WTF?" << std::endl;
			}
			break;
		case 1:
			{ WhenOneEdge(startoffset, nextoffset, edgeisland, progress); }
			break;
		default:
			{ WhenMoreThanOneEdge(startoffset, nextoffset, edgeisland, progress); }
			break;
	}
}

void 
SOP_Operator::WhenOneEdge(const GA_Offset startoffset, const GA_Offset nextoffset, GA_EdgeIsland& edgeisland, UT_AutoInterrupt progress)
{
	// store ourself
	edgeisland.AddEndPoint(nextoffset);

	// are we at start?
	if (nextoffset == startoffset)
	{
		// grab edge...
		auto edge = *this->_edgeExtractedData[nextoffset].begin();
		edgeisland.AddEdge(edge);

		// ... and process next point
		auto newNextOffset = edge.p0() == nextoffset ? edge.p1() : edge.p0();
		FindEdgesRecurse(startoffset, newNextOffset, edgeisland, progress);
	}
}

void 
SOP_Operator::WhenMoreThanOneEdge(const GA_Offset startoffset, const GA_Offset nextoffset, GA_EdgeIsland& edgeisland, UT_AutoInterrupt progress)
{
	// store ourself
	edgeisland.AddPoint(nextoffset);

	// grab edges
	auto edges = this->_edgeExtractedData[nextoffset];
	for (auto edge : edges)
	{
		if (progress.wasInterrupted()) return;

		// find point that != nextoffset and is not stored in edgeisland
		auto newNextOffset = edge.p0() == nextoffset ? edge.p1() : edge.p0();
		if (edgeisland.Contains(newNextOffset)) continue;

		// now we are sure that we didn't visited this branch, so we can add edge...
		edgeisland.AddEdge(edge);

		// ... and process next point
		FindEdgesRecurse(startoffset, newNextOffset, edgeisland, progress);
	}
}

OP_ERROR 
SOP_Operator::StraightenEachEdgeIsland(UT_AutoInterrupt progress, fpreal time /* = 0 */)
{	
	fpreal	straightenPower;
	bool	applyEvenEdgeLengths;
	bool	reportEdgeIslands;
	
	PRM_ACCESS::Get::FloatPRM(this, straightenPower, UI::straightenPowerFloat_Parameter, time);	
	PRM_ACCESS::Get::IntPRM(this, applyEvenEdgeLengths, UI::uniformDistributionToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, reportEdgeIslands, UI::reportEdgeIslandsToggle_Parameter, time);

#define PROGRESS_ESCAPE(node, message, passedprogress) if (passedprogress.wasInterrupted()) { node->addError(SOP_MESSAGE, message); return error(); }
	UT_Vector3Array positions;
	UT_Map<GA_Offset, UT_Vector3> edits;

	for (auto island : this->_edgeIslands)
	{
		PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			
		// debug island
		if (reportEdgeIslands) island.Report();
		
		// ignore not correct ones
		if (!island.IsValid()) continue;

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
			auto newPos = SYSlerp(this->gdp->getPos3(edit.first), edit.second, straightenPower);
			this->gdp->setPos3(edit.first, newPos);
		}						

		// evenly space edges		
		edits.clear();

		if (applyEvenEdgeLengths)
		{
			GUevenlySpaceEdges(edits, *gdp, island.GetEdges());
			for (auto edit : edits)
			{
				PROGRESS_ESCAPE(this, "Operation interrupted", progress)
				this->gdp->setPos3(edit.first, edit.second);
			}
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
		// first we need to get all edges selected by user
		auto success = this->_edgeGroupInput0 && !this->_edgeGroupInput0->isEmpty();
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// edge selection can contain multiple separate edge island
		// so before we find them, we need to find their endpoints, so we could have some starting point
		success = FindAllEndPoints(progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
		
		// once we have endpoints, we can go thru our edge group and split it on edge islands
		success = FindAllEdgeIslands(progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
		
		// finally, we can go thru each edge island...
		// ... and calculate and apply straighten
		StraightenEachEdgeIsland(progress);
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