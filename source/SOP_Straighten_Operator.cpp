/*
	Use this as a starting template for any SOP operator.

	IMPORTANT! ------------------------------------------
	* External macros used:
		GET_SOP_Namespace()								- comes from "Macros_Namespace.h"
		GET_Base_Namespace()							- comes from "Macros_Namespace.h"
		
		PARAMETERLIST_Start()							- comes from "Macros_ParameterList.h"
		____THEN_Switch_VisibilityState()				- comes from "Macros_ParameterList.h"
		____THEN_SwitchInvert_VisibilityState()			- comes from "Macros_ParameterList.h"
		____THEN_SwitchButDontForce_VisibilityState()	- comes from "Macros_ParameterList.h"
		PARAMETERLIST_End()								- comes from "Macros_ParameterList.h"
		DEFAULTS_UpdateParmsFlags()						- comes from "Macros_UpdateParmsFlags.h"
		UPDATE_Switch_VisibilityState()					- comes from "Macros_UpdateParmsFlags.h"
		UPDATE_SwitchInvert_VisibilityState()			- comes from "Macros_UpdateParmsFlags.h"
		UPDATE_SwitchButDontForce_VisibilityState()		- comes from "Macros_UpdateParmsFlags.h"
		PARAMETERLIST_DescriptionPRM()					- comes from "Macros_DescriptionPRM.h"
		UPDATE_DescriptionPRM_ActiveState()				- comes from "Macros_DescriptionPRM.h"
		IMPLEMENT_DescriptionPRM_Callback()				- comes from "Macros_DescriptionPRM.h"
		DEFAULTS_CookMySop()							- comes from "Macros_CookMySop.h"
		
		IMPLEMENT_Set_IntPRM_V1()						- comes from "Macros_IntegerPRM.h"
		IMPLEMENT_Get_IntPRM_V2()						- comes from "Macros_IntegerPRM.h"		
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

#include <UT/UT_Interrupt.h>
#include <OP/OP_AutoLockInputs.h>
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_Error.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>

#include "SOP_Straighten_Parameters.h"
#include <SOP/Macros_ParameterList.h>
#include <Utility_ParameterAccessing.h>

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_Straighten_Operator
#define SOP_Input_Name_0	"Geometry"
#define SOP_Icon_Name		"SOP_Template.svg"
#define SOP_Base_Operator	SOP_Node

#define UI					GET_SOP_Namespace()::UI
#define PRM_ACCESS			GET_Base_Namespace()::Utility::PRM

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::mainSectionSwitcher_Parameter,
	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),
	UI::debugSectionSwitcher_Parameter,


____THEN_Switch_VisibilityState(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::mainSectionSwitcher_Parameter,
	UI::additionalSectionSwitcher_Parameter,
	UI::debugSectionSwitcher_Parameter,

____THEN_SwitchInvert_VisibilityState(SOP_Operator)
____THEN_SwitchButDontForce_VisibilityState(SOP_Operator)

PARAMETERLIST_End()

// update PRMs

auto SOP_Operator::updateParmsFlags()
-> bool
{ 
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// is input connected?
	exint is0Connected = getInput(0) == NULL ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = is0Connected ? 1 : 0;

	// update global parameters visibility
	UPDATE_Switch_VisibilityState(is0Connected)
	UPDATE_SwitchInvert_VisibilityState(is0Connected)
	UPDATE_SwitchButDontForce_VisibilityState(is0Connected)

	/* ---------------------------- Set States --------------------------------------- */

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, is0Connected, UI)

	return changed;
}

/* -----------------------------------------------------------------
INITIALIZATION                                                     |
----------------------------------------------------------------- */

SOP_Operator::~SOP_Straighten_Operator() { }
SOP_Operator::SOP_Straighten_Operator(OP_Network* network, const char* name, OP_Operator* op) : SOP_Base_Operator(network, name, op)
{ 
	op->setIconName(SOP_Icon_Name); 
	
	exint toggleFlag = 1;
	auto message = UT_String("Example operator that can be used as a starting point for developing custom SOP.");
	for (auto p : parametersList)
	{
		if (p.getNamePtr()->getToken() == UI::descriptionToggle_Parameter.getToken()) PRM_ACCESS::Set::IntPRM(this, toggleFlag, p);
		else if (p.getNamePtr()->getToken() == UI::descriptionTextField_Parameter.getToken()) PRM_ACCESS::Set::StringPRM(this, message, p);
	}	
}

auto SOP_Operator::CreateOperator(OP_Network* network, const char* name, OP_Operator* op) -> OP_Node* { return new SOP_Operator(network, name, op); }
auto SOP_Operator::inputLabel(unsigned input) const -> const char* { return SOP_Input_Name_0; }

/* -----------------------------------------------------------------
ADDITIONAL IMPLEMENTATIONS                                         |
----------------------------------------------------------------- */

// setup description PRM

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

//#include <DLLTest.h>
auto SOP_Operator::cookMySop(OP_Context& context)
-> OP_ERROR
{	
	DEFAULTS_CookMySop()

	//auto test = DLLTest::Functions::Add(2, 2);
	//std::cout << test << std::endl;

	return error();
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef PRM_ACCESS
#undef UI

#undef SOP_Base_Operator
#undef SOP_Icon_Name
#undef SOP_Input_Name_0
#undef SOP_Operator