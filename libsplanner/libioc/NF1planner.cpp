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
 
 


#include <libproblem/workspace.h>
#include <libsampling/sampling.h>
#include "localplanner.h"
#include "NF1planner.h"

using namespace libSampling;

namespace libPlanner {
  namespace IOC{
	//! Constructor
    NF1Planner::NF1Planner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              gridPlanner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
		//set intial values
	  
        _guiName = _idName =  "NF1Planner";
		removeParameter("Max. Samples");
		removeParameter("Step Size");
    }

	//! void destructor
	NF1Planner::~NF1Planner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool NF1Planner::setParameters(){
      try{
        HASH_S_K::iterator it = _parameters.find("Speed Factor");
        if(it != _parameters.end())
          _speedFactor = it->second;
        else
          return false;

		char *str = new char[20];
		for(int i=0; i<_wkSpace->getDimension();i++)
		{
			sprintf(str,"Discr. Steps %d",i);
			it = _parameters.find(str);
			if(it != _parameters.end())
			{
				setStepsDiscretization(it->second,i);
			}
			else
				return false;
		}


      }catch(...){
        return false;
      }
      return true;
    }

	

	void NF1Planner::computeNF1(gridVertex  vgoal)
	{

		graph_traits<gridGraph>::vertex_iterator vi, vi_end;

		/* Prints adjacencies 

		graph_traits<gridGraph>::adjacency_iterator avi, avi_end;
		for(tie(vi,vi_end)=vertices(*g); vi!=vi_end; ++vi)
		{
			cout<<*vi<< " adjacent to: ";
			for(tie(avi,avi_end)=adjacent_vertices(*vi, *g); avi!=avi_end; ++avi)
			{
				cout<<*avi<<" ,";
			}
			cout<<endl;
		}
		*/

		//initialize potential to -1 and goal to 0
		for(tie(vi,vi_end)=vertices(*g); vi!=vi_end; ++vi)	setPotential(*vi, -1);
		setPotential(vgoal, 0);
		//propagate potential
		breadth_first_search(*fg, vgoal, visitor(bfs_distance_visitor<PotentialMap>(getpotmat())));

		graph_traits<filteredGridGraph>::vertex_iterator i, end;
		for(tie(i,end)=vertices(*fg); i!=end; ++i)
		{
			cout<<"vertex "<< *i<<" dist "<<getPotential(*i)<<endl;
		}
	}

	  
	//! function to find a solution path
		bool NF1Planner::trySolve()
		{
			//cout << "MyGridPlanner::trySolve - now a simple rectilinear connection without collision checking..."<<endl<<flush;

			if(goalSamp()->isFree()==false || initSamp()->isFree()==false) 
			{
				cout<<"init or goal configuration are in COLLISION!"<<endl;
				return false;
			}

			gridVertex  vg = _samples->indexOf(goalSamp());
			gridVertex  vi = _samples->indexOf(initSamp());

			static int svg = -1;
			
			if(svg == -1 || svg!=vg){
				//recompute navigation function because goal has changed
				svg = vg;
				computeNF1(vg);
			}
			
			Sample* curr;
			gridVertex vc;
			gridVertex vmin;

			curr = _init;
			vc = vi;

			
			PotentialMap pm = getpotmat();

			//if navigation function did'nt arrive at initial cell, return false
			if(pm[vi] == -1) 
			{
				cout<<"CONNECTION NOT POSSIBLE: Init and goal configurations not on the same connected component..."<<endl;
				drawCspace();
				return false;
			}

			//otherwise follow the negated values
			_path.clear();
			clearSimulationPath();
			graph_traits<filteredGridGraph>::adjacency_iterator avi, avi_end;

			while(vc != vg)
			{
				_path.push_back(locations[vc]);
				vmin = vc;
				for(tie(avi,avi_end)=adjacent_vertices(vc, *fg); avi!=avi_end; ++avi)
				{
					KthReal pneigh = pm[*avi];
					KthReal pcurr = pm[vmin];
					if(pneigh < pcurr) vmin = *avi; 
					//if(pm[*avi] < pm[vmin]) vmin = *avi; 
				}
				vc = vmin;
			}
			_path.push_back(locations[vg]);
			_solved = true;
			drawCspace();
			return _solved;
			/*
			_path.clear();
			clearSimulationPath();
			_path.push_back(_init);
			_path.push_back(_goal);
			_solved = true;
			return _solved;
			*/

			/*
			The available resorces to implement your planner are:
			1) Grid represented as a graph, already created in parent class gridplanner. It contains
				both free and collision samples.

			2) setPotential(int i, KthReal value) and getPotential(int i): functions to set and get the values of 
				the potential function.

			3) As a derived class from planner, also the following is available:
				3a) A sampler to obtain more samples:
					Sample *smp;
					smp = _sampler->nextSample();
			      or a way to determine new sample at a given configuration, e.g.:
					Sample *smp = new Sample();
					KthReal* coords = new KthReal[_wkSpace->getDimension()];
					for(int k = 0; k < _wkSpace->getDimension(); k++) coords[k] = 0.0;
					smp->setCoords(coords);

			   3b) A collision-checker to check for collision at a given sample smp:
					_wkSpace->collisionCheck(smp)

			   3c) A local planner to connect two samples
					_locPlanner->canConect();

			The solution must be specified as a sequence of samples (vector _path)
		  	*/

			
			
		}
	  }
}


