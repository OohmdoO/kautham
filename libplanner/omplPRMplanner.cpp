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
 
 

#if defined(KAUTHAM_USE_OMPL)
#include <libproblem/workspace.h>
#include <libsampling/sampling.h>

#include <boost/bind/mem_fn.hpp>

#include "omplPRMplanner.h"




using namespace libSampling;

namespace libPlanner {
  namespace omplplanner{

	//! Constructor
    omplPRMPlanner::omplPRMPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              omplPlanner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
        _guiName = "ompl PRM Planner";
        _idName = "omplPRM";


        ss = ((og::SimpleSetupPtr) new og::SimpleSetup(space));
        ss->setStateValidityChecker(boost::bind(&omplplanner::isStateValid, _1, (Planner*)this));
        ob::SpaceInformationPtr si=ss->getSpaceInformation();

        //si->setValidStateSamplerAllocator(boost::bind(&omplplanner::allocValidStateSampler, _1, (Planner*)this));
        //space->setStateSamplerAllocator(boost::bind(&omplplanner::allocStateSampler, _1, (Planner*)this));


        ob::PlannerPtr planner(new og::PRM(si));
        ss->setPlanner(planner);

        _MaxNearestNeighbors=10;
        addParameter("MaxNearestNeighbors", _MaxNearestNeighbors);
        _expandRoadmap=0.0;
        addParameter("Expand Roadmap (s)", _expandRoadmap);

        ss->getPlanner()->as<og::PRM>()->setMaxNearestNeighbors(_MaxNearestNeighbors);
        ss->getPlanner()->as<og::PRM>()->expandRoadmap(_expandRoadmap);

    }

	//! void destructor
    omplPRMPlanner::~omplPRMPlanner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool omplPRMPlanner::setParameters(){

      omplPlanner::setParameters();
      try{
        HASH_S_K::iterator it = _parameters.find("MaxNearestNeighbors");
        if(it != _parameters.end()){
          _MaxNearestNeighbors = it->second;
          ss->getPlanner()->as<og::PRM>()->setMaxNearestNeighbors(_MaxNearestNeighbors);
        }
        else
          return false;

        it = _parameters.find("Expand Roadmap (s)");
        if(it != _parameters.end()){
            _expandRoadmap = it->second;
            ss->getPlanner()->as<og::PRM>()->expandRoadmap(_expandRoadmap);
      }
        else
          return false;

      }catch(...){
        return false;
      }
      return true;
    }
  }
}

#endif // KAUTHAM_USE_OMPL

