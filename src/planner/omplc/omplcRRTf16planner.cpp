/*************************************************************************\
   Copyright 2014 Institute of Industrial and Control Engineering (IOC)
                 Universitat Politecnica de Catalunya
                 BarcelonaTech
    All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 \*************************************************************************/

/* Author: Alexander Perez, Jan Rosell */

#if defined(KAUTHAM_USE_OMPL)
#include <problem/workspace.h>
#include <sampling/sampling.h>

#include <boost/bind/mem_fn.hpp>
#include <ompl/base/spaces/SE2StateSpace.h>

#include "omplcRRTf16planner.h"



namespace Kautham {
  namespace omplcplanner{


  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Class KinematicF16Model
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //! This class derives from KinematicRobotModel and reimplements the operator() function that
  //! defines the qdot = f(q, u) equations of a f16 plane
  class KinematicF16Model : public KinematicRobotModel
  {
  public:

      /// Constructor
      KinematicF16Model(const ob::StateSpacePtr space) : KinematicRobotModel(space)
      {
          param_.resize(1);
      }

      /// implement the function describing the robot motion: qdot = f(q, u)
      void operator()(const ob::State *state, const oc::Control *control, std::valarray<double> &dstate) const
      {
          //Call the parent operator to resize the dstate
          KinematicRobotModel::operator()(state,  control, dstate);

          //Get the control values
          const double *u = control->as<oc::RealVectorControlSpace::ControlType>()->values;

          //Get the subspace corresponding to robot 0
          ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space_->as<ob::CompoundStateSpace>()->getSubspace(0));

          //Get the SE3 subspace of robot 0
          ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) ssRobot0->as<ob::CompoundStateSpace>()->getSubspace(0));

          //Retrieve the SE3 configuration
          ob::ScopedState<ob::SE3StateSpace> pathscopedstatese3(ssRobot0SE3);
          ob::ScopedState<ob::CompoundStateSpace> sstate(space_);
          sstate = *state;
          sstate >> pathscopedstatese3;

          //Get the current orientation (theta)
          mt::Rotation ori(pathscopedstatese3->rotation().x,
                           pathscopedstatese3->rotation().y,
                           pathscopedstatese3->rotation().z,
                           pathscopedstatese3->rotation().w);

          //translation
          mt::Vector3 advance(u[0], 0.0, 0.0);//advance in the x-direction of the plane
          mt::Vector3 v = ori(advance);
          dstate[0] = v[0];
          dstate[1] = v[1];
          dstate[2] = v[2];

          //computes dstate, the se3 incremental motion

          //rotation: axis-angle
          mt::Unit3 axisx(1.0, 0.0, 0.0);
          double ax = u[0] * tan(u[1]) / l;
          mt::Rotation rx(axisx,ax);

          mt::Unit3 axisy(0.0, 1.0, 0.0);
          double ay = u[0] * tan(u[2]) / l;
          mt::Rotation ry(axisy,ay);

          mt::Unit3 axisz(0.0, 0.0, 1.0);
          double az = u[0] * tan(u[3]) / l;
          mt::Rotation rz(axisz,az);



          mt::Rotation rot = rx*ry*rz;
          mt::Unit3 axis;
          float theta;
          rot.getAxisAngle(axis, theta);

          //compostion
          dstate[3] = axis[0];
          dstate[4] = axis[1];
          dstate[5] = axis[2];

          if(theta>M_PI)
          {
              theta -= 2*M_PI;
          }
          dstate[6] = theta;


      }

      /// Sets the plane length
      void setLength(double d)
      {
          setParameter(0, d);
          l = d;
      }

      /// Gets the plane length
      double getLength()
      {
          return getParameter(0);
      }
   private:
      double l;
  };


  /////////////////////////////////////////////////////////////////////////////////////////////////
  // omplcRRTf16Planner functions
  /////////////////////////////////////////////////////////////////////////////////////////////////
	//! Constructor
    omplcRRTf16Planner::omplcRRTf16Planner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, WorkSpace *ws):
              omplcPlanner(stype, init, goal, samples, ws)
	{
        _guiName = "ompl cRRT Planner";
        _idName = "omplcRRTf16";

        // create a control space
        int numcontrols = 4;
        spacec = ((oc::ControlSpacePtr) new oc::RealVectorControlSpace(space, numcontrols));

        // set the bounds for the control space
        _controlBound_Tras = 30;
        _controlBound_Rot = 0.3;
        addParameter("ControlBound_Tras", _controlBound_Tras);
        addParameter("ControlBound_Rot", _controlBound_Rot);
        ob::RealVectorBounds cbounds(numcontrols);
        cbounds.setLow(0, 0.0);
        cbounds.setHigh(0, _controlBound_Tras);
        cbounds.setLow(1, -_controlBound_Rot);
        cbounds.setHigh(1, _controlBound_Rot);
        cbounds.setLow(2, -_controlBound_Rot);
        cbounds.setHigh(2, _controlBound_Rot);
        cbounds.setLow(3, -_controlBound_Rot);
        cbounds.setHigh(3, _controlBound_Rot);
        spacec->as<oc::RealVectorControlSpace>()->setBounds(cbounds);

        // define a simple setup class
        ss = ((oc::SimpleSetupPtr) new oc::SimpleSetup(spacec));

        // set state validity checking for this space
        ss->setStateValidityChecker(boost::bind(&omplcplanner::isStateValid, ss->getSpaceInformation().get(), _1, (Planner*)this));

        // set the propagation routine for this space
        oc::SpaceInformationPtr si=ss->getSpaceInformation();
        ss->setStatePropagator(oc::StatePropagatorPtr(new omplcPlannerStatePropagator<KinematicF16Model>(si)));

        // propagation step size
        _propagationStepSize = 0.01;
        _duration = 1;
        addParameter("PropagationStepSize", _propagationStepSize);
        addParameter("Duration", _duration);
        static_cast<omplcPlannerStatePropagator<KinematicF16Model>*>(ss->getStatePropagator().get())->setIntegrationTimeStep(_propagationStepSize);
        si->setPropagationStepSize(_propagationStepSize*_duration);

        //Length
        _length = 0.2;
        addParameter("Length", _length);
        static_cast<omplcPlannerStatePropagator<KinematicF16Model>*>(ss->getStatePropagator().get())->getIntegrator()->getOde()->setLength(_length);

        // create a planner for the defined space
        ob::PlannerPtr planner(new oc::RRT(si));

        //set RRT Ggoal Bias
        _GoalBias=(planner->as<oc::RRT>())->getGoalBias();
        addParameter("Goal Bias", _GoalBias);
        planner->as<oc::RRT>()->setGoalBias(_GoalBias);

        //permit intermediate states
        planner->as<oc::RRT>()->setIntermediateStates(true);

        //set the planner
        ss->setPlanner(planner);


    }

	//! void destructor
    omplcRRTf16Planner::~omplcRRTf16Planner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool omplcRRTf16Planner::setParameters(){

      omplcPlanner::setParameters();
      try{
        HASH_S_K::iterator it = _parameters.find("Goal Bias");
        if(it != _parameters.end()){
            _GoalBias = it->second;
            ss->getPlanner()->as<oc::RRT>()->setGoalBias(_GoalBias);
        }
        else
          return false;

     it = _parameters.find("Length");
     if(it != _parameters.end()){
          _length = it->second;
          static_cast<omplcPlannerStatePropagator<KinematicF16Model>*>(ss->getStatePropagator().get())->getIntegrator()->getOde()->setLength(_length);
      }
      else
         return false;

     it = _parameters.find("Duration");
     if(it != _parameters.end()){
         _duration = it->second;
         ss->getSpaceInformation()->setPropagationStepSize(_propagationStepSize*_duration);
      }
      else
         return false;

       it = _parameters.find("PropagationStepSize");
       if(it != _parameters.end()){
           _propagationStepSize = it->second;
           static_cast<omplcPlannerStatePropagator<KinematicF16Model>*>(ss->getStatePropagator().get())->setIntegrationTimeStep(_propagationStepSize);
           ss->getSpaceInformation()->setPropagationStepSize(_propagationStepSize*_duration);
        }
        else
           return false;

        it = _parameters.find("ControlBound_Tras");
        if(it != _parameters.end()){
            _controlBound_Tras = it->second;
            ob::RealVectorBounds cbounds(4);
            cbounds.setLow(0, 0.0);
            cbounds.setHigh(0, _controlBound_Tras);
            cbounds.setLow(1, -_controlBound_Rot);
            cbounds.setHigh(1, _controlBound_Rot);
            cbounds.setLow(2, -_controlBound_Rot);
            cbounds.setHigh(2, _controlBound_Rot);
            cbounds.setLow(3, -_controlBound_Rot);
            cbounds.setHigh(3, _controlBound_Rot);
            spacec->as<oc::RealVectorControlSpace>()->setBounds(cbounds);
         }
         else
            return false;

        it = _parameters.find("ControlBound_Rot");
        if(it != _parameters.end()){
              _controlBound_Rot = it->second;
              ob::RealVectorBounds cbounds(4);
              cbounds.setLow(0, 0.0);
              cbounds.setHigh(0, _controlBound_Tras);
              cbounds.setLow(1, -_controlBound_Rot);
              cbounds.setHigh(1, _controlBound_Rot);
              cbounds.setLow(2, -_controlBound_Rot);
              cbounds.setHigh(2, _controlBound_Rot);
              cbounds.setLow(3, -_controlBound_Rot);
              cbounds.setHigh(3, _controlBound_Rot);
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
