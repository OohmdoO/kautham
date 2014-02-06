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
 

#include <Inventor/nodes/SoSeparator.h>
#include "gui.h"
#include "aboutwidget.h"
#include "dofwidget.h"
#include "sampleswidget.h"
#include "plannerwidget.h"
#include "controlwidget.h"
#include "constrainedcontrolwidget.h"
#include "invkinwidget.h"
#include <QtGui>
#include <libsampling/sampling.h>
#include "planners_toolbar.h"
#include <Inventor/nodes/SoCamera.h>
#include <libpugixml/pugixml.hpp>

#if defined(KAUTHAM_USE_GUIBRO)
#include <libguibro/bronchowidget.h>
#endif

using namespace pugi;

namespace Kautham {
/** \addtogroup libGUI
 *  @{
 */

	GUI::GUI(QWidget *p) {
		setupUi(this);
		problemTree->setEnabled(true);
    QString f(":/kautham.html");
    if (QFile::exists(f)){
      QFile file(f);
      if (file.open(QFile::ReadOnly)){
        QByteArray data = file.readAll();
        QTextCodec *codec = Qt::codecForHtml(data);
        QString str = codec->toUnicode(data);
        textBrowser->setHtml(str);
      }
    }
    qout = new StreamLog(std::cout, textEdit);
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(help()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(outputWindow, SIGNAL(dockLocationChanged (Qt::DockWidgetArea)), this, SLOT(changeDockAreaForOutput(Qt::DockWidgetArea)));
    boolPlanVis = false;
	planToolBar = NULL;
    restart();
	}
  
  void GUI::about(){
    AboutWidget tmp(this);
    tmp.setModal(true);
    tmp.setVisible(true);
    tmp.exec();
  }

  void GUI::changeDockAreaForOutput(Qt::DockWidgetArea area){
    if(area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
      outputWindow->setFeatures(QDockWidget::DockWidgetMovable |    
                                QDockWidget::DockWidgetFloatable);
    else
      outputWindow->setFeatures(QDockWidget::DockWidgetMovable |    
                                QDockWidget::DockWidgetFloatable |
                                QDockWidget::DockWidgetVerticalTitleBar);

  }

  void GUI::help(){
    try{
      QDir currDir;
      std::cout << "Trying to open " << currDir.absolutePath().toUtf8().constData() << "/Kautham2.chm\n";
#ifdef _WIN32
      system("hh.exe Kautham2.chm");
#else
      system("kchmviewer Kautham2.chm");
#endif
    }catch(...){}
  }

  void GUI::changeActiveBackground(){
    try{
      QColor color = QColorDialog::getColor(Qt::black, this);
      if (color.isValid()) {
        SoQtExaminerViewer* tmp = ((Viewer)viewers.at(viewsTab->currentIndex()-1)).window;
        tmp->setBackgroundColor(SbColor(color.redF(),color.greenF(), color.blueF()));
      }
    }catch(...){
    }
  }

	void GUI::clearText(){
		textEdit->clear();
	}

	void GUI::setText(string s){
		if(s.size()!=0){
			textEdit->append(QString(s.c_str()));
		}
	}

  bool GUI::setSampleWidget(SampleSet* samples, Sampler* sampler, Problem* prob){
    SamplesWidget* tmpSam = new SamplesWidget(samples, sampler, prob);
    propertiesTab->addTab(tmpSam, "Samplers");
    connect(tmpSam, SIGNAL(sendText(string)), this, SLOT(setText(string)) );
    return true;
  }

  bool GUI::addControlWidget( Robot* rob, Problem* prob, int offset ){
    if( rob != NULL){
      ControlWidget* tmpControl = new ControlWidget( rob, prob, offset );
      propertiesTab->addTab(tmpControl, "UsrCtrl-" + QString((rob->getName()).c_str()));
	  //JAN
		  indexControlsTab = propertiesTab->indexOf(tmpControl);
      return true;
    }else{
      ControlWidget* tmpControl = new ControlWidget( NULL, NULL, 0 );
      propertiesTab->addTab(tmpControl, "ControlTest");
      return true;
    }
    return false;
  }

  ControlWidget* GUI::getControlWidget()
  {
		return (ControlWidget*)propertiesTab->widget(indexControlsTab);
  }

  PlannerWidget* GUI::getPlannerWidget()
  {
        return (PlannerWidget*)propertiesTab->widget(indexPlannerTab);
  }



  //widged used for virtual bronchsocopy application
  bool GUI::addExternalWidget1( Robot* rob, Problem* prob, int offset, GUI* gui ){
#if defined(KAUTHAM_USE_GUIBRO)
      if(prob->getPlanner()->getIDName() == "GUIBRO Grid Planner")
      {
            if( rob != NULL){
                GUIBRO::bronchoWidget* tmpControl = new GUIBRO::bronchoWidget( rob, prob, offset, gui  );
                propertiesTab->addTab(tmpControl, "bronchoCtrl-" + QString((rob->getName()).c_str()));
                return true;
            }else{
            GUIBRO::bronchoWidget* tmpControl = new GUIBRO::bronchoWidget( NULL, NULL, 0 , NULL);
            propertiesTab->addTab(tmpControl, "bronchoControlTest");
            return true;
        }
      }
      return false;
#endif
    return false;
  }

  //widged not used
  bool GUI::addExternalWidget2( Robot* rob, Problem* prob, int offset, GUI* gui ){
      return false;
  }

  //widged not used
  bool GUI::addExternalWidget3( Robot* rob, Problem* prob, int offset, GUI* gui ){
      return false;
  }




  bool GUI::addConstrainedControlWidget( Robot* rob, Problem* prob, int offset ){
    if( rob != NULL){
      ConstrainedControlWidget* tmpControl = new ConstrainedControlWidget( rob, prob, offset );
      propertiesTab->addTab(tmpControl, "ConstrainedCtrl-" + QString((rob->getName()).c_str()));
      return true;
    }else{
      ConstrainedControlWidget* tmpControl = new ConstrainedControlWidget( NULL, NULL, 0 );
      propertiesTab->addTab(tmpControl, "ConstrainedControlTest");
      return true;
    }
    return false;
  }

  bool GUI::addDOFWidget(Robot* rob ){
    if( rob != NULL){
      DOFWidget* tmpDOF = new DOFWidget( rob );
      propertiesTab->addTab(tmpDOF, "DOF-" +QString((rob->getName()).c_str()));
      connect(tmpDOF, SIGNAL(sendText(string)), this, SLOT(setText(string)));
      return true;
    }else{
      DOFWidget* tmpDOF = new DOFWidget( NULL );
      propertiesTab->addTab(tmpDOF, "DOFTest");
      connect(tmpDOF, SIGNAL(sendText(string)), this, SLOT(setText(string)));
      return true;
    }
    return false;
  }

  bool GUI::addInverseKinematic(InverseKinematic* ikine){
    InvKinWidget* tmpIkine = new InvKinWidget(ikine);
    propertiesTab->addTab(tmpIkine,"IKine");
    connect(tmpIkine, SIGNAL(sendText(string)), this, SLOT(setText(string)));
    return true;
  }


  bool GUI::addWidgetToTab(QWidget* widget, QString name){
    if( widget != NULL){
      propertiesTab->addTab(widget, name);
      return true;
    }
    return false;
  }

  bool GUI::createPlannerToolBar(string loc, string plan, QObject* receiver, const char* member){
    if(loc != "" && plan != ""){
      planToolBar = new PlannerToolBar(this, loc ,plan, receiver, member);;
      planToolBar->setObjectName(QString::fromUtf8("PlanToolBar"));
      planToolBar->setOrientation(Qt::Horizontal);
      addToolBar(Qt::TopToolBarArea, planToolBar);
      boolPlanVis = true;
      planToolBar->setVisible(false);
      return true;
    }else
      return false;
  }

  void GUI::showPlannerToolBar(){
    if(planToolBar != NULL)
      planToolBar->setVisible(boolPlanVis);

    boolPlanVis = !boolPlanVis;
  }


  bool GUI::addPlanner(Planner *plan, SampleSet* samp, GUI* gui){
    if( plan != NULL){
      PlannerWidget* tmpPlan = new PlannerWidget( plan, samp, plan->hasCameraMovements(), gui );
      propertiesTab->addTab(tmpPlan, QString((plan->getGuiName()).c_str()));
      //JAN
        indexPlannerTab = propertiesTab->indexOf(tmpPlan);
      connect(tmpPlan, SIGNAL(sendText(string)), this, SLOT(setText(string)) );
	  if(plan->getIvCspaceScene() != NULL)
	  {
        addViewerTab("CSpace", SPACE, plan->getIvCspaceScene());
	  }
      return true;
    }else{
      PlannerWidget* tmpPlan = new PlannerWidget( NULL, NULL, gui );
      propertiesTab->addTab(tmpPlan, "Planner");
      //JAN
        indexPlannerTab = propertiesTab->indexOf(tmpPlan);
      return true;
    }
    return false;
  }

	bool GUI::addViewerTab(string title, VIEWERTYPE typ, SoSeparator *root){
		viewsTab->setEnabled(true);
    viewsTab->setCurrentIndex(0);
		if(root!=NULL && title.size()!=0 ){
			Viewer v;
			v.tab = new QWidget(viewsTab);
			v.root= root;
			v.title =title;
			v.type=typ;
			v.window= new SoQtExaminerViewer(v.tab);
			viewsTab->addTab(v.tab,QString(title.c_str()));
			v.window->setViewing(FALSE);
			v.window->setSceneGraph(root);
			v.window->setBackgroundColor(SbColor(0.0f,0.0f,0.0f));
			v.window->setPopupMenuEnabled(FALSE);
			v.window->show();
			viewers.push_back(v);
			viewsTab->setCurrentIndex(viewsTab->count()-1);
			return true;
		}
		return false;
	}

    //removes the viewer tab. The Introduction tab viewer cannot be eliminated.
	void GUI::removeViewerTab(string title){
		vector<Viewer>::iterator it_v;
      if( title.compare("Introduction") != 0 )
        for(it_v = viewers.begin(); it_v != viewers.end(); it_v++)
				if((*it_v).title == title){
					viewsTab->removeTab(viewsTab->indexOf( it_v->tab ));
					delete it_v->window;
					it_v->root->unref();
					viewers.erase(it_v);
					break;
				}
	}

  std::string GUI::getActiveViewTitle(){
    return viewsTab->tabText(viewsTab->currentIndex()).toUtf8().constData();
  }


  void GUI::removePropTab(string title){
    if( title.compare("Problem") == -1 )
      for(int i =0; i < propertiesTab->count(); i++)
        if(propertiesTab->tabText(i).compare(QString(title.c_str())) == 0){
          QWidget* tmp = propertiesTab->widget( i );
          delete tmp;
          propertiesTab->removeTab( i );
          break;
        }
  }

	/*!	This function return the root scene SoSeparator that show in tab which this title.
	*/
	SoSeparator* GUI::getRootTab(string title){
		if(title.size()!=0){
			for(unsigned int i=0; i < viewers.size();i++){
				if(viewers[i].title == title)
					return viewers[i].root ;
			}
		}
		return NULL;
	}

  const mt::Transform GUI::getActiveCameraTransfom(){
//    if( typeid(*(viewsTab->currentWidget())) == typeid(SoQtExaminerViewer)){
    try{
      SoQtExaminerViewer* tmp = ((Viewer)viewers.at(viewsTab->currentIndex()-1)).window;
      SoCamera* tmpCam = tmp->getCamera();
      SbVec3f pos, axis;
      mt::Unit3 mtaxis;
      mt::Point3 mtpos;
      float   angle;

      //  Get Transformation matrix for the camera

      tmpCam->orientation.getValue(axis, angle);
      pos = tmpCam->position.getValue();
      pos.getValue(mtpos.at(0), mtpos.at(1), mtpos.at(2));
      axis.getValue(mtaxis.at(0), mtaxis.at(1), mtaxis.at(2));

      const mt::Rotation rot(mtaxis, angle);

      const mt::Transform ci2w(rot, mtpos);
      return ci2w;
    }catch(...){
      const mt::Transform ci2w;
      return ci2w;
    }
  }

  bool GUI::setActiveCameraRotation(float qx, float qy, float qz, float qw){
    try{
      SoQtExaminerViewer* tmp = ((Viewer)viewers.at(viewsTab->currentIndex()-1)).window;
      SoCamera* tmpCam = tmp->getCamera();
      tmpCam->orientation.setValue(qx, qy, qz, qw);
      tmp->viewAll();
      tmp->setFeedbackVisibility(true) ;
      return true;
    }catch(...){
      return false;
    }
  }

  bool GUI::setActiveCameraPosition(float x, float y, float z ){
    try{
      SoQtViewer* tmp = ((Viewer)viewers.at(viewsTab->currentIndex()-1)).window;
      SoCamera* tmpCam = tmp->getCamera();
      tmpCam->position.setValue(x, y, z);
      tmp->viewAll();
      return true;
    }catch(...){
      return false;
    }
  }

  bool GUI::setActiveCameraPointAt(float x, float y, float z ){
    try{
      SoQtViewer* tmp = ((Viewer)viewers.at(viewsTab->currentIndex()-1)).window;
      SoCamera* tmpCam = tmp->getCamera();
      SbVec3f pos;
      pos.setValue(x, y, z);
      tmpCam->pointAt(pos);
      tmp->viewAll();
      return true;
    }catch(...){
      return false;
    }
  }

  bool GUI::setActiveCameraTransform(mt::Transform tra){
    try{
      SoQtViewer* tmp = ((Viewer)viewers.at(viewsTab->currentIndex()-1)).window;
      SoCamera* tmpCam = tmp->getCamera();
      SbVec3f pos, axis;

      mt::Unit3 mtaxis;
      float   angle;

      const mt::Point3 mtpos = tra.getTranslation();
      const mt::Rotation rot = tra.getRotation();

      rot.getAxisAngle(mtaxis, angle);

      axis.setValue(mtaxis.at(0), mtaxis.at(1), mtaxis.at(2));

      tmpCam->position.setValue(mtpos.at(0), mtpos.at(1), mtpos.at(2));
      tmpCam->orientation.setValue(axis, angle);
      //tmp->viewAll();
      return true;
    }catch(...){
      return false;
    }
  }

  SoQtExaminerViewer* GUI::getViewerTab(string title){
    if(title.size()!=0){
      for(unsigned int i=0; i < viewers.size();i++){
        if(viewers[i].title == title)
          return &(*(viewers.at(i).window) );
      }
    }
    return NULL;
  }


	bool GUI::addSeparator(WHERETYPE typ){
		switch(typ){
				case TOOLBAR:
					toolBar->addSeparator();
					break;
				case FILEMENU:
					menuFile->addSeparator();
					break;
				case ACTIONMENU:
					menuActions->addSeparator();
					break;
				case FILETOOL:
					menuFile->addSeparator();
					toolBar->addSeparator();
					break;
				case ACTIONTOOL:
					menuActions->addSeparator();
					toolBar->addSeparator();
					break;
				default:
					return false;
		}
		return true;
	}
	/*! This function add an action where do you like in GUI and asociate with the QObject
			and a Slot pass throw parameters
	*/
	bool GUI::setAction(WHERETYPE typ, string name, string shortcut, string iconame,
					QObject *receiver, const char *member){
		QAction *ac;

		if(iconame.size()!=0 && name.size()!=0 ){
			ac = new QAction(QIcon(iconame.c_str()), tr(name.c_str()), this);
			if(shortcut.size()!=0) ac->setShortcut(tr(shortcut.c_str()));
			connect(ac, SIGNAL(triggered()), receiver, member);

			switch(typ){
				case TOOLBAR:
					toolBar->addAction(ac);
					break;
				case FILEMENU:
					menuFile->addAction(ac);
					break;
				case ACTIONMENU:
					menuActions->addAction(ac);
					break;
				case FILETOOL:
					menuFile->addAction(ac);
					toolBar->addAction(ac);
					break;
				case ACTIONTOOL:
					menuActions->addAction(ac);
					toolBar->addAction(ac);
					break;
				default:
					return false;
			}
			return true;
		}
		return false;

	}

  bool GUI::setToogleAction(WHERETYPE typ, string name, string shortcut, string iconame,
          QObject *receiver, const char *member){
    QAction *ac;

    if(iconame.size()!=0 && name.size()!=0 ){
      ac = new QAction(QIcon(iconame.c_str()), tr(name.c_str()), this);
      ac->setCheckable(true);
      if(shortcut.size()!=0)
        ac->setShortcut(tr(shortcut.c_str()));

      connect(ac, SIGNAL(changed()), receiver, member);

      switch(typ){
        case TOOLBAR:
          toolBar->addAction(ac);
          break;
        case FILEMENU:
          menuFile->addAction(ac);
          break;
        case ACTIONMENU:
          menuActions->addAction(ac);
          break;
        case FILETOOL:
          menuFile->addAction(ac);
          toolBar->addAction(ac);
          break;
        case ACTIONTOOL:
          menuActions->addAction(ac);
          toolBar->addAction(ac);
          break;
        default:
          return false;
      }
      return true;
    }
    return false;

  }

    bool GUI::restart(){
        //do not put the index i inside the loop since the vector keeps decreasing after each remove
        for(int i=0; i<viewers.size() ;i++)
           removeViewerTab(viewers[0].title);

        //the same
    for(int i=viewsTab->count(); i>1 ;i--)
      viewsTab->removeTab(1);//tab 0 is never removed

    //the same
    for(int i=propertiesTab->count();i>0;i--)
      propertiesTab->removeTab(1);//tab 0 is never removed

    viewers.clear();
    problemTree->clear();
		textEdit->clear();
    stringstream tmp;
    tmp << "Kautham ";
    tmp << MAJOR_VERSION;
    tmp << ".";
    tmp << MINOR_VERSION;
    tmp << " - Institute of Industrial and Control Engineering";
    tmp << "- Technical University of Catalonia";
    setWindowTitle( tmp.str().c_str() );
		return true;
	}

	bool GUI::addToProblemTree(string problemPath){
    xml_document doc;
    xml_parse_result result = doc.load_file(problemPath.c_str());
    if( !result ) return false;

		problemTree->setEnabled(true);
		QTreeWidgetItem *itemTree;
		QTreeWidgetItem *subItemTree;
		QTreeWidgetItem *subSubItemTree;
		QStringList::const_iterator iterator;
		QStringList labelList= QString("X Y Z WX WY WZ TH").split(" ");

    xml_node::iterator it;
    xml_node tmpNode = doc.child("Problem");
    string names="";
		for( it = tmpNode.begin(); it != tmpNode.end(); ++it ){
      names = it->name();
      if( names == "Robot" ){
			  itemTree = new QTreeWidgetItem(problemTree);
			  itemTree->setText(0, "Robot");
			  itemTree->setText(1, it->attribute("robot").value() );

			  subItemTree = new QTreeWidgetItem(itemTree);
			  subItemTree->setText(0, "scale");
			  subItemTree->setText(1, it->attribute("scale").value() );

			  subItemTree = new QTreeWidgetItem(itemTree);
			  subItemTree->setText(0, "Home Position");
			  subItemTree->setText(1, QString());

			  iterator = labelList.constBegin();
        string tmp;
			  for(int i=0; i<7; i++){
				  subSubItemTree = new QTreeWidgetItem(subItemTree);
          tmp = (*iterator++).toUtf8().constData();
          subSubItemTree->setText( 0, tmp.c_str() );
				  subSubItemTree->setText( 1, it->child("Home").attribute(tmp.c_str()).value() ); 
			  }

        // Limits
        subItemTree = new QTreeWidgetItem(itemTree);
			  subItemTree->setText(0, "Limits");
			  subItemTree->setText(1, QString());

        xml_node::iterator itLim;
        //xml_node lim = it->child("Limits");
			  for( itLim = it->begin(); itLim != it->end(); ++itLim){
          names = itLim->name();
          if( names != "Limits" ) continue;
				  subSubItemTree = new QTreeWidgetItem(subItemTree);
          subSubItemTree->setText( 0, itLim->attribute("name").value() );
          tmp = "[";
          tmp.append(itLim->attribute("min").value() );
          tmp.append(", ");
          tmp.append( itLim->attribute("max").value() );
          tmp.append("]");
				  subSubItemTree->setText( 1, tmp.c_str() ); 
			  }

        //InvKinematic
        if( it->child("InvKinematic") ){
          subItemTree = new QTreeWidgetItem(itemTree);
			    subItemTree->setText(0, "InvKinematic");
          subItemTree->setText(1, it->child("InvKinematic").attribute("name").value());
        }

      }//closing if( names == "Robot" )

      if( names == "Scene" ){
			  itemTree = new QTreeWidgetItem(problemTree);
			  itemTree->setText(0, "Obstacle");
			  itemTree->setText(1, it->attribute("scene").value() );

			  subItemTree = new QTreeWidgetItem(itemTree);
			  subItemTree->setText(0, "Scale");
			  subItemTree->setText(1, it->attribute("scale").value() );

			  subItemTree = new QTreeWidgetItem(itemTree);
			  subItemTree->setText(0, "Position");
			  subItemTree->setText(1, QString());
        
        iterator = labelList.constBegin();
        string tmp;
			  for(int i=0; i<7; i++){
				  subSubItemTree = new QTreeWidgetItem(subItemTree);
          tmp = (*iterator++).toUtf8().constData();
          subSubItemTree->setText( 0, tmp.c_str() );
				  subSubItemTree->setText( 1, it->child("Location").attribute( tmp.c_str() ).value() ); 
			  }
		  }
    }
    problemTree->expandAll();
		return true;
	}

    /** @}   end of Doxygen module "libGUI" */
}

