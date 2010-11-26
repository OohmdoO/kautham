/***************************************************************************
*               Generated by StarUML(tm) C++ Add-In                        *
***************************************************************************/
/***************************************************************************
*                                                                          *
*           Institute of Industrial and Control Engineering                *
*                 Technical University of Catalunya                        *
*                        Barcelona, Spain                                  *
*                                                                          *
*                Project Name:       Kautham Planner                       *
*                                                                          *
*     Copyright (C) 2007 - 2009 by Alexander Pérez and Jan Rosell          *
*            alexander.perez@upc.edu and jan.rosell@upc.edu                *
*                                                                          *
*             This is a motion planning tool to be used into               *
*             academic environment and it's provided without               *
*                     any warranty by the authors.                         *
*                                                                          *
*          Alexander Pérez is also with the Escuela Colombiana             *
*          de Ingeniería "Julio Garavito" placed in Bogotá D.C.            *
*             Colombia.  alexander.perez@escuelaing.edu.co                 *
*                                                                          *
***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 

#if !defined(_MYGRIDPLANNER_H)
#define _MYGRIDPLANNER_H


#include <boost/graph/visitors.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>

#include <libproblem/workspace.h>
#include <libsampling/sampling.h>
#include "localplanner.h"
#include "planner.h"
#include "gridplanner.h"

using namespace std;
using namespace libSampling;

namespace libPlanner {
  namespace gridplanner{
   namespace mygridplanner{

    //CLASS bfs_distance_visitor
    // visitor that terminates when we find the goal
    template <class DistanceMap>
    class bfs_distance_visitor : public boost::default_bfs_visitor 
	{
		public:
			bfs_distance_visitor(DistanceMap dist) : d(dist) {};

			template <typename Edge, typename Graph> 
			void tree_edge(Edge e, Graph& g)
			{
				typename graph_traits<Graph>::vertex_descriptor s=source(e,g);
				typename graph_traits<Graph>::vertex_descriptor t=target(e,g);
				d[t] = d[s] + 1;
				//potmap[t] = d[t];
			}

		private:
			DistanceMap d;
    };

    class MyGridPlanner:public gridPlanner {
	    public:
        MyGridPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, 
          WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize);
        ~MyGridPlanner();
        
		bool trySolve();
		bool setParameters();
		//Add public data and functions
		

		protected:
		//Add protected data and functions
		int _firstParameter;
		double _secondParameter;
		double _thirdParameter;	

		
	    private:
		//Add private data and functions	
		void computeNF1(gridVertex  vgoal);

	  };
   }
  }
}

#endif  //_MYGRIDPLANNER_H

