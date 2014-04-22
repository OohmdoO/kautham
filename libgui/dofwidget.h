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
 
 
 
#if !defined(_DOFWIDGET_H)
#define _DOFWIDGET_H

#include <QtGui>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <vector>
#include <string>
#include <libproblem/robot.h>
#include <libproblem/problem.h>
#include <libkthutil/kauthamdefs.h>


using namespace std;


namespace Kautham {
/** \addtogroup libGUI
 *  @{
 */

	class DOFWidget:public QWidget{
		Q_OBJECT

	private slots:
		void              sliderChanged(int val);
	public:
		DOFWidget( Robot* rob );
		~DOFWidget();
		inline vector<KthReal>   *getValues(){return &values;}
        void setValues(vector<KthReal> &val);
	private:
		vector<QSlider*>  sliders;
		vector<QLabel*>   labels;
		QGridLayout       *gridLayout;
		QVBoxLayout       *vboxLayout;
        QStringList       names;
		vector<KthReal>   values;
        vector<KthReal>   low;
        vector<KthReal>   high;
	};


    /** @}   end of Doxygen module "libGUI" */
}

#endif  //_DOFWIDGET_H


