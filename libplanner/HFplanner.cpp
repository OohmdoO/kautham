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
#include "HFplanner.h"

using namespace libSampling;

namespace libPlanner {
  namespace gridplanner{
   namespace HF_planner{
	//! Constructor
    HFPlanner::HFPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              gridPlanner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
		//set intial values
	  
		_guiName = "HF Planner";
		removeParameter("Max. Samples");
		removeParameter("Step Size");
    }

	//! void destructor
	HFPlanner::~HFPlanner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool HFPlanner::setParameters(){
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

	

	void HFPlanner::computeHF(gridVertex  vgoal)
	{
		//initialize potential to -1 and goal to 0
		setPotential(vgoal, _goalPotential);
		//relax potential
		graph_traits<gridGraph>::vertex_iterator vi, vend;
		graph_traits<gridGraph>::adjacency_iterator avi, avi_end;
		for(int i=0; i<10; i++)
		{
			for(tie(vi,vend)=vertices(*g); vi!=vend; ++vi)
			{
				if(getPotential(*vi) == _goalPotential ||
				   getPotential(*vi) == _obstaclePotential) continue;
				

				KthReal p=0;
				int count=0;
				for(tie(avi,avi_end)=adjacent_vertices(*vi, *g); avi!=avi_end; ++avi)
				{
					count++;
					p+=getPotential(*avi);
				}
				setPotential(*vi, p/count);
			}
		}
	}

	  
	//! function to find a solution path
		bool HFPlanner::trySolve()
		{
			//verify init and goal samples
			if(goalSamp()->isFree()==false || initSamp()->isFree()==false) 
			{
				cout<<"init or goal configuration are in COLLISION!"<<endl;
				return false;
			}

			//set init and goal vertices
			gridVertex  vg = _samples->indexOf(goalSamp());
			gridVertex  vi = _samples->indexOf(initSamp());
			

			Sample* curr;
			gridVertex vc;
			gridVertex vmin;

			curr = _init;
			vc = vi;

			_path.clear();
			clearSimulationPath();
			graph_traits<gridGraph>::adjacency_iterator avi, avi_end;

			//relax HF
			computeHF(vg);
			int count = 0;
			int countmax = 100;
			std::vector<int> vpath;
			vpath.push_back(vi);
			_path.push_back(locations[vi]);

			while(vc != vg && count < countmax)
			{
				vmin = vc;
				for(tie(avi,avi_end)=adjacent_vertices(vc, *g); avi!=avi_end; ++avi)
				{
					KthReal pneigh = getPotential(*avi);
					KthReal pcurr = getPotential(vmin);
					if(pneigh < pcurr) {
						vmin = *avi; 
						vpath.push_back(vmin);
						_path.push_back(locations[vmin]);
					}
				}
				if(vc == vmin) {
					//relax HF again and resume
					computeHF(vg);
					_path.clear();
					vpath.clear();
					vc = vi;
					count++;
				}
				else vc = vmin;
			}
			if(count < countmax) _solved = true;
			else
			{
				_path.clear();
				vpath.clear();
				_solved = false;
			}
			if(_solved)
			{
				cout<<"PATH:";
				for(int i=0;i<vpath.size();i++)
				{
					cout<<" "<<vpath[i]<<"("<<getPotential(vpath[i])<<"), ";
				}
				cout<<endl;
			}

			return _solved;
		}
	  }
    }
}


