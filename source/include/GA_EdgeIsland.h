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

#define pragma once

#ifndef ____ga_edgeisland_h____
#define ____ga_edgeisland_h____

struct GA_EdgeIsland
{
	GA_EdgeIsland()
	{
		this->_edges.clear();
		this->_points.clear();
		this->_endPoints.clear();
	}
	~GA_EdgeIsland() {}

	void						AddEdge(GA_Edge edge) { this->_edges.insert(edge); }
	void						AddEdges(UT_Set<GA_Edge> edges) { for (auto edge : edges) this->_edges.insert(edge); }
	void						AddPoint(GA_Offset offset) { this->_points.insert(offset); }
	void						AddEndPoint(GA_Offset offset) { this->_endPoints.insert(offset); this->_points.insert(offset); }
	bool						Contains(GA_Edge edge) const { return this->_edges.contains(edge); }
	bool						Contains(GA_Offset offset) const { return this->_endPoints.contains(offset) || this->_points.contains(offset); }
	bool						IsValid() const { return this->_endPoints.size() == 2; }

	const UT_Set<GA_Edge>		GetEdges() { return this->_edges; }
	const UT_Set<GA_Offset>		GetPoints() { return this->_points; }
	const UT_Set<GA_Offset>		GetEndPoints() { return this->_endPoints; }

	void Report() const
	{
		std::cout << "Island state: " << (this->IsValid() ? "VALID" : "INVALID") << std::endl;
		std::cout << "No. of edges: " << this->_edges.size() << std::endl;
		std::cout << "Points: [ ";
		for (auto p : this->_points) std::cout << p << " ";
		std::cout << "]" << std::endl;
		std::cout << "EndPoints: [ ";
		for (auto p : this->_endPoints) std::cout << p << " ";
		std::cout << "]" << std::endl;		
		std::cout << " --------------- " << std::endl;
	}

private:
	UT_Set<GA_Edge>				_edges;
	UT_Set<GA_Offset>			_points;
	UT_Set<GA_Offset>			_endPoints;
};

#endif // !____ga_edgeisland_h____