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
*     Copyright (C) 2007 - 2012 by Alexander Pérez and Jan Rosell          *
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
 
 
 
#include "workspace.h"
#include <vector>
#include <typeinfo>

using namespace std;

namespace libProblem {
  WorkSpace::WorkSpace(){
	  obstacles.clear();
	  robots.clear();
    distVec.clear();
    workDim = 0;
    _configMap.clear();
    _robWeight.clear();
  }


  unsigned int    libProblem::WorkSpace::_countWorldCollCheck = 0;
  void            libProblem::WorkSpace::resetCollCheckCounter(){_countWorldCollCheck = 0;}
  unsigned int    libProblem::WorkSpace::getCollCheckCounter(){ return _countWorldCollCheck;}
  void            libProblem::WorkSpace::increaseCollCheckCounter(){_countWorldCollCheck++;}

	void WorkSpace::addDistanceMapFile(string distanceFile)
	{
		distanceMapFile = distanceFile;
	}
	/*void WorkSpace::addNeighborhoodMapFile(string neighFile)
	{
		neighborhoodMapFile = neighFile;
	}*/


  vector<KthReal>* WorkSpace::distanceCheck(Sample* sample) {
    vector<KthReal> tmpVec;
    int j, from = 0;
    distVec.clear();
    for(unsigned int i=0; i< robots.size(); i++){
      tmpVec.clear();
      for( j=0; j < robots[i]->getNumControls(); j++ )
        tmpVec.push_back(sample->getCoords()[from + j]);
      
      from = j;
      robots[i]->control2Pose(tmpVec);
      for(unsigned int m = 0; m < obstacles.size(); m++){
        distVec.push_back(robots[i]->distanceCheck(obstacles[m]));
      }
    }
    return &distVec;
  }

  void WorkSpace::moveRobotsTo(Sample* sample){
    vector<KthReal> tmpVec;
    int j, from = 0;
    for(unsigned int i=0; i< robots.size(); i++){
		  if(sample->getMappedConf().size()==0){
        tmpVec.clear();
        for( j=0; j < robots[i]->getNumControls(); j++ )
           tmpVec.push_back(sample->getCoords()[from + j]);

        from = j;
        robots[i]->control2Pose(tmpVec);
	    }else{
			  robots[i]->Kinematics(sample->getMappedConf().at(i));
		  }
    }
  }

  void WorkSpace::moveObstacleTo( size_t mobObst, vector<KthReal>& pmd ){
    // The parameter pmd is the same type of data the user will be send to
    // move a robot. It is the value of parameter of a normal sample.
    if( mobObst < _mobileObstacle.size() ){
      _mobileObstacle[mobObst]->control2Pose( pmd );
    }else
      cout << "The mobObst index is greater than the counter of mobile obstacles.\n"; 

  }

  void WorkSpace::moveObstacleTo( size_t mobObst, RobConf& robConf ){
    // The parameter pmd is the same type of data the user will be send to
    // move a robot. It is the value of parameter of a normal sample.
    if( mobObst < _mobileObstacle.size() ){
      _mobileObstacle[mobObst]->Kinematics( robConf );
    }else
      cout << "The mobObst index is greater than the counter of mobile obstacles.\n"; 

  }

  bool WorkSpace::collisionCheck(Sample* sample ) {

	  increaseCollCheckCounter();

    vector<KthReal> tmpVec;
    bool collision = false;
    int j, from = 0;
    if(sample->getMappedConf().size() == 0){
      for(unsigned int i=0; i< robots.size(); i++){
        tmpVec.clear();
        for( j=0; j < robots[i]->getNumControls(); j++ )
          tmpVec.push_back(sample->getCoords()[from + j]);

        from = j;
        robots[i]->control2Pose(tmpVec);

        //first is testing if the robots collide with the environment (obstacles)
        for( unsigned int m = 0; m < obstacles.size(); m++){
          if( robots[i]->collisionCheck(obstacles[m]) ){
            collision = true;
            break;
          }
          if(collision) break;
        }
        // second test if a robot collides with another one present in the workspace.
        if( i > 0 ){
          for( int k = i-1; k = 0; --k){
            if( robots[i]->collisionCheck( robots[k] ) ){
              collision = true;
              break;
            }
          }
          if(collision) break;
        }
      }
    }else{
      for(unsigned int i=0; i< robots.size(); i++){
        robots[i]->Kinematics(sample->getMappedConf().at(i));
        //first is testing if the robot collides with the environment (obstacles)
        for( unsigned int m = 0; m < obstacles.size(); m++){
          if( robots[i]->collisionCheck(obstacles[m]) ){
            collision = true;
            break;
          }
          if(collision) break;
        }
        // second test if the robot collides with another one present in the workspace.
        // This validation is done with the robots validated previously.
        if( i > 0 ){
          for( int k = i-1; k = 0; --k){
            if( robots[i]->collisionCheck( robots[k] ) ){
              collision = true;
              break;
            }
          }
          if(collision) break;
        }
      }
    }
    // Here will be putted the configuration mapping 
    sample->setMappedConf(_configMap);
    
    if(collision) sample->setcolor(-1);
      else sample->setcolor(1);

    return collision;
  }

  //! This method returns the distances between two samples smp1 and
  //! smp2 passed as arguments. If the SPACETYPE is CONFIGSPACE, first
  //! the samples are inspected looking for the RobConf associated.
  //! If the sample do not has one, the workspace is asked for the 
  //! respective Mapping and then the distance is calculated.
  //! Be careful with samples non-free or without collision checking
  //! because they do not have mapping.
  //! If the SPACETYPE is SAMPLEDSPACE the distance is calculated with
  //! the coordinates directly.
  KthReal WorkSpace::distanceBetweenSamples(Sample& smp1, Sample& smp2,
                                            Kautham::SPACETYPE spc){
    switch(spc){
    case SAMPLEDSPACE:
      return smp1.getDistance(&smp2, spc);

    case CONFIGSPACE:
      if( smp1.getMappedConf().size() == 0){
        this->moveRobotsTo(&smp1);
        smp1.setMappedConf(getConfigMapping());
      }
      if( smp2.getMappedConf().size() == 0){
        this->moveRobotsTo(&smp2);
        smp2.setMappedConf(getConfigMapping());
      }
      return smp1.getDistance(&smp2, _robWeight, spc);

    default:
      return (KthReal)-1.0;
    }
  }


  //! returns the pair obstacle-robotlink that are colliding
  /*
	void WorkSpace::getCollisionInfo()
	{

	}
*/
 	
  KthReal WorkSpace::distanceCheck(Conf* conf, unsigned int robot) {
    KthReal resp = (KthReal)1e10;
    KthReal temp = (KthReal)0.0;
    robots[robot]->Kinematics(conf);
    for(unsigned int i = 0; i < obstacles.size(); i++){
	    temp = robots[robot]->distanceCheck(obstacles[i]);
	    if(resp > temp)resp = temp;
    }
    if(!resp){      // Now I test the robots collision
      if(robots.size() > 1 )
        for( size_t i=0; i < robots.size(); i++){
          if( i == robot ) continue;
          if( robots[robot]->distanceCheck( robots[i] ) ){
            resp = true;
            break;
          }
        }
    }
    return resp;
  }

  bool WorkSpace::collisionCheck(Conf* conf, unsigned int robot ) {
    bool resp = false;
    robots[robot]->Kinematics(conf);
    for(unsigned int i = 0; i < obstacles.size(); i++){
	    resp = robots[robot]->collisionCheck(obstacles[i]);
	    if(resp)break;
    }
    if(!resp){      // Now I test the robots collision
      if(robots.size() > 1 )
        for( size_t i=0; i < robots.size(); i++){
          if( i == robot ) continue;
          if( robots[robot]->collisionCheck( robots[i] ) ){
            resp = true;
            break;
          }
        }
    }
    return resp;
  }

  void WorkSpace::addRobot(Robot* robot){
    robots.push_back(robot);
    workDim = 0;
    _configMap.clear();
    _robWeight.clear();
    for(unsigned int i = 0; i < robots.size(); i++){
      workDim += robots[i]->getNumControls();
      _configMap.push_back(((Robot*)robots.at(i))->getCurrentPos());
      _robWeight.push_back( ((Robot*)robots.at(i))->getRobWeight() );
    }
  }

  void WorkSpace::addMobileObstacle(Robot* obs){
    _mobileObstacle.push_back( obs );
  }
      
  void WorkSpace::addObstacle(Obstacle* obs){
    obstacles.push_back(obs);
  }

  bool WorkSpace::inheritSolution(vector<Sample*>& path){
    vector< vector<RobConf*> > tmpRobPath;
    
    for(unsigned int i = 0; i < robots.size(); i++)
        tmpRobPath.push_back(*(new vector<RobConf*>)) ;

    vector<Sample*>::iterator it;
    for(it = path.begin(); it != path.end(); ++it){
      if((*it)->getMappedConf().size() == 0 )
        collisionCheck((*it));

      vector<RobConf>& tmpMapp = (*it)->getMappedConf();

      for(unsigned int i = 0; i < robots.size(); i++)
        tmpRobPath[i].push_back(&(tmpMapp.at(i)) );
      
    }

    for(unsigned int i = 0; i < robots.size(); i++)
      robots.at(i)->setProposedSolution(tmpRobPath[i]);

    for(unsigned int i = 0; i < robots.size(); i++)
      tmpRobPath.at(i).clear();

    tmpRobPath.clear();
    return true;
  }

  void WorkSpace::eraseSolution(){
    vector<Robot*>::iterator it;
    for(it = robots.begin(); it != robots.end(); ++it){
      (*it)->cleanProposedSolution();
    }
  }

  void WorkSpace::setPathVisibility(bool vis){
    for(size_t i = 0; i < robots.size(); i++ )
      robots.at(i)->setPathVisibility( vis );
  }

  bool WorkSpace::attachObstacle2RobotLink(string robot, string link, unsigned int obs ){
    return false;
  }

  bool WorkSpace::detachObstacleFromRobotLink(string robot, string link ){
    return false;
  }

}


 
