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
 
 

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "planner.h"
#include <libutil/pugixml/pugixml.hpp>

using namespace Kautham;
using namespace pugi;

namespace libPlanner{
  Planner::Planner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler,
    WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize){
    _guiName = _idName = "";
	  _sampler = sampler;
		_spType = stype;		
    _init = init; 
    _goal = goal; 
    _samples = samples; 
    _wkSpace = ws; 
    _locPlanner = lcPlan; 
    _stepSize = ssize;
    _path.clear();
    _parameters.clear();
    _solved = false;
    _hasCameraInformation = false;
    _triedSamples = _maxNumSamples = _generatedEdges = 0;
    _totalTime = _smoothTime = 0. ;
	}

  Planner::~Planner(){

  }

  bool Planner::addQuery(int init, int goal ){
    KthQuery tmp(init, goal);
    _queries.push_back(tmp);
      return true;
  }

  int Planner::findQuery(int init, int goal, int from){
    if( from < _queries.size() ){
      for(int i = from; i < _queries.size(); i++)
        if( _queries[i].sameInitGoal(init, goal) ) return i;
    }
    return -1;
  }

  bool Planner::solveAndInherit(){
    libProblem::Element::resetCollCheckCounter();
    if(trySolve()){
      moveAlongPath(0);
      _wkSpace->inheritSolution(_simulationPath);

      // Add the results to the Query vector.
      KthQuery* currQue = NULL;
      addQuery( _samples->indexOf( _init ), _samples->indexOf( _goal ));
      currQue = &(_queries.at( _queries.size() - 1 ));
      currQue->solved(_solved);
      currQue->setSampleStats(_triedSamples, _samples->getSize(), _generatedEdges);
      currQue->setTotalTime( _totalTime );
      currQue->setSmoothTime( _smoothTime );
      vector<int> solu;
      for(int i = 0; i < _path.size(); i++)
        solu.push_back(_samples->indexOf( _path[i] ));
      currQue->setPath( solu );
      return true;
    }
    return false;
  }
  
  void Planner::clearSimulationPath(){
    for(unsigned int i = 0; i < _simulationPath.size(); i++)
      delete _simulationPath[i];
    _simulationPath.clear();
    _wkSpace->eraseSolution();
  }
  
  void Planner::moveAlongPath(unsigned int step){
    if(_solved){
      if(_simulationPath.size() == 0 ){ // Then calculate the simulation path based on stepsize
		    char sampleDIM = _path[0]->getDim();
        unsigned int maxsteps = 0;
        Sample* tmpSam;
        KthReal dist = (KthReal)0.0;
        KthReal* steps = new KthReal[sampleDIM];
        vector<KthReal> tmpCoords(sampleDIM);
        if(_path.size() >=2 ){
          for(unsigned i = 0; i < _path.size()-1; i++){

            for(int k = 0; k < sampleDIM; k++){
			        steps[k] = _path[i+1]->getCoords()[k] - _path[i]->getCoords()[k];
		    }
            dist = wkSpace()->distanceBetweenSamples(*_path.at(i), *_path.at(i+1), Kautham::CONFIGSPACE);
		    maxsteps = (dist/_stepSize) + 2; //the 2 is necessary to always reduce the distance...???
			  
            for(unsigned int j = 0; j < maxsteps; j++){
			  tmpSam = _path[i]->interpolate(_path[i+1],j/(KthReal)maxsteps);
			  _simulationPath.push_back(tmpSam);
            }
          }
        }
        delete []steps;
		tmpSam = new Sample(*_path.at(_path.size()-1));
        _simulationPath.push_back(tmpSam);
      }
      if( _simulationPath.size() >= 2 ){
        step = step % _simulationPath.size();
        _wkSpace->moveTo(_simulationPath[step]);
      }else
        std::cout << "The problem is wrong solved. The solution path has less than two elements." << std::endl;
    }
  }

  bool Planner::saveData(string path){

    xml_document doc;
    xml_node planNode = doc.append_child();
    planNode.set_name("Planner");
    planNode.append_attribute("ProblemName") = "";
    planNode.append_attribute("Date") = "";
    xml_node paramNode = planNode.append_child();
    paramNode.set_name("Parameters");
    xml_node planname = paramNode.append_child();
    planname.set_name("Name");
    planname.append_child(node_pcdata).set_value(_idName.c_str());
    
    xml_node localPlan = paramNode.append_child();
    localPlan.set_name("LocalPlanner");
    localPlan.append_child(node_pcdata).set_value(_locPlanner->getIDName().c_str());
    localPlan.append_attribute("stepSize") = _locPlanner->stepSize();


    // Adding the parameters
    string param = getParametersAsString();
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
    vector<KthQuery>::iterator it = _queries.begin();
    for(it = _queries.begin(); it != _queries.end(); ++it){
      xml_node queryItem = queryNode.append_child();
      queryItem.set_name("Query");
      queryItem.append_attribute("Init") = (*it).printInit().c_str();
      queryItem.append_attribute("Goal") = (*it).printGoal().c_str();
      queryItem.append_attribute("Solved") = (*it).printSolved().c_str();

      xml_node queryItemResultP = queryItem.append_child();
      queryItemResultP.set_name("Result");
      queryItemResultP.append_attribute("name") = "Path";
      queryItemResultP.append_child(node_pcdata).set_value( (*it).printPath().c_str() );

      xml_node queryItemResultT = queryItem.append_child();
      queryItemResultT.set_name("Result");
      queryItemResultT.append_attribute("name") = "TotalTime";
      queryItemResultT.append_child(node_pcdata).set_value( (*it).printTotalTime().c_str() );

      xml_node queryItemResultST = queryItem.append_child();
      queryItemResultST.set_name("Result");
      queryItemResultST.append_attribute("name") = "SmoothTime";
      queryItemResultST.append_child(node_pcdata).set_value( (*it).printSmoothTime().c_str() );

      xml_node queryItemResultG = queryItem.append_child();
      queryItemResultG.set_name("Result");
      queryItemResultG.append_attribute("name") = "Generated Samples";
      queryItemResultG.append_child(node_pcdata).set_value( (*it).printGeneratedSamples().c_str() );

      xml_node queryItemResultE = queryItem.append_child();
      queryItemResultE.set_name("Result");
      queryItemResultE.append_attribute("name") = "Generated Edges";
      queryItemResultE.append_child(node_pcdata).set_value( (*it).printGeneratedEdges().c_str() );
    }

    //  Now it is adding the samples set.
    xml_node sampNode = planNode.append_child();
    sampNode.set_name("SampleSet");
    sampNode.append_attribute("dim") = _wkSpace->getDimension();
    sampNode.append_attribute("size") = _samples->getSize();
    for(int i = 0; i < _samples->getSize(); i++){
      xml_node sampItem = sampNode.append_child();
      sampItem.set_name("Sample");
      sampItem.append_attribute("conComp") = _samples->getSampleAt(i)->getConnectedComponent();
      sampItem.append_child(node_pcdata).set_value(_samples->getSampleAt(i)->print(true).c_str());
    }
    
    saveData();
    return doc.save_file(path.c_str());
  }

  bool Planner::loadData(string path){
    // First, I will clean SampleSet
    _samples->clear();

    xml_document doc;
    xml_parse_result result = doc.load_file(path.c_str());

    xml_node tempNode = doc.child("Planner").child("Parameters");
    std::stringstream temp;
    if( !(_idName.compare(tempNode.child("Name").value()) && // it means the solution has been produced with the same planner
        _locPlanner->getIDName().compare(tempNode.child("LocalPlanner").value())))
      return false;
    std::string par="";
    for (pugi::xml_node_iterator it = tempNode.begin(); it != tempNode.end(); ++it){
      par = it->name();
      if( par == "Parameter" ){
        temp << it->attribute("name").value() << "|";
        temp << it->child_value() << "|";
      }
    }
    par = temp.str();
    try{
      setParametersFromString(par.substr(0,par.length()-1)); // Trying to set the read parameter
    }catch(...){
      std::cout << "Current planner doesn't have at least one of the parameters" 
        << " you found in the file (" << par << ")." << std::endl; 
      return false; //if it is wrong maybe the file has been generated with another planner.
    }

    tempNode = doc.child("Planner").child("SampleSet");

    char dim = _wkSpace->getDimension();
    if( dim != tempNode.attribute("dim").as_int()){
      std::cout << "Dimension of samples doesn't correspond with the problem's dimension."
          << std::endl;
      return false;
    }

    vector<KthReal> coordsVec(dim);
    Sample* tmpSampPointer=NULL;

    for (pugi::xml_node_iterator it = tempNode.begin(); it != tempNode.end(); ++it){
      string sentence = it->child_value();
      vector<string> tokens;
      boost::split(tokens, sentence, boost::is_any_of("| "));
      if(tokens.size() != dim){
        std::cout << "Dimension of a samples doesn't correspond with the problem's dimension."
            << std::endl;
        return false;
      }

      tmpSampPointer = new Sample(dim);
      for(char i=0; i<dim; i++)
        coordsVec[i] = (KthReal)atof(tokens[i].c_str());

      tmpSampPointer->setCoords(coordsVec);
      tmpSampPointer->setConnectedComponent(it->attribute("conComp").as_int());
      _samples->add(tmpSampPointer);
    }
    if(_samples->getSize() != tempNode.attribute("size").as_int()){
      std::cout << "Something wrong with the samples. Not all samples has been loaded."
          << std::endl;
      return false;
    }
    return true;
  }

  void Planner::saveData(){};
}

