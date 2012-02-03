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
 
 
 
 
#if !defined(_WORKSPACE_H)
#define _WORKSPACE_H

#include <vector>
#include <libsampling/robconf.h>
#include <libutil/kauthamdefs.h>
#include <libsampling/sample.h>
#include "obstacle.h"
#include "robot.h"

using namespace std;
using namespace Kautham;
using namespace libSampling;

namespace libProblem{
  class WorkSpace {
	  public:
      WorkSpace();
		  KthReal               distanceCheck( Conf* conf, unsigned int robot ) ;
		  bool                  collisionCheck( Conf* conf, unsigned int robot ) ;
      KthReal               distanceBetweenSamples(Sample& smp1, Sample& smp2,
                                                Kautham::SPACETYPE spc);

      vector<KthReal>*      distanceCheck(Sample* sample) ;
      bool                  collisionCheck(Sample* sample ) ;
      void                  moveRobotsTo(Sample* sample);
      void                  moveObstacleTo( size_t mobObst, vector<KthReal>& pose );
      void                  moveObstacleTo( size_t mobObst, RobConf& robConf );
		  void                  addRobot(Robot* robot);
      inline Robot*         getRobot(unsigned int i){if( i < robots.size() ) return robots[i]; return NULL;}
		  void                  addObstacle(Obstacle* obs);
		  void                  addMobileObstacle(Robot* obs);
      inline Obstacle*      getObstacle(unsigned int i){if(i < obstacles.size()) return obstacles[i]; return NULL;}
      inline Robot*         getMobileObstacle(unsigned int i){if( i<_mobileObstacle.size() ) return _mobileObstacle[i];return NULL;}
      inline unsigned int   robotsCount(){return robots.size();}
      inline unsigned int   obstaclesCount(){return obstacles.size();}
      inline unsigned int   mobileObstaclesCount(){return _mobileObstacle.size();}
      inline int            getDimension() const {return workDim;}

	    void addDistanceMapFile(string distanceFile);
	    inline string getDistanceMapFile(){return distanceMapFile;};
	  //void addNeighborhoodMapFile(string neighFile);
	  //inline string getNeighborhoodMapFile(){return neighborhoodMapFile;};

//      //! This method returns true if the all robots in the scene only accepts SE3 data;
//      //! This method is deprecated. Maybe it never has been used.
//      bool                  isSE3();

      //! This vector contains a pointers to the RobConf of each robot in the
      //! WorkSpace
      inline vector<RobConf*>&     getConfigMapping(){return _configMap;}
      inline vector<RobConf*>&     getConfigMapping(Sample* sample){moveRobotsTo(sample); return _configMap;}
      bool                         inheritSolution(vector<Sample*>& path);
      void                         eraseSolution();
      void                         setPathVisibility(bool vis);

      //! This method attaches an object object to a robot link. The obstacle is designated by its index.
      bool                  attachObstacle2RobotLink(string robot, string link, unsigned int obs );

      //! This method detaches an object previously attached to a Robot link.
      bool                  detachObstacleFromRobotLink(string robot, string link );
	  protected:
		  virtual void          updateScene() = 0;
		  vector<Obstacle*>     obstacles;
		  vector<Robot*>        _mobileObstacle;
		  vector<Robot*>        robots;
      vector<KthReal>       distVec;
      int                   workDim;

      //! This attribute groups the configurations of the robots
      vector<RobConf*>      _configMap;
      vector<RobWeight*>    _robWeight;

	  string distanceMapFile;
	  //string neighborhoodMapFile;

  };
}

#endif  //_WORKSPACE_H

