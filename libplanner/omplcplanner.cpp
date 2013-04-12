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

#include "localplanner.h"
#include "omplcplanner.h"


#include <ompl/base/spaces/SE2StateSpace.h>
#include <ompl/base/PlannerStatus.h>


#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoLineSet.h>


using namespace libSampling;

namespace libPlanner {
  namespace omplcplanner{

  bool isStateValid(const oc::SpaceInformation *si, const ob::State *state, Planner *p)//, Sample *smp)
  //bool isStateValid(const ob::State *state, Planner *p)//, Sample *smp)
  {

      //const ob::SE2StateSpace::StateType *se2state = state->as<ob::SE2StateSpace::StateType>();
      //const ob::RealVectorStateSpace::StateType *pos = se2state->as<ob::RealVectorStateSpace::StateType>(0);
      //const ob::SO2StateSpace::StateType *rot = se2state->as<ob::SO2StateSpace::StateType>(1);

      const ob::RealVectorStateSpace::StateType *R2state = state->as<ob::RealVectorStateSpace::StateType>();
      int d = p->wkSpace()->getDimension();
      Sample *smp = new Sample(d);
      vector<KthReal> coords;
      coords.resize(d);

      for(int i=0;i<d;i++)
        coords[i] = R2state->values[i];

      //coords[0] = pos->values[0];
      //coords[1] = pos->values[1];
      //coords[2] = rot->value;
      smp->setCoords(coords);
      if( si->satisfiesBounds(state)==false | p->wkSpace()->collisionCheck(smp) )
          return false;
      return true;
  }



	//! Constructor
    omplcPlanner::omplcPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              Planner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
		//set intial values
        _planningTime = 10;

		//set intial values from parent class data
		_speedFactor = 1;
        _solved = false;
        _stepSize = ssize;
	  
        _guiName = "ompl Planner";
        _idName = "ompl Planner";
        addParameter("Step Size", ssize);
        addParameter("Speed Factor", _speedFactor);
        addParameter("Max Planning Time", _planningTime);


        // construct the state space we are planning in
        int d = _wkSpace->getDimension();
        space =  ((ob::StateSpacePtr) new ob::RealVectorStateSpace(d));


        // set the bounds
        ob::RealVectorBounds bounds(d);
        bounds.setLow(0);
        bounds.setHigh(1);
        space->as<ob::RealVectorStateSpace>()->setBounds(bounds);


        // define a simple setup class
        //ss = ((og::SimpleSetupPtr) new og::SimpleSetup(space));
        //plannerdata = ((ob::PlannerDataPtr) new ob::PlannerData(ss->getSpaceInformation()));

        //Derived classes should specify a given planner
        //ob::SpaceInformationPtr si=ss->getSpaceInformation();
        //ob::PlannerPtr planner(new og::PRM(si));
        //ob::PlannerPtr planner(new og::RRT(si));
        //ob::PlannerPtr planner(new og::RRTConnect(si));
        //ss->setPlanner(planner);

        //ss->setStateValidityChecker(boost::bind(&isStateValid, _1, (Planner*)this));
    }

	//! void destructor
    omplcPlanner::~omplcPlanner(){
			
	}
	
	//! setParameters sets the parameters of the planner
    bool omplcPlanner::setParameters(){
      try{
        HASH_S_K::iterator it = _parameters.find("Speed Factor");
        if(it != _parameters.end())
          _speedFactor = it->second;
        else
          return false;

        it = _parameters.find("Max Planning Time");
        if(it != _parameters.end())
            _planningTime = it->second;
        else
          return false;

      }catch(...){
        return false;
      }
      return true;
    }

  	
    SoSeparator *omplcPlanner::getIvCspaceScene()
    {
        if(_wkSpace->getDimension()<=3)
        {
            //_sceneCspace = ((IVWorkSpace*)_wkSpace)->getIvScene();
            _sceneCspace = new SoSeparator();
        }
        else _sceneCspace=NULL;
        return Planner::getIvCspaceScene();
    }


    void omplcPlanner::drawCspace()
    {
        if(_sceneCspace==NULL) return;

        if(_wkSpace->getDimension()<=3)
        {
            //first delete whatever is already drawn
            while (_sceneCspace->getNumChildren() > 0)
            {
                _sceneCspace->removeChild(0);
            }


            //draw points
            SoSeparator *psep = new SoSeparator();
            SoCoordinate3 *points  = new SoCoordinate3();
            SoPointSet *pset  = new SoPointSet();


            //KthReal xmin=100000000.0;
            //KthReal xmax=-100000000.0;
            //KthReal ymin=100000000.0;
            //KthReal ymax=-100000000.0;
            KthReal xmin=0.0;
            KthReal xmax=1.0;
            KthReal ymin=0.0;
            KthReal ymax=1.0;

            KthReal x,y;

            ob::PlannerDataPtr pdata;
            pdata = ((ob::PlannerDataPtr) new ob::PlannerData(ss->getSpaceInformation()));
            ss->getPlanner()->getPlannerData(*pdata);

            for(int i=0;i<pdata->numVertices();i++)
            {
                x=pdata->getVertex(i).getState()->as<ob::RealVectorStateSpace::StateType>()->values[0];
                y=pdata->getVertex(i).getState()->as<ob::RealVectorStateSpace::StateType>()->values[1];

                points->point.set1Value(i,x,y,0);

                //if(x<xmin) xmin=x;
                //if(x>xmax) xmax=x;
                //if(y<ymin) ymin=y;
                //if(y>ymax) ymax=y;
            }

            SoDrawStyle *pstyle = new SoDrawStyle;
            pstyle->pointSize = 2;
            SoMaterial *color = new SoMaterial;
            color->diffuseColor.setValue(0.2,0.8,0.2);

            //draw samples
            psep->addChild(color);
            psep->addChild(points);
            psep->addChild(pstyle);
            psep->addChild(pset);

            _sceneCspace->addChild(psep);


            //draw edges:
            SoSeparator *lsep = new SoSeparator();

            int numOutgoingEdges;

            //std::map< unsigned int, const ob::PlannerDataEdge * > edgeMap;
            std::vector< unsigned int > outgoingVertices;

            for(int i=0;i<pdata->numVertices();i++)
            {
                //numOutgoingEdges = plannerdata->getEdges (i, edgeMap);
                //std::map< unsigned int, const ob::PlannerDataEdge * >::iterator it;
                //for ( it=edgeMap.begin(); it != edgeMap.end(); it++ ){

                numOutgoingEdges = pdata->getEdges (i, outgoingVertices);
                for ( int j=0; j<numOutgoingEdges; j++ ){

                  SoCoordinate3 *edgepoints  = new SoCoordinate3();

                  //edgepoints->point.set1Value((0,points->point[i]);
                  //edgepoints->point.set1Value(1,points->point[(*it).first]);
                  //edgepoints->point.set1Value(1,points->point[outgoingVertices.at(j)]);


                  float x1,y1,x2,y2,z;
                  x1=pdata->getVertex(i).getState()->as<ob::RealVectorStateSpace::StateType>()->values[0];
                  y1=pdata->getVertex(i).getState()->as<ob::RealVectorStateSpace::StateType>()->values[1];
                  z=0.0;
                  edgepoints->point.set1Value(0,x1,y1,z);

                  x2=pdata->getVertex(outgoingVertices.at(j)).getState()->as<ob::RealVectorStateSpace::StateType>()->values[0];
                  y2=pdata->getVertex(outgoingVertices.at(j)).getState()->as<ob::RealVectorStateSpace::StateType>()->values[1];
                  edgepoints->point.set1Value(1,x2,y2,z);


                  //cout<<"i:"<<i<<" j:"<<outgoingVertices.at(j)<<" weight:"<<plannerdata->getEdgeWeight(i,outgoingVertices.at(j))<<endl;

                  lsep->addChild(edgepoints);

                  SoLineSet *ls = new SoLineSet;
                  ls->numVertices.set1Value(0,2);//two values
                  //cout<<"EDGE "<<(*itC)->first<<" "<<(*itC)->second<<endl;
                  lsep->addChild(ls);
                }
            }
            _sceneCspace->addChild(lsep);

            //draw path:
            if(_solved)
            {
                SoSeparator *pathsep = new SoSeparator();
                std::vector< ob::State * > & pathstates = ss->getSolutionPath().getStates();

                for(int i=0; i<ss->getSolutionPath().getStateCount()-1; i++)
                {
                    SoCoordinate3 *edgepoints  = new SoCoordinate3();
                    x=pathstates[i]->as<ob::RealVectorStateSpace::StateType>()->values[0];
                    y=pathstates[i]->as<ob::RealVectorStateSpace::StateType>()->values[1];
                    edgepoints->point.set1Value(0,x,y,0);
                    x=pathstates[i+1]->as<ob::RealVectorStateSpace::StateType>()->values[0];
                    y=pathstates[i+1]->as<ob::RealVectorStateSpace::StateType>()->values[1];
                    edgepoints->point.set1Value(1,x,y,0);

                    pathsep->addChild(edgepoints);

                    SoLineSet *ls = new SoLineSet;
                    ls->numVertices.set1Value(0,2);//two values
                    SoDrawStyle *lstyle = new SoDrawStyle;
                    lstyle->lineWidth=3;
                    SoMaterial *path_color = new SoMaterial;
                    path_color->diffuseColor.setValue(0.8,0.2,0.2);
                    pathsep->addChild(path_color);
                    pathsep->addChild(lstyle);
                    pathsep->addChild(ls);
                }
                _sceneCspace->addChild(pathsep);
            }


            //draw floor
            SoSeparator *floorsep = new SoSeparator();
            SoCube *cs = new SoCube();
            cs->width = xmax-xmin;
            cs->depth = (xmax-xmin)/50.0;
            cs->height = ymax-ymin;

            SoTransform *cub_transf = new SoTransform;
            SbVec3f centre;
            centre.setValue(xmin+(xmax-xmin)/2,ymin+(ymax-ymin)/2,-cs->depth.getValue());
            cub_transf->translation.setValue(centre);
            cub_transf->recenter(centre);

            SoMaterial *cub_color = new SoMaterial;
            cub_color->diffuseColor.setValue(0.2,0.2,0.2);

            floorsep->addChild(cub_color);
            floorsep->addChild(cub_transf);
            floorsep->addChild(cs);
            _sceneCspace->addChild(floorsep);

            //cout<<"xmin:"<<xmin<<" xmax: "<<xmax<<endl;
            //cout<<"ymin:"<<ymin<<" ymax: "<<ymax<<endl;

            //plannerdata->printGraphviz(outGraphviz);
            //plannerdata->printGraphviz(cout);
        }
    }

	//! function to find a solution path
        bool omplcPlanner::trySolve()
		{

            /*
            ob::ScopedState<ob::SE2StateSpace> startompl(space);
            startompl->setX(_init->getCoords()[0]);
            startompl->setY(_init->getCoords()[1]);
            startompl->setYaw(0.0);
            ob::ScopedState<ob::SE2StateSpace> goalompl(space);
            goalompl->setX(_goal->getCoords()[0]);
            goalompl->setY(_goal->getCoords()[1]);
            goalompl->setYaw(0.0);
            */


           //Start
            int d = _wkSpace->getDimension();

            ob::ScopedState<> startompl(space);
            std::vector< double > coordsInit;
            coordsInit.resize(d);
            for(int i=0;i<d;i++)
                coordsInit[i] = _init->getCoords()[i];
            startompl =coordsInit;

            //Goal
            ob::ScopedState<> goalompl(space);
            std::vector< double > coordsGoal;
            coordsGoal.resize(d);
            for(int i=0;i<d;i++)
                coordsGoal[i] = _goal->getCoords()[i];
            goalompl = coordsGoal;



             // set the start and goal states
            ss->setStartAndGoalStates(startompl, goalompl, 0.01);//0.05);

            // attempt to solve the problem within _planningTime seconds of planning time
            ss->clear();//to remove previous solutions, if any
            ss->getPlanner()->clear();
            ob::PlannerStatus solved = ss->solve(_planningTime);
            //UNKNOWN = 0,
            //INVALID_START,
            //INVALID_GOAL,
            //UNRECOGNIZED_GOAL_TYPE,
            //TIMEOUT,
            //APPROXIMATE_SOLUTION,
            //EXACT_SOLUTION,
            //CRASH,
            //TYPE_COUNT

            ss->print();

            if (solved)
            {
                    std::vector< KthReal > coords;
                    coords.resize(d);
                    std::cout << "Found solution (solved=<<"<<solved.asString()<<"):" << std::endl;
                    // print the path to screen
                    std::cout << "Geomeric Path solution:" << std::endl;
                    ss->getSolutionPath().asGeometric().print(std::cout);
                    std::cout << "Control Path solution:" << std::endl;
                    ss->getSolutionPath().print(std::cout);


                    std::vector< ob::State * > & pathstates = ss->getSolutionPath().asGeometric().getStates();
                    Sample *smp=new Sample(d);

                    _path.clear();
                    clearSimulationPath();
                    int l = ss->getSolutionPath().asGeometric().getStateCount();

                    for(int i=0;i<l;i++){
                        for(int j=0;j<d;j++)
                          coords[j]=ss->getSolutionPath().asGeometric().getState(i)->as<ob::RealVectorStateSpace::StateType>()->values[j];
                         //   coords[j]=pathstates[i]->as<ob::RealVectorStateSpace::StateType>()->values[j];

                        //coords[0] = ss->getSolutionPath().asGeometric().getState(i)->as<ob::SE2StateSpace::StateType>()->getX();
                        //coords[1] = ss->getSolutionPath().asGeometric().getState(i)->as<ob::SE2StateSpace::StateType>()->getY();
                        //coords[2] = ss->getSolutionPath().asGeometric().getState(i)->as<ob::SE2StateSpace::StateType>()->getYaw();

                        smp->setCoords(coords);
                        //_wkSpace->collisionCheck(smp);
                        _path.push_back(smp);
                        smp=new Sample(d);
                   }


                    _solved = true;
                    drawCspace();
                    return _solved;
                }
                else{
                    std::cout << "No solution found (solved=<<"<<solved.asString()<<")" << std::endl;
                    _solved = false;
                    drawCspace();
                    return _solved;
            }
		}
    }
}

#endif // KAUTHAM_USE_OMPL


