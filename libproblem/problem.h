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
 
  
 
#if !defined(_PROBLEM_H)
#define _PROBLEM_H

#include <libsplanner/planner.h>
#include <libsplanner/localplanner.h>
#include <libsampling/sampling.h>
#include "robot.h"
#include "ivworkspace.h"
#include "workspace.h"


namespace Kautham {

/** \addtogroup libProblem
 *  @{
 */

	class Problem {
  public:
    Problem();
    ~Problem();

    //!	This member function create the work phisycal space.
    /*!	With this fuction you can to create the work phisycal space that represent
    *		the problem. One or more Robot and one or more obstacle  compose it.
    *		These Robots could be both the freefly type or cinematic chain type, it
    *		mean that if a problem contain many robots all of them should be the
    *		same class.
    *		\sa WSpace Robot ChainRobot Obstacle*/
    bool			              createWSpace(string xml_doc);

    //! This method is deprecated. Please take care with the problem XML file.
    //bool			              createWSpace(ProbStruc *reader);

    static string           plannersNames();
    bool                    createPlanner(string name, KthReal step = 0.05);
    bool                    createPlannerFromFile(string path);
    static string           localPlannersNames();
    bool                    createLocalPlanner(string name, KthReal step = 0.5);
    bool                    createLocalPlannerFromFile(string path);
    bool                    createCSpace();
    bool                    createCSpaceFromFile(string xml_doc);
    bool                    tryToSolve();
    bool                    setCurrentControls(vector<KthReal> &val, int offset=0);
		WorkSpace*		          wSpace();
    void                    setHomeConf(Robot* rob, HASH_S_K* param);
		void			              setStartConf(Robot* rob, HASH_S_K* param);
    void			              setGoalConf(Robot* rob, HASH_S_K* param);
    void                    setLocalPlanner(LocalPlanner* loc){if(_locPlanner==NULL)_locPlanner = loc;}
    void                    setPlanner(Planner* plan){if(_planner==NULL)_planner = plan;}
    inline vector<Conf*>*		startConf(){return &_qStart;}
    inline vector<Conf*>*		GoalConf(){return &_qGoal;}
    inline LocalPlanner*    getLocalPlanner(){return _locPlanner;}
    inline Planner*         getPlanner(){return _planner;}
    inline SampleSet*       getSampleSet(){return _cspace;}
    inline Sampler*         getSampler(){return _sampler;}
    inline void             setSampler(Sampler* smp){_sampler = smp;}
    inline int              getDimension(){return _wspace->getDimension();}
    inline vector<KthReal>& getCurrentControls(){return _currentControls;}
    inline string           getFilePath(){return _filePath;}
		bool                    inheritSolution();
    bool                    setupFromFile(string xml_doc);

    //! This method saves the information of the problem's planner . 
    //! This method checks if the file_path file exists or not. In 
    //! case the file doesn't exist, the method copies the current 
    //! problem file and adds the planner's attributes.
    bool                    saveToFile(string file_path = "");
  
  private:
    const static KthReal    _toRad;
    WorkSpace*              _wspace;
    vector<Conf*>           _qStart;
    vector<Conf*>           _qGoal;
    CONFIGTYPE              _problemType;
    SampleSet*              _cspace;
    Sampler*                _sampler;
    Planner*                _planner;
    LocalPlanner*           _locPlanner;
    vector<KthReal>         _currentControls;
    string                  _filePath;
	};

    /** @}   end of Doxygen module "libProblem" */
}

#endif  //_PROBLEM_H

