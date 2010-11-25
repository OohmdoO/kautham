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
#include "mygridplanner.h"

using namespace libSampling;

namespace libPlanner {
  namespace gridplanner{
   namespace mygridplanner{
	//! Constructor
    MyGridPlanner::MyGridPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              gridPlanner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
		//set intial values
		_firstParameter = 10;
		_secondParameter = 0.5;
		_thirdParameter = 0.1;
	  
		_guiName = "My Grid Planner";
		addParameter("First parameter", _firstParameter);
		addParameter("Second parameter", _secondParameter);
		addParameter("Third parameter", _thirdParameter);
    }

	//! void destructor
	MyGridPlanner::~MyGridPlanner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool MyGridPlanner::setParameters(){
      try{
        HASH_S_K::iterator it = _parameters.find("Step Size");
		if(it != _parameters.end())
			setStepSize(it->second);//also changes stpssize of localplanner
        else
          return false;

        it = _parameters.find("Speed Factor");
        if(it != _parameters.end())
          _speedFactor = it->second;
        else
          return false;

        it = _parameters.find("Number Discretization Steps");
        if(it != _parameters.end())
		{
			_stepsDiscretization = it->second;
			_maxNumSamples = (int)pow((float)_stepsDiscretization, _wkSpace->getDimension());
			if(_isGraphSet) clearGraph();
			_samples->clear();
			discretizeCspace();
	    }
        else
          return false;


      }catch(...){
        return false;
      }
      return true;
    }

	
	  
	//! function to find a solution path
		bool MyGridPlanner::trySolve()
		{
			cout << "MyGridPlanner::trySolve - now a simple rectilinear connection without collision checking..."<<endl<<flush;
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

			
			_path.clear();
			clearSimulationPath();
			_path.push_back(_init);
			_path.push_back(_goal);
			_solved = true;
			return _solved;
			
		}
	  }
    }
}


