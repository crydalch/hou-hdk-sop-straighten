/*
	Use this as a starting template for any SOP operator.

	IMPORTANT! ------------------------------------------
	* External macros used:
		DECLARE_SOP_Namespace_Start()					- comes from "Macros_Namespace.h"
		DECLARE_SOP_Namespace_End						- comes from "Macros_Namespace.h"
		DECLARE_DescriptionPRM_Callback()				- comes from "Macros_DescriptionPRM.h"
		DECLARE_Switch_VisibilityState()				- comes from "Macros_UpdateParmsFlags.h"
		DECLARE_SwitchInvert_VisibilityState()			- comes from "Macros_UpdateParmsFlags.h"
		DECLARE_SwitchButDontForce_VisibilityState()	- comes from "Macros_UpdateParmsFlags.h"
		DECLARE_UpdateParmsFlags()						- comes from "Macros_UpdateParmsFlags.h"
		DECLARE_CookMySop()								- comes from "Macros_CookMySop.h"
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

#include <SOP/SOP_Node.h>

#include <SOP/Macros_CookMySop.h>
#include <SOP/Macros_DescriptionPRM.h>
#include <SOP/Macros_Namespace.h>
#include <SOP/Macros_UpdateParmsFlags.h>

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_Straighten_Operator : public SOP_Node
	{
		DECLARE_CookMySop()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()
		DECLARE_Switch_VisibilityState()
		DECLARE_SwitchInvert_VisibilityState()
		DECLARE_SwitchButDontForce_VisibilityState()

	protected:
		virtual ~SOP_Straighten_Operator() override;
		SOP_Straighten_Operator(OP_Network* network, const char* name, OP_Operator* op);

	public:
		static OP_Node*			CreateOperator(OP_Network* network, const char* name, OP_Operator* op);

		static					PRM_Template parametersList[];

	private:
		virtual const char*		inputLabel(unsigned input) const override;
	};

DECLARE_SOP_Namespace_End