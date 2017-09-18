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

#pragma once

#ifndef ____parameters_h____
#define ____parameters_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
#include <Macros/GroupMenuPRM.h>
#include <Macros/FloatPRM.h>
#include <Macros/TogglePRM.h>
#include <Macros/ErrorLevelMenuPRM.h>
#include <Macros/SeparatorPRM.h>

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
		__DECLARE__Filter_Section_PRM(4)
		DECLARE_Default_EdgeGroup_Input_0_PRM(input0)		
		DECLARE_Custom_Separator_PRM("filtererrorsseparator", filterErrors)
		DECLARE_ErroLevelMenu_PRM("groupnotspecifiederrormode", "Group Not Specified", 1, "Specify group not specified node error mode.", groupNotSpecified)
		DECLARE_ErroLevelMenu_PRM("improperedgeislanderrormode", "Improper Edge Island", 1, "Specify improper edge island detection node error mode.", improperEdgeIsland)

		__DECLARE_Main_Section_PRM(2)		
		DECLARE_Toggle_with_Separator_OFF_PRM("setuniformpointdistribution", "Uniform Point Distribution", "setuniformpointdistributionseparator", 0, "Uniformly distribute points to create even length edges.", uniformDistribution)		

		__DECLARE_Additional_Section_PRM(7)		
		DECLARE_Toggle_with_Separator_OFF_PRM("setmorph", "Morph", "setmorphseparator", &SOP_Operator::CallbackSetMorph, "Blend between original and modified position.", setMorph)
		DECLARE_Custom_Float_MinR_to_MaxU_PRM("morphpower", "Power", 0, 100, 100, 0, "Specify morph amount.", morphPower)
		
		DECLARE_DescriptionPRM(SOP_Operator)
	}

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator

#endif // !____parameters_h____