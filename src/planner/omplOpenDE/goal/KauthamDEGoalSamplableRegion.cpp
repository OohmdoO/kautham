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

/* Author: Muhayyuddin  */


#if defined(KAUTHAM_USE_OMPL)
#if defined(KAUTHAM_USE_ODE)
#include "KauthamDEGoalSamplableRegion.h"
class KauthamODEobject;

using namespace std;


namespace Kautham {

namespace omplcplanner{

vector<KauthamODEobject> KauthamDEGoalSamplableRegion::smp2KauthamOpenDEState(WorkSpace *wkSpace,Sample *goal)
       {


           vector<KauthamODEobject> kauthamob;

           for(unsigned int i=0; i<=(int(wkSpace->getNumRobots())-1); i++)
           {
               //wkSpace->getRobot(0)->Kinematics(goal->getMappedConf()[0]);
                wkSpace->getRobot(i)->Kinematics(goal->getMappedConf().at(0).getSE3());
               for(unsigned int j=0; j<= (wkSpace->getRobot(i)->getNumLinks())-1;j++)
               {

                   KauthamODEobject odeob;
                   std::cout<< "Link  "<< wkSpace->getRobot(i)->getLink(j)->getName()<<" added "<<std::endl;
                   odeob.objectposition[0] = wkSpace->getRobot(i)->getLink(j)->getElement()->getPosition()[0];
                   odeob.objectposition[1] = wkSpace->getRobot(i)->getLink(j)->getElement()->getPosition()[1];
                   odeob.objectposition[2] = wkSpace->getRobot(i)->getLink(j)->getElement()->getPosition()[2];
                   //Kauthamodebodies.insert(pair<int,KauthamODEobject>(i,*(wkSpace->getRobot(i)->getLink(j)->getElement()->getPosition()));
                   odeob.objectorientation[0] = wkSpace->getRobot(i)->getLink(j)->getElement()->getOrientation()[0];
                   odeob.objectorientation[1] = wkSpace->getRobot(i)->getLink(j)->getElement()->getOrientation()[1];
                   odeob.objectorientation[2] = wkSpace->getRobot(i)->getLink(j)->getElement()->getOrientation()[2];
                   odeob.objectorientation[3] = wkSpace->getRobot(i)->getLink(j)->getElement()->getOrientation()[3];
                   kauthamob.push_back(odeob);


               }

           }
           //std::cout<<"goal is "<<Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[0]<<"  "<<Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[1]<<"  "<<Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[2]<<std::endl;
           return kauthamob;
       }

       double KauthamDEGoalSamplableRegion::distanceGoal(const ob::State *st) const
       {
           KthReal distance;
           distance=0.0;

           if(!onlyend)
           {
               for(unsigned int i=0; i < Kauthamodebodies.size(); i++)
               {
                   const double *pos = st->as<oc::OpenDEStateSpace::StateType>()->getBodyPosition(i);
                   const double *vel = st->as<oc::OpenDEStateSpace::StateType>()->getBodyLinearVelocity(i);
                   double dx = Kauthamodebodies[i].objectposition[0] - pos[0];
                   double dy = Kauthamodebodies[i].objectposition[1] - pos[1];
                   //double dz = Kauthamodebodies[i].objectposition[2] - pos[2];
                   double dot = dx * vel[0] + dy * vel[1];// + dz *vel[2];
                   if(dot > 0)
                       dot=0;
                   else
                       dot=sqrt(fabs(dot));

                   distance = distance+ sqrt(dx*dx + dy*dy )+dot;//+ dz*dz) + dot;
               }

           }
           else
           {
       //        const double *pos = st->as<oc::OpenDEStateSpace::StateType>()->getBodyPosition(Kauthamodebodies.size()-1);
       //        distance = sqrt(fabs(pos[0]-Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[0]))+(fabs(pos[1]-Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[1]))+(fabs(pos[2]-Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[2]));
               const double *pos = st->as<oc::OpenDEStateSpace::StateType>()->getBodyPosition(7);
               const double *vel = st->as<oc::OpenDEStateSpace::StateType>()->getBodyLinearVelocity(7);
               double dx = Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[0] - pos[0];
               double dy = Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[1] - pos[1];
               double dz = Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[2] - pos[2];
               double dot = dx * vel[0] + dy * vel[1] + dz *vel[2];
               if(dot > 0)
                   dot=0;
               else
                   dot=sqrt(fabs(dot));

               distance = distance+ sqrt(dx*dx + dy*dy + dz*dz) + dot;
//               std::cout<<" Goal is :       [ "<<Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[0]<<" , "<<Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[1]<<" , " <<Kauthamodebodies[Kauthamodebodies.size()-1].objectposition[2]<<" ]"<<std::endl;
//               std::cout<<" Current P is :  [ "<<pos[0]<<" , "<<pos[1]<<" , " <<pos[2]<<" ]"<<std::endl;
//               std::cout<<" Difference is : [ "<<dx<<" , "<<dy<<" , " <<dz<<" ]"<<std::endl;

           }
           return distance;
       }

     void KauthamDEGoalSamplableRegion::sampleGoal(ob::State *st) const
       {
        double *v = st->as<oc::OpenDEStateSpace::StateType>()->getBodyPosition(0);
        stateSampler_->sampleUniform(st);
        v[0]=st->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(0)->values[0];
        v[1]=st->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(0)->values[1];
        //v[2]=st->as<ob::CompoundState>()->as<ob::RealVectorStateSpace::StateType>(0)->values[2];

    }

     bool KauthamDEGoalSamplableRegion::isSatisfied(const ob::State *st, double *distance) const
     {
         double d2g = distanceGoal(st);
        if(d2g<120)
         std::cout<<"Distance To Goal is : "<<d2g<<std::endl;
         if (distance)
             *distance = d2g;
         return d2g < threshold_;
     }

     bool KauthamDEGoalSamplableRegion::isSatisfied(const ob::State *st) const
     {
         return isSatisfied(st, NULL);
     }
KauthamDEGoalSamplableRegion::~KauthamDEGoalSamplableRegion()
{
}


}

}
#endif//KAUTHAM_USE_ODE
#endif// KAUTHAM_USE_OMPL
