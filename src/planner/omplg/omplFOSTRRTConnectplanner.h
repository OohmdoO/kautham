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

/* Author: Nestor Garcia Hidalgo */


#ifndef _OMPLFOSTRRTCONNECTPLANNER_H
#define _OMPLFOSTRRTCONNECTPLANNER_H


#include "omplplanner.h"


/** \addtogroup Planner
 * @{
*/
namespace Kautham {
    namespace omplplanner {
        class omplFOSTRRTConnectPlanner:public omplPlanner {
        public:
            omplFOSTRRTConnectPlanner(SPACETYPE stype, Sample *init, Sample *goal,
                                   SampleSet *samples, WorkSpace *ws, og::SimpleSetup *ssptr);

            virtual ~omplFOSTRRTConnectPlanner();

            virtual bool setParameters();

            virtual bool trySolve();

            bool setSynergyTree(string filename);

        private:
           ompl::base::OptimizationObjectivePtr opt_;
        };
    }
}
/** @}   end of Doxygen module "Planner */

#endif  // _OMPLFOSTRRTCONNECTPLANNER_H
