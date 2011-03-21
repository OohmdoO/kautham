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

#include "problem.h"
#include <libplanner/linearlocplan.h>
#include <libplanner/constlinearlocplan.h>
#include <libplanner/ML_locplan.h>

#include <libplanner/myplanner.h>
#include <libplanner/drmplanner.h>
#include <libplanner/prmplanner.h>
#include <libplanner/prmplanner_pca.h>
#include <libplanner/rrtplanner.h>
#include <libplanner/prmhandplannerICRA.h>
#include <libplanner/prmhandplannerICRAthumb.h>
#include <libplanner/prmhandplannerICRAjournal.h>
#include <libplanner/prmhandplannerarmhandpca.h>
#include <libplanner/prmrobothandconstplannerICRA.h>
#include <libplanner/prmhandplannerIROS.h>
#include <libplanner/myprmplanner.h>
#include <libplanner/mygridplanner.h>
#include <libplanner/NF1planner.h>
#include <libplanner/HFplanner.h>
#include <libutil/pugixml/pugixml.hpp> 
#include <string>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>

#if !defined(M_PI)
#define M_PI 3.1415926535897932384626433832795
#endif


using namespace std;
using namespace DRM;
using namespace PRM;
using namespace RRT;
using namespace myplanner;
using namespace myprmplanner;
using namespace gridplanner;
using namespace NF1_planner;
using namespace HF_planner;
using namespace mygridplanner;
using namespace libPlanner;
using namespace pugi;

namespace libProblem {
  const KthReal Problem::_toRad = (KthReal)(M_PI/180.0);
  Problem::Problem() {
		_wspace = NULL;
    _locPlanner = NULL;
    _planner = NULL;
    _sampler = NULL;
    _cspace = new SampleSet();
    _cspace->clear();
    _qStart.clear();
		_qGoal.clear();
    _currentControls.clear();
	}

  Problem::~Problem(){
    delete _wspace;
    delete _locPlanner;
    delete _planner;
    delete _sampler;
    delete _cspace; 
  }
	
  // This is the new implementation trying to avoid the old strucparse and ProbStruc.
  bool Problem::createWSpace(string xml_doc){
    if(_wspace != NULL ) delete _wspace;
    _wspace = new IVWorkSpace();

    xml_document doc;

    xml_parse_result result = doc.load_file(xml_doc.c_str());

    if(!result) return false;

    // Setup the examples directory.
    string dir = xml_doc;

    dir.erase(dir.find_last_of("/") + 1 );

    KthReal pob[3], oob[4];
    Obstacle *obs;
    Robot *rob;
    bool flagCol;

    xml_node tmpNode = doc.child("Problem");
    string name = "";
    for(xml_node_iterator it = tmpNode.begin(); it != tmpNode.end(); ++it){ // Processing each child
      name = it->name();
      // it can be an Scene or a fixed obstacle.
      if(name == "Scene" ){
        if((*it).child("Collision"))
          flagCol = (*it).child("Collision").attribute("Enable").as_bool();
        else
          flagCol = true;

        xml_node locObs = (*it).child("Location");
        pob[0] = (KthReal)locObs.attribute("X").as_double();
        pob[1] = (KthReal)locObs.attribute("Y").as_double();
        pob[2] = (KthReal)locObs.attribute("Z").as_double();
        oob[0] = (KthReal)locObs.attribute("WX").as_double();
        oob[1] = (KthReal)locObs.attribute("WY").as_double();
        oob[2] = (KthReal)locObs.attribute("WZ").as_double();
        oob[3] = (KthReal)locObs.attribute("TH").as_double() * _toRad;

        // Changing between axis angle to quaternion.
        SE3Conf::fromAxisToQuaternion(oob);

#ifndef KAUTHAM_COLLISION_PQP
        obs = new Obstacle(dir + (*it).attribute("scene").value(), pob, oob,
                                    (KthReal)(*it).attribute("scale").as_double(), INVENTOR, flagCol);
#else
        obs = new Obstacle( dir + (*it).attribute("scene").value(), pob, oob,
                          (KthReal)(*it).attribute("scale").as_double(), IVPQP, flagCol);
#endif
        _wspace->addObstacle(obs);
      }

      // it can be a Robot.
      if(name == "Robot" ){
#ifndef KAUTHAM_COLLISION_PQP
        rob = new Robot( dir + (*it).attribute("robot").value(),
                        (KthReal)(*it).attribute("scale").as_double(),INVENTOR);
#else
        rob = new Robot( dir + (*it).attribute("robot").value(),
                        (KthReal)(*it).attribute("scale").as_double(),IVPQP);
#endif

        // Setup the Inverse Kinematic if it has one.
        if((*it).child("InvKinematic")){
          name = (*it).child("InvKinematic").attribute("name").value();
          if( name == "TX90")
            rob->setInverseKinematic( Kautham::TX90 );
          else if( name == "HAND")
            rob->setInverseKinematic( Kautham::HAND );
          else if( name == "TX90HAND")
            rob->setInverseKinematic( Kautham::TX90HAND );
          else
            rob->setInverseKinematic(Kautham::UNIMPLEMENTED);
        }else
          rob->setInverseKinematic(Kautham::UNIMPLEMENTED);

        // Setup the Constrained Kinematic if it has one.
        if((*it).child("ConstrainedKinematic")){
          name = (*it).child("ConstrainedKinematic").attribute("name").value();
          if( name == "BRONCHOSCOPY" )
            rob->setConstrainedKinematic( Kautham::BRONCHOSCOPY );
          else
            rob->setConstrainedKinematic( Kautham::UNCONSTRAINED );
        }else{
          rob->setConstrainedKinematic( Kautham::UNCONSTRAINED );
        }

        // Setup the limits of the moveable base
        for(xml_node_iterator itL = (*it).begin(); itL != (*it).end(); ++itL){
          name = (*itL).name();
          if( name == "Limits" ){
            name = (*itL).attribute("name").value();
            if( name == "X")
              rob->setLimits(0, (KthReal)(*itL).attribute("min").as_double(),
                             (KthReal)(*itL).attribute("max").as_double());
            else if( name == "Y")
              rob->setLimits(1, (KthReal)(*itL).attribute("min").as_double(),
                             (KthReal)(*itL).attribute("max").as_double());
            else if( name == "Z")
              rob->setLimits(2, (KthReal)(*itL).attribute("min").as_double(),
                             (KthReal)(*itL).attribute("max").as_double());
            else if( name == "WX")
              rob->setLimits(3, (KthReal)(*itL).attribute("min").as_double(),
                             (KthReal)(*itL).attribute("max").as_double());
            else if( name == "WY")
              rob->setLimits(4, (KthReal)(*itL).attribute("min").as_double(),
                             (KthReal)(*itL).attribute("max").as_double());
            else if( name == "WZ")
              rob->setLimits(5, (KthReal)(*itL).attribute("min").as_double(),
                             (KthReal)(*itL).attribute("max").as_double());
            else if( name == "TH")
              rob->setLimits(6, (KthReal)(*itL).attribute("min").as_double() * _toRad,
                             (KthReal)(*itL).attribute("max").as_double() * _toRad);
          }
          if( name == "Home" ){
          // If robot hasn't a home, it will be assumed in the origin.
            SE3Conf tmpC;
            vector<KthReal> cords(7);
            cords[0] = (KthReal)(*itL).attribute("X").as_double();
            cords[1] = (KthReal)(*itL).attribute("Y").as_double();
            cords[2] = (KthReal)(*itL).attribute("Z").as_double();
            cords[3] = (KthReal)(*itL).attribute("WX").as_double();
            cords[4] = (KthReal)(*itL).attribute("WY").as_double();
            cords[5] = (KthReal)(*itL).attribute("WZ").as_double();
            cords[6] = (KthReal)(*itL).attribute("TH").as_double() * _toRad;

            // Here is needed to convert from axis-angle to
            // quaternion internal represtantation.
            SE3Conf::fromAxisToQuaternion(cords);

            tmpC.setCoordinates(cords);

            //cout << tmpC.print();

            rob->setHomePos(&tmpC);
          }
        }

        _wspace->addRobot(rob);
      }

    }// closing for(xml_node_iterator it = tmpNode.begin(); it != tmpNode.end(); ++it){ // Processing each child

    _currentControls.clear();
    _currentControls.resize(_wspace->getDimension());

    for(int i = 0; i<_wspace->getDimension(); i++)
      _currentControls[i] = (KthReal)0.0;
    return true;
  }

	WorkSpace* Problem::wSpace(){
		return _wspace;
	}

  void Problem::setHomeConf(Robot* rob, HASH_S_K* param){
    Conf* tmpC = new SE3Conf();
    vector<KthReal> cords(7);
    string search[]={"X", "Y", "Z", "WX", "WY", "WZ", "TH"};
    HASH_S_K::iterator it;

    for(int i = 0; i < 7; i++){
      it = param->find(search[i]);
      if( it != param->end())
        cords[i]= it->second;
      else
        cords[i] = 0.0;
    }

    // Here is needed to convert from axis-angle to
    // quaternion internal represtantation.
    SE3Conf::fromAxisToQuaternion(cords);
    
    tmpC->setCoordinates(cords);
    rob->setHomePos(tmpC);
    delete tmpC;
    cords.clear();

    if(rob->getNumJoints() > 0){
      cords.resize(rob->getNumJoints());
      tmpC = new RnConf(rob->getNumJoints());
      for(int i = 0; i < tmpC->getDim(); i++){
        it = param->find(rob->getLink(i+1)->getName());
        if( it != param->end())
          cords[i]= it->second;
        else
          cords[i] = 0.0;
      }

      tmpC->setCoordinates(cords);
      rob->setHomePos(tmpC);
      delete tmpC;
    }
  }


	void Problem::setStartConf(Robot* rob, HASH_S_K* param){
    Conf* tmpC = new SE3Conf();
    vector<KthReal> cords(7);
    string search[]={"X", "Y", "Z", "WX", "WY", "WZ", "TH"};
    HASH_S_K::iterator it;

    for(int i = 0; i < 7; i++){
      it = param->find(search[i]);
      if( it != param->end())
        cords[i]= it->second;
      else
        cords[i] = 0.0;
    }  
    // Here is needed to convert from axis-angle to
    // quaternion internal represtantation.
    SE3Conf::fromAxisToQuaternion(cords);

    tmpC->setCoordinates(cords);
    rob->setInitPos(tmpC);
    delete tmpC;
    cords.clear();

    if(rob->getNumJoints() > 0){
      cords.resize(rob->getNumJoints());
      tmpC = new RnConf(rob->getNumJoints());
      for(int i = 0; i < tmpC->getDim(); i++){
        it = param->find(rob->getLink(i+1)->getName());
        if( it != param->end())
          cords[i]= it->second;
        else
          cords[i] = 0.0;
      }
      tmpC->setCoordinates(cords);
      rob->setInitPos(tmpC);
      delete tmpC;
    }
	}

	void Problem::setGoalConf(Robot* rob, HASH_S_K* param){
    Conf* tmpC = new SE3Conf();
    vector<KthReal> cords(7);
    string search[]={"X", "Y", "Z", "WX", "WY", "WZ", "TH"};
    HASH_S_K::iterator it;

    for(int i = 0; i < 7; i++){
      it = param->find(search[i]);
      if( it != param->end())
        cords[i]= it->second;
      else
        cords[i] = 0.0;
    }  

    // Here is needed to convert from axis-angle to
    // quaternion internal represtantation.
    SE3Conf::fromAxisToQuaternion(cords);

    tmpC->setCoordinates(cords);
    rob->setGoalPos(tmpC);
    delete tmpC;
    cords.clear();

    if(rob->getNumJoints() > 0){
      cords.resize(rob->getNumJoints());
      tmpC = new RnConf(rob->getNumJoints());
      for(int i = 0; i < tmpC->getDim(); i++){
        it = param->find(rob->getLink(i+1)->getName());
        if( it != param->end())
          cords[i]= it->second;
        else
          cords[i] = 0.0;
      }
      tmpC->setCoordinates(cords);
      rob->setGoalPos(tmpC);
      delete tmpC;
    }
	}

  string Problem::localPlannersNames(){
    return "Linear|Const Linear|Manhattan Like";
  }

  bool Problem::createLocalPlanner(string name, KthReal step){
    //if(_locPlanner != NULL)
      delete _locPlanner;

    if(name == "Const Linear")
      _locPlanner = new ConstLinearLocalPlanner(CONTROLSPACE, NULL,NULL,
                                           _wspace, step);

    else if(name == "Linear")
      _locPlanner = new LinearLocalPlanner(CONTROLSPACE, NULL,NULL,
                                      _wspace, step);

    else if(name == "Manhattan Like")
      _locPlanner = new ManhattanLikeLocalPlanner(CONTROLSPACE, NULL,NULL,
                                             _wspace, step);

    if(_locPlanner != NULL)
      return true;
    else
      return false;
  }

  
  bool Problem::createLocalPlannerFromFile(string path){
    xml_document doc;
    xml_parse_result result = doc.load_file( path.c_str() );
    if( result ){
      xml_node locplanNode = doc.child("Problem").child("Planner").child("Parameters").child("LocalPlanner");

      if(_locPlanner != NULL ) delete _locPlanner;

      string name = locplanNode.child_value();
      if( name != "" ){
        createLocalPlanner(name, (KthReal)locplanNode.attribute("stepSize").as_double() );
        return true;
      }
    }
    return false;
  }

  string Problem::plannersNames(){
    return   "DRM|PRM|PRM PCA|RRT|PRM Hand IROS|PRM Hand ICRA|PRM Hand-Thumb ICRA|PRM RobotHand-Const ICRA|PRM RobotHand ICRA J|PRM RobotArmHand PCA|MyPlanner|MyPRMPlanner|MyGridPlanner|NF1Planner|HFPlanner";
  }

  bool Problem::createPlanner( string name, KthReal step ){
    if(_planner != NULL )
      delete _planner;

    if(_locPlanner == NULL ) return false;
    if(name == "DRM")
      _planner = new DRMPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, step);
    else if(name == "PRM")
      _planner = new PRMPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, step);
	else if(name == "PRM PCA")
      _planner = new PRMPlannerPCA(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, step,1,1);
	else if(name == "RRT")
      _planner = new RRTPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, (KthReal)0.01);
    else if(name == "PRM Hand IROS")
      _planner = new PRMHandPlannerIROS(CONTROLSPACE, NULL, NULL,
                                       _cspace, _sampler, _wspace, _locPlanner,
                                       step, 5, (KthReal)0.001 );
    else if(name == "PRM Hand ICRA")
      _planner = new PRMHandPlannerICRA(CONTROLSPACE, NULL, NULL,
                                       _cspace, _sampler, _wspace, _locPlanner,
                                       step, 100, 5, (KthReal)0.010, 5);
    else if(name == "PRM Hand-Thumb ICRA")
      _planner = new PRMHandPlannerICRAthumb(CONTROLSPACE, NULL, NULL, _cspace,
                                             _sampler, _wspace, _locPlanner,
                                             step, 10, (KthReal)0.0010, 10);
	else if(name == "PRM RobotHand ICRA J")
      _planner = new PRMHandPlannerICRAjournal(CONTROLSPACE, NULL, NULL, _cspace,
                                             _sampler, _wspace, _locPlanner,
                                             step, 10, (KthReal)0.0010, 10);
	//////////////////////////////////////////////////////////////////////////
	else if(name == "PRM RobotArmHand PCA")
      _planner = new PRMHandPlannerArmHandPCA(CONTROLSPACE, NULL, NULL, _cspace,
                                             _sampler, _wspace, _locPlanner,
                                             step, 10,0, (KthReal)0.0010, 10,0.0,0.0);
	//////////////////////////////////////////////////////////////////////////
   else if(name == "PRM RobotHand-Const ICRA")
      _planner = new PRMRobotHandConstPlannerICRA(CONTROLSPACE, NULL, NULL, _cspace,
                                             _sampler, _wspace, _locPlanner,
                                             step, 3, (KthReal)50.0);                                        
   	 else if(name == "MyPlanner")
      _planner = new MyPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, (KthReal)0.01); 
	  
	 else if(name == "MyPRMPlanner")
      _planner = new MyPRMPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, (KthReal)0.01);
	   
	  else if(name == "MyGridPlanner")
      _planner = new MyGridPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, (KthReal)0.01);	   
	   
	   else if(name == "NF1Planner")
      _planner = new NF1Planner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, (KthReal)0.01);  
	   
	   else if(name == "HFPlanner")
      _planner = new HFPlanner(CONTROLSPACE, NULL, NULL,
                               _cspace, _sampler, _wspace, _locPlanner, (KthReal)0.01);
   
   

   

   

    if(_planner != NULL)
      return true;
    else
      return false;
  }

  bool Problem::createPlannerFromFile(string path){
    if(_planner == NULL ){
      xml_document doc;
      xml_parse_result result = doc.load_file( path.c_str() );
      if( result ){
        xml_node planNode = doc.child("Problem").child("Planner").child("Parameters");
        string name = planNode.child("Name").child_value();

        if( name != ""){
          createPlanner( name );
          xml_node::iterator it;
          for(it = planNode.begin(); it != planNode.end(); ++it){
            name = it->name();
            try{
              if( name == "Parameter" ){
                name = it->attribute("name").value();
                _planner->setParametersFromString( name.append("|").append(it->child_value()));
              }
            }catch(...){
              std::cout << "Current planner doesn't have at least one of the parameters" 
                << " you found in the file (" << name << ")." << std::endl; 
              return false; //if it is wrong maybe the file has been generated with another planner.
            }
          }
          return true;
        }
      }
    }
    return false;
  }


  bool Problem::createCSpaceFromFile(string xml_doc){
    if( createCSpace() ){
      xml_document doc;
      xml_parse_result res = doc.load_file(xml_doc.c_str());
      if( res ){
        xml_node queryNode = doc.child("Problem").child("Planner").child("Queries");
        xml_node::iterator it;
        int i = 0;
        vector<string> tokens;
        string sentence = "";
        Sample* tmpSampPointer = NULL;
        unsigned int dim = _wspace->getDimension();
        vector<KthReal> coordsVec( dim );
        for( it = queryNode.begin(); it != queryNode.end(); ++it ){
          xml_node sampNode = it->child("Init");
          if( dim == sampNode.attribute("dim").as_int() ){
            sentence = sampNode.child_value();

            boost::split(tokens, sentence, boost::is_any_of("| "));
            if(tokens.size() != dim){
              std::cout << "Dimension of a samples doesn't correspond with the problem's dimension.\n";
              break;
            }

            tmpSampPointer = new Sample(dim);
            for(char i=0; i<dim; i++)
              coordsVec[i] = (KthReal)atof(tokens[i].c_str());

            tmpSampPointer->setCoords(coordsVec);
            _cspace->add(tmpSampPointer);

          }else{
            cout << "Dimension of a samples doesn't correspond with the problem's dimension.\n";
            break;
          }

          sampNode = it->child("Goal");
          if( dim == sampNode.attribute("dim").as_int() ){
            sentence = sampNode.child_value();

            boost::split(tokens, sentence, boost::is_any_of("| "));
            if(tokens.size() != dim){
              std::cout << "Dimension of a samples doesn't correspond with the problem's dimension.\n";
              break;
            }

            tmpSampPointer = new Sample(dim);
            for(char i=0; i<dim; i++)
              coordsVec[i] = (KthReal)atof(tokens[i].c_str());

            tmpSampPointer->setCoords(coordsVec);
            _cspace->add(tmpSampPointer);

          }else{
            cout << "Sample doesn't have the right dimension.\n";
            break;
          }
        }
        if( _cspace->getSize() >= 2 )
          return true;
      }
    }
    return false;
  }

  bool Problem::createCSpace(){
    try{
      if(_cspace == NULL) _cspace = new SampleSet();
      if(_sampler == NULL) _sampler = new RandomSampler(_wspace->getDimension());
      _cspace->clear();
      return true;
    }catch(...){
      return false;
    }
  }

  bool Problem::setCurrentControls(vector<KthReal> &val, int offset){
    try{
      for(unsigned int i=0; i < val.size(); i++)
        _currentControls[i+offset] = (KthReal)val[i];
      return true;
    }catch(...){
      return false;
    }

  }

  bool Problem::setupFromFile(string xml_doc){
    _filePath = xml_doc;
    if( createWSpace( xml_doc ) &&
        createCSpaceFromFile( xml_doc ) &&
        createLocalPlannerFromFile( xml_doc ) &&
        createPlannerFromFile( xml_doc ) )
        return true;
    else
      return false;
  }

  bool Problem::saveToFile(string file_path){
    if( file_path == "" )  file_path = _filePath;
    if( _filePath != file_path ){ // If save as 
      ifstream initialFile(_filePath.c_str(), ios::in|ios::binary);
	    ofstream outputFile(file_path.c_str(), ios::out|ios::binary);

	    //As long as both the input and output files are open...
	    if(initialFile.is_open() && outputFile.is_open()){
        outputFile << initialFile.rdbuf() ;
	    }else           //there were any problems with the copying process
        return false;
    		
	    initialFile.close();
	    outputFile.close();
    }

    xml_document doc;
    xml_parse_result result = doc.load_file( file_path.c_str() );
    if( !result ) return false;

    if( _planner == NULL || _locPlanner == NULL ) return false;
    if( _planner->initSamp() == NULL || _planner->goalSamp() == NULL ) 
      return false;

    if( doc.child("Problem").child("Planner") )
      doc.child("Problem").remove_child("Planner");

    xml_node planNode = doc.child("Problem").append_child();
    planNode.set_name("Planner");
    xml_node paramNode = planNode.append_child();
    paramNode.set_name("Parameters");
    xml_node planname = paramNode.append_child();
    planname.set_name("Name");
    planname.append_child(node_pcdata).set_value(_planner->getIDName().c_str());
    
    xml_node localPlan = paramNode.append_child();
    localPlan.set_name("LocalPlanner");
    localPlan.append_child(node_pcdata).set_value(_locPlanner->getIDName().c_str());
    localPlan.append_attribute("stepSize") = _locPlanner->stepSize();

    // Adding the parameters
    string param = _planner->getParametersAsString();
    vector<string> tokens;
    boost::split(tokens, param, boost::is_any_of("|"));

    for(int i=0; i<tokens.size(); i=i+2){
      xml_node paramItem = paramNode.append_child();
      paramItem.set_name("Parameter");
      paramItem.append_attribute("name") = tokens[i].c_str();
      paramItem.append_child(node_pcdata).set_value(tokens[i+1].c_str());
    }

    // Adding the Query information
    
    xml_node queryNode = planNode.append_child();
    queryNode.set_name("Queries");

    xml_node queryItem = queryNode.append_child();
    queryItem.set_name("Query");
    xml_node initNode = queryItem.append_child();
    initNode.set_name( "Init" );
    initNode.append_attribute("dim") = _wspace->getDimension();
    initNode.append_child(node_pcdata).set_value( _planner->initSamp()->print(true).c_str() );
    xml_node goalNode = queryItem.append_child();
    goalNode.set_name( "Goal" );
    goalNode.append_attribute("dim") = _wspace->getDimension();
    goalNode.append_child(node_pcdata).set_value( _planner->goalSamp()->print(true).c_str() );
    
    return doc.save_file(file_path.c_str());
  }

  bool Problem::inheritSolution(){
    if(_planner->isSolved()){
      _wspace->inheritSolution(*(_planner->getSimulationPath()));
      return true;
    }
    return false;
  }
}

