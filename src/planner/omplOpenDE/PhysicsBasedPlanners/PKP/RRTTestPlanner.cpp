//*************************************************************************\
//   Copyright 2014 Institute of Industrial and Control Engineering (IOC)
//                 Universitat Politecnica de Catalunya
//                 BarcelonaTech
//    All Rights Reserved.

//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the
//    Free Software Foundation, Inc.,
//    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// \*************************************************************************/

///* Author: Muhayyuddin */


//#if defined(KAUTHAM_USE_OMPL)
//#if defined(KAUTHAM_USE_ODE)

//#include "KauthamOpenDERRTTestPlanner.h"
//namespace Kautham {

//namespace omplcplanner{

//// The planner is based on the example of OpenDERigidBodyPlanning.
//// Create the Environment, the State Space, and simple setup. It is said that the Goal class KAuthamDEGoal and planning through simple setup after haverli declared planner and Bounds.
///*! Constructor create the dynamic environment, and setup all the parameters for planning.
// * it defines simple setup, Planner and Planning parameters.
// */
//KauthamDERRT2RobotPlanner::KauthamDERRT2RobotPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, WorkSpace *ws):
//    KauthamDEPlanner(stype, init, goal, samples, ws)
//{
//    //_wkSpace->moveRobotsTo(init);

//    //set intial values from parent class data
//    _guiName = "ompl ODE RRT 3Robot Planner";
//    _idName = "omplODERRT2RobotPlanner";
//    dInitODE2(0);
//    oc::OpenDEEnvironmentPtr envPtr(new KauthamDE2RobotEnvironment (ws,_maxspeed,_maxContacts,_minControlSteps,_maxControlSteps, _erp, _cfm));
//    stateSpace = new KauthamDE2RobotStateSpace(envPtr);
//    stateSpacePtr = ob::StateSpacePtr(stateSpace);

//    ss = new oc::OpenDESimpleSetup(stateSpacePtr);
//    oc::SpaceInformationPtr si=ss->getSpaceInformation();
//    ob::PlannerPtr planner(new oc::RRT(si));

//    //set RRT Ggoal Bias
//    _GoalBias=(planner->as<oc::RRT>())->getGoalBias();
//    addParameter("Goal Bias", _GoalBias);
//    planner->as<oc::RRT>()->setGoalBias(_GoalBias);

//    //set the planner
//    ss->setPlanner(planner);

//}
////! void destructor
//KauthamDERRT2RobotPlanner::~KauthamDERRT2RobotPlanner(){

//}
////! this function set the necessary parameters for RRT Planner.
//bool KauthamDERRT2RobotPlanner::setParameters()
//{
//    try{
//        HASH_S_K::iterator it = _parameters.find("Goal Bias");
//        if(it != _parameters.end()){
//            _GoalBias = it->second;
//            ss->getPlanner()->as<oc::RRT>()->setGoalBias(_GoalBias);
//        }
//        else
//            return false;

//    }catch(...){
//        return false;
//    }
//    return true;

//}

//}

//}

//#endif //KAUTHAM_USE_ODE
//#endif // KAUTHAM_USE_OMPL


