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
#include <ompl/base/spaces/SE2StateSpace.h>

#include "omplcRRTplanner.h"




using namespace libSampling;

namespace libPlanner {
  namespace omplcplanner{

  void propagate(const ob::State *start, const oc::Control *control, const double duration, ob::State *result)
  {
      /*
      const ob::SE2StateSpace::StateType *se2state = start->as<ob::SE2StateSpace::StateType>();
      const ob::RealVectorStateSpace::StateType *pos = se2state->as<ob::RealVectorStateSpace::StateType>(0);
      const ob::SO2StateSpace::StateType *rot = se2state->as<ob::SO2StateSpace::StateType>(1);
      const oc::RealVectorControlSpace::ControlType *rctrl = control->as<oc::RealVectorControlSpace::ControlType>();

      result->as<ob::SE2StateSpace::StateType>()->as<ob::RealVectorStateSpace::StateType>(0)->values[0] =
          (*pos)[0] + (*rctrl)[0] * duration * cos(rot->value);

      result->as<ob::SE2StateSpace::StateType>()->as<ob::RealVectorStateSpace::StateType>(0)->values[1] =
          (*pos)[1] + (*rctrl)[0] * duration * sin(rot->value);

      result->as<ob::SE2StateSpace::StateType>()->as<ob::SO2StateSpace::StateType>(1)->value =
          rot->value + (*rctrl)[1];
      */
      const ob::RealVectorStateSpace::StateType *R2state = start->as<ob::RealVectorStateSpace::StateType>();
      const oc::RealVectorControlSpace::ControlType *rctrl = control->as<oc::RealVectorControlSpace::ControlType>();

      //smp->setCoords(coords);
      //vector<RobConf*>& rconf = getConfigMapping(smp);


      vector<KthReal> coordsresult;
      coordsresult.resize(3);

      double du;
      du=duration;
      //if(duration>0) du=duration;
      //else du = -duration;

      coordsresult[0] = R2state->values[0] + (*rctrl)[0] * duration * cos(R2state->values[2]*2*M_PI);
      //if(coordsresult[0]>1.0) coordsresult[0] = 1.0;
      //else if (coordsresult[0]<0.0) coordsresult[0] = 0.0;
      coordsresult[1] = R2state->values[1] + (*rctrl)[0] * duration * sin(R2state->values[2]*2*M_PI);
      //if(coordsresult[1]>1.0) coordsresult[1] = 1.0;
      //else if (coordsresult[1]<0.0) coordsresult[1] = 0.0;
      coordsresult[2] = (R2state->values[2]*2*M_PI + (*rctrl)[1]/1.0)/(2*M_PI);
      if(coordsresult[2]<0.0)
      {
          coordsresult[2] = 1.0 - coordsresult[2];
      }
      else if(coordsresult[2]>1.0)
      {
          coordsresult[2] = coordsresult[2] - 1.0;
      }

      result->as<ob::RealVectorStateSpace::StateType>()->values[0] = coordsresult[0];
      result->as<ob::RealVectorStateSpace::StateType>()->values[1] = coordsresult[1];
      result->as<ob::RealVectorStateSpace::StateType>()->values[2] = coordsresult[2];

  }


	//! Constructor
    omplcRRTPlanner::omplcRRTPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              omplcPlanner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
        _guiName = "ompl cRRT Planner";
        _idName = "ompl cRRT Planner";

  // construct the state space we are planning in
 // space =  ((ob::StateSpacePtr) new ob::SE2StateSpace());

  // set the bounds for the R^2 part of SE(2)
  //ob::RealVectorBounds bounds(2);
  //bounds.setLow(0);
  //bounds.setHigh(1);

  //space->as<ob::SE2StateSpace>()->setBounds(bounds);


        // create a control space
        spacec = ((oc::ControlSpacePtr) new oc::RealVectorControlSpace(space, 2));


        // set the bounds for the control space
        _controlBound_Tras = 0.3;
        _controlBound_Rot = 0.03;
        _onlyForward = 1;
        addParameter("ControlBound_Tras", _controlBound_Tras);
        addParameter("ControlBound_Rot", _controlBound_Rot);
        addParameter("OnlyForward (0/1)", _onlyForward);
        ob::RealVectorBounds cbounds(2);
        cbounds.setLow(0, - _controlBound_Tras * (1 - _onlyForward));
        cbounds.setHigh(0, _controlBound_Tras);
        cbounds.setLow(1, -_controlBound_Rot);
        cbounds.setHigh(1, _controlBound_Rot);
        spacec->as<oc::RealVectorControlSpace>()->setBounds(cbounds);

        ss = ((oc::SimpleSetupPtr) new oc::SimpleSetup(spacec));
        ss->setStateValidityChecker(boost::bind(&omplcplanner::isStateValid, ss->getSpaceInformation().get(), _1, (Planner*)this));

        // set the state propagation routine
        ss->setStatePropagator(boost::bind(&propagate, _1, _2, _3, _4));
        oc::SpaceInformationPtr si=ss->getSpaceInformation();


        _propagationStepSize = 1.0;
        addParameter("PropagationStepSize", _propagationStepSize);
        si->setPropagationStepSize(_propagationStepSize);

        _minControlDuration = 1;
        _maxControlDuration = 10;
        addParameter("MinControlDuration", _minControlDuration);
        addParameter("MaxControlDuration", _maxControlDuration);
        si->setMinMaxControlDuration(_minControlDuration, _maxControlDuration);


        // create a planner for the defined space
        ob::PlannerPtr planner(new oc::RRT(si));
        ss->setPlanner(planner);

        _GoalBias=(planner->as<oc::RRT>())->getGoalBias();
        addParameter("Goal Bias", _GoalBias);
        ss->getPlanner()->as<oc::RRT>()->setGoalBias(_GoalBias);


        addParameter("PropagationStepSize", _propagationStepSize);
    }

	//! void destructor
    omplcRRTPlanner::~omplcRRTPlanner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool omplcRRTPlanner::setParameters(){

      omplcPlanner::setParameters();
      try{
        HASH_S_K::iterator it = _parameters.find("Goal Bias");
        if(it != _parameters.end()){
            _GoalBias = it->second;
            ss->getPlanner()->as<oc::RRT>()->setGoalBias(_GoalBias);
        }
        else
          return false;

        it = _parameters.find("Goal Bias");
                if(it != _parameters.end()){
                    _GoalBias = it->second;
                    ss->getPlanner()->as<oc::RRT>()->setGoalBias(_GoalBias);
                }
                else
                  return false;

        it = _parameters.find("Step Size");
            if(it != _parameters.end())
                     _stepSize = it->second;
                else
                  return false;

        it = _parameters.find("MinControlDuration");
            if(it != _parameters.end()){
                _minControlDuration = it->second;
                ss->getSpaceInformation()->setMinMaxControlDuration(_minControlDuration, _maxControlDuration);
            }
            else
                return false;

        it = _parameters.find("MaxControlDuration");
            if(it != _parameters.end()){
                 _maxControlDuration = it->second;
                 ss->getSpaceInformation()->setMinMaxControlDuration(_minControlDuration, _maxControlDuration);
            }
            else
                return false;

        it = _parameters.find("PropagationStepSize");
                if(it != _parameters.end()){
                     _propagationStepSize = it->second;
                     ss->getSpaceInformation()->setPropagationStepSize(_propagationStepSize);
                }
                else
                    return false;

        it = _parameters.find("ControlBound_Tras");
                if(it != _parameters.end()){
                             _controlBound_Tras = it->second;
                             ob::RealVectorBounds cbounds(2);
                             cbounds.setLow(0, - _controlBound_Tras * (1 - _onlyForward));
                             cbounds.setHigh(0, _controlBound_Tras);
                             cbounds.setLow(1, -_controlBound_Rot);
                             cbounds.setHigh(1, _controlBound_Rot);
                             spacec->as<oc::RealVectorControlSpace>()->setBounds(cbounds);
                }
                else
                   return false;

        it = _parameters.find("ControlBound_Rot");
               if(it != _parameters.end()){
                      _controlBound_Rot = it->second;
                      ob::RealVectorBounds cbounds(2);
                      cbounds.setLow(0, - _controlBound_Tras * (1 - _onlyForward));
                      cbounds.setHigh(0, _controlBound_Tras);
                      cbounds.setLow(1, -_controlBound_Rot);
                      cbounds.setHigh(1, _controlBound_Rot);
                      spacec->as<oc::RealVectorControlSpace>()->setBounds(cbounds);
                }
                else
                   return false;

        it = _parameters.find("OnlyForward (0/1)");
                if(it != _parameters.end()){
                    if(it->second != 0) _onlyForward=1;
                    else _onlyForward=0;

                    ob::RealVectorBounds cbounds(2);
                    cbounds.setLow(0, - _controlBound_Tras * (1 - _onlyForward));
                    cbounds.setHigh(0, _controlBound_Tras);
                    cbounds.setLow(1, -_controlBound_Rot);
                    cbounds.setHigh(1, _controlBound_Rot);
                    spacec->as<oc::RealVectorControlSpace>()->setBounds(cbounds);
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
