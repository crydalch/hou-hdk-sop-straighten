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

#pragma once

#ifndef ____parameters_h____
#define ____parameters_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <SOP/Macros_SwitcherPRM.h>
#include <SOP/Macros_GroupMenuPRM.h>
#include <SOP/Macros_FloatPRM.h>
#include <SOP/Macros_TogglePRM.h>

// this
#include "SOP_Straighten.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator GET_SOP_Namespace()::SOP_Straighten

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace UI
	{
		__DECLARE__Filter_Section_PRM(1)
		DECLARE_Default_EdgeGroup_Input_0_PRM(input0)

		__DECLARE_Main_Section_PRM(3)
		DECLARE_Custom_Float_0R_to_1R_PRM("straightenpower", "Power", 1, "How much straighten apply.", straightenPower)						
		DECLARE_Custom_Toggle_with_Separator_OFF_PRM("setuniformpointdistribution", "Uniform Point Distribution", "setuniformpointdistributionseparator", 0, "Uniformly distribute points to create even length edges.", uniformDistribution)

		__DECLARE_Additional_Section_PRM(4)		
		DECLARE_DescriptionPRM(SOP_Operator)

		__DECLARE_Debug_Section_PRM(1)
		DECLARE_Custom_Toggle_with_Separator_OFF_PRM("reportedgeislands", "Report Edge Islands", "reportedgeislandsseparator", 0, "Print information about each detected edge island.", reportEdgeIslands)
	}

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator

#endif // !____parameters_h____