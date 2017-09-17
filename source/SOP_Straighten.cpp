/*
	Tool that straightens selected edges.

	IMPORTANT! ------------------------------------------
	-----------------------------------------------------

	Author: 	SWANN
	Email:		sebastianswann@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SWANN
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
#include <Utility/EdgeGroupAccessing.h>

// this
#include "Parameters.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_Straighten
#define SOP_SmallName			"modeling::straighten::1.0"
#define SOP_Input_Name_0		"Geometry"
#define SOP_Icon_Name			"nodeway_short_dark_WB.png"
#define SOP_Base_Operator		SOP_Node
#define MSS_Selector			GET_SOP_Namespace()::MSS_StraightenSelector

// very important
#define SOP_GroupFieldIndex_0	1

#define UI						GET_SOP_Namespace()::UI
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define GRP_ACCESS				GET_Base_Namespace()::Utility::Groups

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::input0EdgeGroup_Parameter,
	UI::edgeIslandErrorModeChoiceMenu_Parameter,

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
	exint is0Connected = getInput(0) == nullptr ? 0 : 1;

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

SOP_Operator::SOP_Straighten(OP_Network* network, const char* name, OP_Operator* op) 
: SOP_Base_Operator(network, name, op), 
_edgeGroupInput0(nullptr)
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

OP_ERROR 
SOP_Operator::StraightenEachEdgeIsland(GA_EdgeIslandBundle& edgeislands, UT_AutoInterrupt progress, fpreal time)
{	
	UT_Map<GA_Offset, UT_Vector3>	originalPositions;
	UT_Map<GA_Offset, UT_Vector3>	edits;

	bool							setMorphState;
	fpreal							morphPowerState;
	bool							setUniformDistributionState;
	exint							edgeIslandErrorLevelState;

	PRM_ACCESS::Get::IntPRM(this, setUniformDistributionState, UI::uniformDistributionToggle_Parameter, time);

	PRM_ACCESS::Get::IntPRM(this, setMorphState, UI::setMorphToggle_Parameter, time);
	PRM_ACCESS::Get::FloatPRM(this, morphPowerState, UI::morphPowerFloat_Parameter, time);	
	morphPowerState = setMorphState ? 0.01 * morphPowerState : 1.0; // convert from percentage
	
	PRM_ACCESS::Get::IntPRM(this, edgeIslandErrorLevelState, UI::edgeIslandErrorModeChoiceMenu_Parameter, time);

#define PROGRESS_ESCAPE(node, message, passedprogress) if (passedprogress.wasInterrupted()) { node->addError(SOP_ErrorCodes::SOP_MESSAGE, message); return error(); }	
	for (auto island : edgeislands)
	{
		PROGRESS_ESCAPE(this, "Operation interrupted", progress)
					
#ifdef DEBUG_ISLANDS
		island.Report();
#endif // DEBUG_ISLANDS		
		
		// ignore not correct ones
		if (!island.IsValid())
		{
			switch (edgeIslandErrorLevelState)
			{
				default: /* do nothing */ continue;
				case static_cast<exint>(HOU_NODE_ERROR_LEVEL::Warning) : { addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Edge islands with more than 2 endpoints detected."); } continue;
				case static_cast<exint>(HOU_NODE_ERROR_LEVEL::Error) : { addError(SOP_ErrorCodes::SOP_MESSAGE, "Edge islands with more than 2 endpoints detected."); } return error();				
			}
		}

		// ignore single edge ones
		if (island.GetEdges().size() <= 1) continue;
		
		// store original positions		
		auto it = island.Begin();
		for (it; !it.atEnd(); it.advance())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			originalPositions[*it] = this->gdp->getPos3(*it);
		}

		// calculate direction
		auto direction = this->gdp->getPos3(island.Last()) - this->gdp->getPos3(island.First());
		direction.normalize();
		
		// straighten edges
		edits.clear();

		GUstraightenEdges(edits, *gdp, island.GetEdges(), &direction);
		for (const auto edit : edits)
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)			 
			this->gdp->setPos3(edit.first, edit.second);
		}		

		// uniform distribution
		if (setUniformDistributionState)
		{		
			// if anyone wonders why I didn't used GUevenlySpaceEdges to do this, my algorithm works better, SESI version fails in some situations			
			/*
			edits.clear();		
		
			GUevenlySpaceEdges(edits, *gdp, island.GetEdges());
			for (auto edit : edits)
			{
				PROGRESS_ESCAPE(this, "Operation interrupted", progress)			
				this->gdp->setPos3(edit.first, edit.second);
			}*/

			const auto distance = (this->gdp->getPos3(island.Last()) - this->gdp->getPos3(island.First())).length() / (island.Entries() - 1);

			UT_Vector3 currentPosition;					
			exint multiplier = 0;

			it = island.Begin();
			for (it; !it.atEnd(); it.advance())
			{
				if (multiplier == 0)
				{
					currentPosition = this->gdp->getPos3(*it);
					multiplier++;
				}

				// skip first and last point
				if (*it == island.First() || *it == island.Last()) continue;

				const auto newPosition = currentPosition + (direction * (distance * multiplier));
				this->gdp->setPos3(*it, newPosition);

				multiplier++;
			}
		}

		// morph
		if (!setMorphState) continue;

		it = island.Begin();
		for (it; !it.atEnd(); it.advance())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			const auto newPos = SYSlerp(originalPositions[*it], this->gdp->getPos3(*it), morphPowerState);
			this->gdp->setPos3(*it, newPos);
		}
	}
#undef PROGRESS_ESCAPE

	return error();
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */
#include <Enums/NodeErrorLevel.h>

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
			clearSelection();
			addWarning(SOP_ErrorCodes::SOP_ERR_BADGROUP);
			return error();
		}		

		// edge selection can contain multiple separate edge island, so before we find them, we need to find their endpoints, so we could have some starting point		
		auto edgeData = GA_EdgesData();
		edgeData.Clear();

		success = GRP_ACCESS::Edge::Break::PerPoint(this, this->_edgeGroupInput0, edgeData, progress);		
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();		

		// once we have endpoints, we can go thru our edge group and split it on edge islands
		auto edgeIslands = GA_EdgeIslandBundle();
		edgeIslands.clear();

		success = GRP_ACCESS::Edge::Break::PerIsland(this, edgeData, edgeIslands, EdgeIslandType::OPEN, progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// finally, we can go thru each edge island and calculate and apply straighten
		return StraightenEachEdgeIsland(edgeIslands, progress, currentTime);
	}

	return error();
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

MSS_Selector::~MSS_StraightenSelector() { }

MSS_Selector::MSS_StraightenSelector(OP3D_View& viewer, PI_SelectorTemplate& templ) 
: MSS_ReusableSelector(viewer, templ, SOP_SmallName, CONST_EdgeGroupInput0_Name, nullptr, true) 
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector* 
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ) 
{ return new MSS_Selector(reinterpret_cast<OP3D_View&>(viewer), templ); }

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