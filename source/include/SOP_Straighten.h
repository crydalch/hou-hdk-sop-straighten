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

#define pragma once

#ifndef ____sop_straighten_h____
#define ____sop_straighten_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <SOP/SOP_Node.h>
#include <MSS/MSS_ReusableSelector.h>

// hou-hdk-common
#include <Macros/CookMySop.h>
#include <Macros/DescriptionPRM.h>
#include <Macros/Namespace.h>
#include <Macros/UpdateParmsFlags.h>

#include <Containers/GA_EdgeTData.h>
#include <Containers/GA_EdgeTIsland.h>

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

// uncomment to print island debug info
//#define DEBUG_ISLANDS

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;

/* -----------------------------------------------------------------
OPERATOR DECLARATION                                               |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_Straighten : public SOP_Node
	{
		DECLARE_CookMySop()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()

	private:
		static PRM_Template _switchActiveState[];

	protected:
		~SOP_Straighten() override;
		SOP_Straighten(OP_Network* network, const char* name, OP_Operator* op);		
		const char*							inputLabel(unsigned input) const override;

	public:		
		static OP_Node*						CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		OP_ERROR							cookInputGroups(OP_Context& context, int alone = 0) override;
		static PRM_Template					parametersList[];
		
		static int							CallbackSetMorph(void* data, int index, float time, const PRM_Template* tmp);

	private:
		OP_ERROR							StraightenEachEdgeIsland(GA_EdgeIslandBundle& edgeislands, UT_AutoInterrupt progress, fpreal time);

		const GA_EdgeGroup*					_edgeGroupInput0;		
	};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
SELECTOR DECLARATION                                               |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class MSS_StraightenSelector : public MSS_ReusableSelector
	{
	public:
		virtual ~MSS_StraightenSelector();
		MSS_StraightenSelector(OP3D_View& viewer, PI_SelectorTemplate& templ);

		static BM_InputSelector*			CreateMe(BM_View& Viewer, PI_SelectorTemplate& templ);
		const char*							className() const override;
	};

DECLARE_SOP_Namespace_End

#endif // !____sop_straighten_h____