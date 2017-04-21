/*
	Use this as a starting template for any SOP operator.

	IMPORTANT! ------------------------------------------
	* External macros used:
		GET_SOP_Namespace()				- comes from "Macros_Namespace.h"
		DECLARE_SOP_Namespace_Start()	- comes from "Macros_Namespace.h"
		DECLARE_SOP_Namespace_End		- comes from "Macros_Namespace.h"	
		__FILTER_SECTION_PRM()
		__MAIN_SECTION_PRM()
		__ADDITIONAL_SECTION_PRM()
		DECLARE_DescriptionPRM()		- comes from "Macros_DescriptionPRM.h"
		__DEBUG_SECTION_PRM()	
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

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

#include "SOP_Straighten_Operator.h"
#include <SOP/Macros_SwitcherPRM.h>

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator GET_SOP_Namespace()::SOP_Straighten_Operator

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

// YOUR CODE GOES HERE...

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace UI
	{
		__DECLARE__Filter_Section_PRM(0)
		__DECLARE_Main_Section_PRM(0)

		__DECLARE_Additional_Section_PRM(4)
		DECLARE_DescriptionPRM(SOP_Operator)

		__DECLARE_Debug_Section_PRM(0)
	}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator