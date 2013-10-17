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
 
 

#include "wmat.h"
#include <sstream>

using namespace std;

namespace Kautham {


/** \addtogroup libSampling
 *  @{
 */

	WMat::WMat(int dim, int level) {
		int i;
		int j;
		int P;
		d=dim;
		m=level;
		w = new long *[dim];

		for( i=0; i<dim; i++ ) {
			w[i] = new long [level];
		}

		P = ( level*dim ) - 1;
		for( j=0; j<level; j++ ) {
			for( i=dim-1; i>=0; i-- ) {
				w[i][j] = 0x01<<P--;
			}
		}
	}

	WMat::~WMat() {
		for(int ix=0; ix < d; ix++) {
			delete w[ix];
		}
		delete[] w;
	}

	std::string WMat::printMatrix() {
		int i;
		int j;

		stringstream sal;

		sal << "Matrix W:\n";
		for(i=0; i<d; i++) {
			sal << "[";
			for(j=0; j<m; j++) {
				sal << "\t" ;
				sal << (int)w[i][j];
			}
			sal << "]\n";
		}
		return sal.str();
	}

    /** @}   end of Doxygen module "libSampling" */
}

