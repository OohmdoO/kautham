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
 
 

#if !defined(_PLANNER_H)
#define _PLANNER_H

#include <libproblem/workspace.h>
#include <libsampling/sampling.h>
#include <cmath>
#include <string>
#include "localplanner.h"
#include <mt/transform.h>
#include "kthquery.h"

using namespace std;
using namespace libSampling;
using namespace Kautham;

namespace libPlanner {
  class Planner: public KauthamObject{
	public:
    Planner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, 
      WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize);

    ~Planner();
    virtual bool                  trySolve()=0;
    virtual bool                  setParameters() = 0;
    virtual void                  moveAlongPath(unsigned int step);
    virtual bool                  solveAndInherit();

    //! This is a simple save method to store the samples set. It should be overload in the
    //! derived class in order to save the connectivity and the solutions if exists. The
    //! data is saved in a KPS (Kautham Planner Solution)file using xml format. For an
    //! example see /libplanner/output_file_model.kps
    virtual bool                  saveData(string path);

    //! This functions provides the user to save any interested data in a convenient way. 
    //! Data saved using this method can not be loaded by the loadData method. This function
    //! is called when the standard saveData method is finishing.
    virtual void                  saveData();

    //! This method is a simple load method to restore the samples set. It should be
    //! overload in the derived class in order to load the connectivity and the solutions
    //! if exists.  The
    //! data is saved in a KPS (Kautham Planner Solution)file using xml format. For an
    //! example see /libplanner/output_file_model.kps
    virtual bool                  loadData(string path);

    inline string                 getIDName(){return _idName;}
    inline void                   setSampler(Sampler* smp){_sampler = smp;}
    inline Sampler*               getSampler(){return _sampler;}
    inline void                   setSampleSet(SampleSet* smpSet){_samples = smpSet;}
    inline Sample*                initSamp(){return _init;}
    inline Sample*                goalSamp(){return _goal;}
    inline void                   setInitSamp(Sample* init){_init = init;}
    inline void                   setGoalSamp(Sample* goal){_goal = goal;}
    inline WorkSpace*             wkSpace(){return _wkSpace;}
    inline void                   setWorkSpace(WorkSpace *ws){_wkSpace = ws;}
    inline LocalPlanner*          lcPlanner(){return _locPlanner;}
    inline void                   setLocalPlanner(LocalPlanner *lcPlan){_locPlanner = lcPlan;}
    inline KthReal                stepSize(){return _stepSize;}
    inline void                   setStepSize(KthReal step){_stepSize = step; _locPlanner->setStepSize(step);}
    inline vector<Sample*>*       getPath(){if(_solved)return &_path;else return NULL;}
    inline int                    getSpeedFactor(){return _speedFactor;}
    inline void                   setSpeedFactor(int sf){_speedFactor = sf;}
    inline long int               getMaxNumSamples(){return _maxNumSamples;}
    void                          clearSimulationPath();
    inline bool                   isSolved(){return _solved;}
    inline vector<Sample*>*       getSimulationPath(){if(_solved)return &_simulationPath;else return NULL;}
    inline void                   setCameraMovements(bool c){_hasCameraInformation = c;}
    inline bool                   hasCameraMovements(){return _hasCameraInformation;}
    inline void                   addCameraMovement(mt::Transform &t){_cameraPath.push_back(t);}
    inline void                   clearCameraMovement(){ _cameraPath.clear();}
    inline mt::Transform*         getCameraMovement(unsigned int step){
      if(_solved && _cameraPath.size() > 0 )
        return &_cameraPath.at(step % _simulationPath.size());
      else
        return NULL;
    }

    inline vector<KthQuery>&       getQueries(){return _queries;}
    bool                           addQuery(int init, int goal );
    int                            findQuery(int init, int goal, int from = 0);

	  virtual inline SoSeparator*    getIvCspaceScene(){return _sceneCspace;};//_sceneCspace is initiallized to NULL


	protected:
    Planner();
    vector<KthQuery>              _queries;
    std::string                   _idName;
    Sampler*                      _sampler;
    SampleSet*                    _samples;
    Sample*                       _init;
    Sample*                       _goal;
    vector<Sample*>               _path;
    vector<Sample*>               _simulationPath;
    vector<mt::Transform>         _cameraPath;
    WorkSpace*                    _wkSpace;
    LocalPlanner*                 _locPlanner;
    KthReal                       _stepSize;
    bool                          _solved;
    SPACETYPE                     _spType;
    int                           _speedFactor;
    bool                          _hasCameraInformation;
	
	  SoSeparator*                  _sceneCspace;

    // Stats
    unsigned int                  _maxNumSamples;
    unsigned int                  _triedSamples;
    unsigned int                  _generatedEdges;
    KthReal                       _totalTime;
    KthReal                       _smoothTime;


	};
}

#endif  //_PLANNER_H

