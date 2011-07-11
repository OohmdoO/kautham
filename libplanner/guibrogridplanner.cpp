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
*     Copyright (C) 2007 - 2009 by Alexander P�rez and Jan Rosell          *
*            alexander.perez@upc.edu and jan.rosell@upc.edu                *
*                                                                          *
*             This is a motion planning tool to be used into               *
*             academic environment and it's provided without               *
*                     any warranty by the authors.                         *
*                                                                          *
*          Alexander P�rez is also with the Escuela Colombiana             *
*          de Ingenier�a "Julio Garavito" placed in Bogot� D.C.            *
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
 
 

#include <libproblem/ivworkspace.h>
#include <libproblem/workspace.h>
#include <libsampling/sampling.h>
#include <libsampling/robconf.h>
#include "localplanner.h"
#include "guibrogridplanner.h"
#include <libproblem/ConsBronchoscopyKin.h>

using namespace libSampling;

namespace libPlanner {
   namespace GUIBROGRID{
	//! Constructor
    GUIBROgridPlanner::GUIBROgridPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws, LocalPlanner *lcPlan, KthReal ssize):
              Planner(stype, init, goal, samples, sampler, ws, lcPlan, ssize)
	{
		//set intial values
	    _gen = new LCPRNG();
      _drawnLink = 0; //the path of base is defaulted
	  
	  //define look-ahead points
		_counterFirstPoint = _wkSpace->obstaclesCount();
		KthReal pos[3] = {0.0,0.0,0.0};
		KthReal ori[4] = {1.0,0.0,0.0,0.0};
		KthReal scale = 1;
		bool obscollisions = false;
		Obstacle *obs; 
		//add points to look-ahead points as obstacles
		_maxLookAtPoints = 121;
		for(int i=0;i<_maxLookAtPoints;i++)
		{
			obs = new Obstacle("",pos,ori,scale,IVPQP,obscollisions);
			_wkSpace->addObstacle(obs);
		}


	  _showObstacle = new int[_wkSpace->obstaclesCount()];
	  for(int i=0; i<_wkSpace->obstaclesCount();i++) 
		  _showObstacle[i]=-1;

	  //interface to show obstacles (not considering look-ahead points
	  char str[50];
	  for(int i=0; i<_counterFirstPoint;i++) 
	  {
		  sprintf(str,"Show Obstacle %d (0/1)",i);
		  addParameter(str, 1);//shown
	  }
	  //interface to show look-ahead points
	  addParameter("Show Points (0/1)",0);

	  _stepsAdvance = 10.0;
	  addParameter("Steps Advance",_stepsAdvance);

	  _onlyNF1=1;
	  addParameter("Only NF1(0/1)",_onlyNF1);

		//set intial values from parent class data
		_speedFactor = 1;
		_solved = false;
		setStepSize(ssize);//also changes stpssize of localplanner
	  
		_guiName = _idName = "GUIBRO Grid Planner";
		addParameter("Step Size", ssize);
		addParameter("Speed Factor", _speedFactor);

        addParameter("Drawn Path Link",_drawnLink);

		_maxNumSamples=10000;
		//removeParameter("Max. Samples");
		//removeParameter("Neigh Thresshold");
		//removeParameter("Drawn Path Link");
		//removeParameter("Max. Neighs");

		_wkSpace->getRobot(0)->setLinkPathDrawn(_drawnLink);


		grid = new workspacegridPlanner(stype, init, goal, samples, sampler, ws, lcPlan, ssize);

		
    }

	//! void destructor
	GUIBROgridPlanner::~GUIBROgridPlanner(){
			
	}


//! Collision-check based on the spheres centered at each link origin.
	//!Bronchoscope links are defined as a cylinder of heigth 1mm and two spheres of radius 1mm centered
	//!at the bottom and top of the cylinder. The bottom sphere of one link coincides with the
	//!top sphere of the previous link.
	//!For collision-check purposes the bottom sphere of each link is considered.
	bool GUIBROgridPlanner::collisionCheck(int *distcost, int *NF1cost, KthReal radius)
	{
		//origin of the regular grid in world coordinates
		KthReal *O = grid->getOrigin();
		//size of the voxels in the grid
		KthReal *voxelSize = grid->getVoxelSize();
		KthReal maxSize = voxelSize[0];
		if(voxelSize[1]>maxSize) maxSize = voxelSize[1];
		if(voxelSize[2]>maxSize) maxSize = voxelSize[2];
		//number of cells per axis
		int *steps = grid->getDiscretization();
		//number of links of the bronchoscope
		int nlinks = _wkSpace->getRobot(0)->getNumLinks();
		//transform of the origin of each link
		mt::Transform linkTransf;
		//position of the origin of each link
		mt::Point3 pos;
		//indices and label of the cell closest to the origin of the link
		unsigned int i,j,k,label;
		//distance of the occupied cell to the walls of the bronchi 
		int dist;
		//flag indicating if the bronchoscope is in collision or not
		bool collision=false;
		//flag indicating if the shere centered at the occupied cell is free or not
		bool freesphere;
		//threshold distance, measured in cells,
		//int threshold = (int)(sqrt(3.0)*radius/maxSize);//sqrt(3) is added to consider the distance to the vertex of the voxel
		int threshold = 1;
		//reset the cost
		*distcost=0;
		for(int n=0;n<nlinks;n++)
		{
			linkTransf = _wkSpace->getRobot(0)->getLinkTransform(n);
			pos = linkTransf.getTranslation();
			i = (pos[0]-O[0]+voxelSize[0]/2)/voxelSize[0];
			j = (pos[1]-O[1]+voxelSize[1]/2)/voxelSize[1];
			k = (pos[2]-O[2]+voxelSize[2]/2)/voxelSize[2];
			label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;
			dist = -10*threshold; //value that will take thos spheres out of bounds
			freesphere=grid->getDistance(label, &dist);
			if(freesphere && dist<=threshold) freesphere = false;
			*distcost += dist;
			if(freesphere==false) collision = true;

			//for the tip of the bronchoscope: 
			if(n==nlinks-1)
			{
				//Evaluate the sphere of the tip
				mt::Transform totip;	
				totip.setRotation(mt::Rotation(0.0,0.0,0.0,1));
				totip.setTranslation(mt::Vector3(_wkSpace->getRobot(0)->getLink(n)->getA(),0,0) );
				mt::Transform tipTransf = linkTransf*totip;
				pos = tipTransf.getTranslation();
				i = (pos[0]-O[0]+voxelSize[0]/2)/voxelSize[0];
				j = (pos[1]-O[1]+voxelSize[1]/2)/voxelSize[1];
				k = (pos[2]-O[2]+voxelSize[2]/2)/voxelSize[2];
				label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;
				dist = -10*threshold; //value that will take thos spheres out of bounds
				freesphere=grid->getDistance(label, &dist);

				//more constrained for the tip
				threshold = 2;

				if(freesphere && dist<=threshold) freesphere = false;
				*distcost += dist;
				if(freesphere==false) collision = true;

				//Evaluate the NF1 function
				grid->getNF1value(label, NF1cost);
			}
		}
		return collision;
	}


	
	bool GUIBROgridPlanner::findGraphVertex(Sample * s, gridVertex *v)
	{
		unsigned int label = findGridCell(s);
		if(grid->getGraphVertex(label, v)==false)
		{
			return false;
		}
		return true;
	}

	unsigned int GUIBROgridPlanner::findGridCell(Sample * s)
	{
		//origin of the regular grid in world coordinates
		KthReal *O = grid->getOrigin();
		
		//size of the voxels in the grid
		KthReal *voxelSize = grid->getVoxelSize();
		KthReal maxSize = voxelSize[0];
		if(voxelSize[1]>maxSize) maxSize = voxelSize[1];
		if(voxelSize[2]>maxSize) maxSize = voxelSize[2];
		//number of cells per axis
		int *steps = grid->getDiscretization();
		
		//transform of the origin of each link
		mt::Transform linkTransf;
		//position of the origin of each link
		mt::Point3 pos;
		//indices and label of the cell closest to the origin of the link
		unsigned int i,j,k,label;

		//move to s
		_wkSpace->moveTo(s);
		linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
		pos = linkTransf.getTranslation();
		i = (pos[0]-O[0]+voxelSize[0]/2)/voxelSize[0];
		j = (pos[1]-O[1]+voxelSize[1]/2)/voxelSize[1];
		k = (pos[2]-O[2]+voxelSize[2]/2)/voxelSize[2];
		label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;

		return label;
	}



	//! setParameters sets the parameters of the planner
    bool GUIBROgridPlanner::setParameters(){
      try{
        HASH_S_K::iterator it = _parameters.find("Step Size");
		if(it != _parameters.end())
			setStepSize(it->second);//also changes stpssize of localplanner
        else
          return false;

        it = _parameters.find("Speed Factor");
        if(it != _parameters.end())
          _speedFactor = it->second;
        else
          return false;


		it = _parameters.find("Steps Advance");
        if(it != _parameters.end())
          _stepsAdvance = it->second;
        else
          return false;

		
		//Show/Unshow obstacles
		char str[50];
		SoSeparator *seproot = ((IVWorkSpace*)_wkSpace)->getIvScene();
		for(int i=0; i<_counterFirstPoint;i++)
		{
			sprintf(str,"Show Obstacle %d (0/1)",i);
			it = _parameters.find(str);
			if(it != _parameters.end())
			{
				//if the obstacle should be removed
				if(it->second ==0)
				{
					_showObstacle[i] = it->second;
					sprintf(str,"obstacle%d",i);
					SoNode *sepgrid = seproot->getByName(str);
					//comprovacio
					//int c = seproot->findChild(sepgrid);
					seproot->removeChild(sepgrid);

     			}
				//if the obstacle should be added (because it has been previously removed)
				//(note that _showObstacle is initalized to -1 in order not to add again the
				//obstacle if it is already there.)
				else if(_showObstacle[i]==0)
				{
					_showObstacle[i] = it->second;
					SoSeparator* gridSep = (SoSeparator*)_wkSpace->getObstacle(i)->getModel();
					sprintf(str,"obstacle%d",i);
					gridSep->setName(str);
					seproot->addChild(gridSep);
					
					//comprovacio
					//SoNode *sepgrid = seproot->getByName(str);
					//int c = seproot->findChild(sepgrid);
					//int kk=0;
				}
			}
			else
				return false;
		}
		//end show obstacles

		
		//Show/Unshow lookat points
		sprintf(str,"Show Points (0/1)");
		it = _parameters.find(str);
		if(it != _parameters.end())
		{
			for(int i=_counterFirstPoint; i<_wkSpace->obstaclesCount();i++)
			{
				//if the points should be removed
				if(it->second ==0)
				{
					_showObstacle[i] = it->second;
					sprintf(str,"obstacle%d",i);
					SoNode *sepgrid = seproot->getByName(str);
					//comprovacio
					//int c = seproot->findChild(sepgrid);
					seproot->removeChild(sepgrid);

     			}
				//if the obstacle should be added (because it has been previously removed)
				//(note that _showObstacle is initalized to -1 in order not to add again the
				//obstacle if it is already there.)
				else if(_showObstacle[i]==0)
				{
					_showObstacle[i] = it->second;
					SoSeparator* gridSep = (SoSeparator*)_wkSpace->getObstacle(i)->getModel();
					sprintf(str,"obstacle%d",i);
					gridSep->setName(str);
					seproot->addChild(gridSep);
					
					//comprovacio
					//SoNode *sepgrid = seproot->getByName(str);
					//int c = seproot->findChild(sepgrid);
					//int kk=0;
				}
			}
		}
		else
			return false;		
		//end show lookat points
		
		it = _parameters.find("Only NF1(0/1)");
		if(it != _parameters.end()){
          _onlyNF1 = it->second;
		}else
          return false;
		
		
		it = _parameters.find("Drawn Path Link");
		if(it != _parameters.end()){
          _drawnLink = it->second;
          _wkSpace->getRobot(0)->setLinkPathDrawn(_drawnLink);
		}else
          return false;

      }catch(...){
        return false;
      }
      return true;
    }

	void GUIBROgridPlanner::printInfo(guibroSample *gs)
	{
		cout<<"id="<<gs->id<<endl;
		cout<<"u = ("<<gs->u[0]<<", "<<gs->u[1]<<", "<<gs->u[2]<<")"<<endl;
		Sample *s = gs->smpPtr;
		cout<<"sample "<<_samples->indexOf(s)<<endl;
		
		vector<KthReal>&  p = (s->getMappedConf())[0].getSE3().getPos();
		vector<KthReal>&  o = (s->getMappedConf())[0].getSE3().getOrient();
			
		cout<<"pos: "<<p[0]<<", "<<p[1]<<", "<<p[2]<<endl;
		cout<<"ori: "<<o[0]<<", "<<o[1]<<", "<<o[2]<<", "<<o[3]<<endl;
	}

	
  void GUIBROgridPlanner::moveAlongPath(unsigned int step){
    if(_solved){
      if( _simulationPath.size() >= 2 ){
        step = step % _simulationPath.size();
        _wkSpace->moveTo(_simulationPath[step]);

      }else
        std::cout << "The problem is wrong solved. The solution path has less than two elements." << std::endl;
	}
  }


	int GUIBROgridPlanner::advanceToBest(KthReal stepsahead, Sample *smp, FILE *fp)
	{
		vector<KthReal>  values;
		KthReal alpha, beta;
		

		//look 10 steps ahead
		int r = look(stepsahead, &alpha, &beta);
		if(r>0)
		{
			mt::Transform linkTransf;
			mt::Point3 pos,pos0;
			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos0 = linkTransf.getTranslation();

			//move one step ahead interpolating. The values alpha, beta shopuld be reached after stepahead steps	
			KthReal alpha0 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(0);
			KthReal beta0 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(1);

			KthReal a = alpha0+(alpha-alpha0)/stepsahead;
			KthReal b = beta0+(beta-beta0)/stepsahead;
			values.push_back(a);
			values.push_back(b);
			values.push_back(-1);
			_wkSpace->getRobot(0)->ConstrainedKinematics(values);
			
			//verify if the interpolation step towards the best configuration is collision-free
			int currentNF1cost;
			int currentdcost;
			bool colltest = collisionCheck(&currentdcost,&currentNF1cost);

			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos = linkTransf.getTranslation();
			if(fp!=NULL)
			{
				if(colltest) fprintf(fp,"Apply steps= %.2f a= %.2f b= %.2f to go from (%.2f %.2f %.2f) to (%.2f %.2f %.2f) Reached: NF1= %d Dist = %d col = TRUE\n",
					stepsahead,a,b, pos0[0],pos0[1],pos0[2],pos[0],pos[1],pos[2],currentNF1cost,currentdcost);
				else fprintf(fp,"Apply steps= %.2f a= %.2f b= %.2f to go from (%.2f %.2f %.2f) to (%.2f %.2f %.2f) Reached: NF1= %d Dist = %d col = FALSE\n",
					stepsahead,a,b, pos0[0],pos0[1],pos0[2],pos[0],pos[1],pos[2],currentNF1cost,currentdcost);
			}
			else{
				if(colltest){ 
					cout<<"Apply steps= "<<stepsahead<<" a= "<<a<<" b= "<<b;
					cout<<" to go from ("<<pos0[0]<<pos0[1]<<pos0[2]<<") to ("<<pos[0]<<pos[1]<<pos[2];
					cout<<") Reached: NF1= "<<currentNF1cost<<" Dist = "<<currentdcost<<" col = TRUE"<<endl;
				}
				else{ 
					cout<<"Apply steps= "<<stepsahead<<" a= "<<a<<" b= "<<b;
					cout<<" to go from ("<<pos0[0]<<pos0[1]<<pos0[2]<<") to ("<<pos[0]<<pos[1]<<pos[2];
					cout<<") Reached: NF1= "<<currentNF1cost<<" Dist = "<<currentdcost<<" col = FALSE"<<endl;
				}
			}

			if(colltest)
			{
				//interpolation step is in collision. Return to previous position
				values[2]=1;
				_wkSpace->getRobot(0)->ConstrainedKinematics(values);
				linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
				pos = linkTransf.getTranslation();
				if(fp!=NULL)
					fprintf(fp,"move back to (%.2f %.2f %.2f)\n",pos[0],pos[1],pos[2]);
				else cout<<"move back to ("<<pos[0]<<pos[1]<<pos[2]<<")"<<endl;
				
				r = -3;
				cout<<"Sorry, interpolation motion is not free. Cannot move..."<<endl;
			}

			if(smp!=NULL)
			{
				smp->setMappedConf(_wkSpace->getConfigMapping());
			}
		}
		else if(r==-1)
		{
			cout<<"Goal already reached!!, no best configuration to move..."<<endl;
		}
		else if(r==-2)
		{
			cout<<"Sorry, local minima. Cannot move to a better configuration..."<<endl;
		}
		else{
			cout<<"Sorry, no free configuration available. Cannot move to a better configuration..."<<endl;
		}
		return r;
	}

//looks a step ahead
int GUIBROgridPlanner::look(KthReal stepsahead, KthReal *bestAlpha, KthReal *bestBeta)
		{
			//store current position
			

			KthReal alpha0 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(0);
			KthReal beta0 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(1);
			vector<KthReal> alpha(_maxLookAtPoints);
			vector<KthReal> beta(_maxLookAtPoints);	

			//working in degrees
			KthReal DeltaAlpha;//to be determined as a function of beta
			KthReal maxAlpha = (180/M_PI)*((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxAlpha();
			KthReal a;
			KthReal a0 = maxAlpha*alpha0; //alpha goes from -1 to +1 that corresponds to the range [-maxAlpha,maxAlpha]
			KthReal alphaRange;

			KthReal DeltaBeta;// = 2;//fixed step
			//if(stepsahead<2.5) DeltaBeta = 5.0; 
			//else if(stepsahead<5.0) DeltaBeta = 4.0; 
			//else if(stepsahead<7.5) DeltaBeta = 3.0; 
			//else DeltaBeta = 2.0; 
			//DeltaBeta = 5.0-(stepsahead-2.5)/2.5;
			DeltaBeta = 10.0-stepsahead/2.5;
			if(DeltaBeta<2.0) DeltaBeta=2.0;
			
			KthReal maxBeta = (180/M_PI)*((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxBending();
			KthReal b;
			KthReal b0 = maxBeta*beta0; //beta goes from -1 to +1 that corresponds to the range [-maxBeta,maxBeta]

			//sweep 20 degrees in beta, centered around beta0
			int k=0;
			for(int i=-5;i<=5;i++)
			{
				b = b0 + i*DeltaBeta;
				if(b>maxBeta || b<-maxBeta) continue;
				//find the range of alpha for the current value of beta (b)
				//alphaRange = 20+fabs(2.0*b+180); 
				//alphaRange = (maxAlpha/180.0)*238.500000000000398-6.55535714285715976*fabs(b)+0.473214285714287364e-1*b*b;
				//if(alphaRange>maxAlpha)alphaRange=maxAlpha;
				//else if(alphaRange<15.0) alphaRange=15.0;


				alphaRange = maxAlpha*2;
				/*if(fabs(b)<10) alphaRange = maxAlpha;
				else if(fabs(b)<20) alphaRange = 0.95*maxAlpha;//0.9*maxAlpha;
				else if(fabs(b)<30) alphaRange = 0.9*maxAlpha;//0.8*maxAlpha;
				else if(fabs(b)<40) alphaRange = 0.8*maxAlpha;//0.5*maxAlpha;
				else if(fabs(b)<50) alphaRange = 0.6*maxAlpha;//0.3*maxAlpha;
				else if(fabs(b)<60) alphaRange = 0.4*maxAlpha;//0.2*maxAlpha;
				else if(fabs(b)<=maxBeta) alphaRange = 0.4*maxAlpha;//0.3*maxAlpha;
				else alphaRange = 15;//should not reach this else
				*/
				DeltaAlpha = alphaRange/10;
				//sweep alphaRange degrees around alpha0
				for(int j=-5;j<=5;j++)
				{
					a = a0 + j*DeltaAlpha;
					if(a>maxAlpha || a<-maxAlpha) continue;
					//store a look-at point (a,b), in the normalized form between -1 and 1
					alpha[k] = a/maxAlpha;
					beta[k] = b/maxBeta;
					k++;
				}
			}
			for(int i=k;i<_maxLookAtPoints;i++)
			{
					alpha[i] = alpha0;
					beta[i] = beta0;
			}


			vector<int> dcost(_maxLookAtPoints);
			vector<int> NF1cost(_maxLookAtPoints);
			vector<bool> col(_maxLookAtPoints);

			int freepoints = 0;
			int minNF1cost = 1000000;
			int maxNF1cost = -1000000;
			int mindcost = 1000000;
			int maxdcost = -1000000;
			int currentNF1cost;
			int currentdcost;
			int pointminNF1cost = -1;
			//current cost 
			bool colltest = collisionCheck(&currentdcost,&currentNF1cost);
			if(currentNF1cost==0)
			{
				//reached
				return -1;
			}
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				//alpha = alpha0 + (KthReal)_gen->d_rand()*0.2-0.1;
				//xi = xi0 + (KthReal)_gen->d_rand()*0.2-0.1;
				col[i] = testLookAtPoint(_counterFirstPoint+i, alpha[i], beta[i], stepsahead, &dcost[i], &NF1cost[i]);
				//restore alpha0, beta0
				//((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->setvalues(alpha0,0);
				//((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->setvalues(beta0,1);

				if(col[i]==false)
				{
					//find extreme values for NF1 cost
					if(NF1cost[i]>0){
						if(NF1cost[i]<minNF1cost){
							minNF1cost = NF1cost[i];
							pointminNF1cost = _counterFirstPoint+i;
						}
						else if(NF1cost[i]>maxNF1cost){
							maxNF1cost = NF1cost[i];
						}
					}
					//find extreme values for distance cost
					if(dcost[i]>0){
						if(dcost[i]<mindcost){
							mindcost = dcost[i];
						}
						else if(dcost[i]>maxdcost){
							maxdcost = dcost[i];
						}
					}
					//cout<<"Point "<<i<<"("<<alpha[i]<<","<<xi[i]<<") collision "<<col<<" cost = "<<cost<<endl; 
					freepoints++;
				}
			}

			//if no lookat point has a NF1 value, retunr 0
			KthReal color[3];
			if(pointminNF1cost==-1) {
				for(int i=0; i<_maxLookAtPoints;i++)
				{
					color[0]=0.8;
					color[1]=0.2;
					color[2]=0.2;
					_wkSpace->getObstacle(_counterFirstPoint+i)->getElement()->setColor(color);
				}
				//return to current position
				/*vector<KthReal>  values;
				values.push_back(alpha0);
				values.push_back(beta0);
				values.push_back(0);
				_wkSpace->getRobot(0)->ConstrainedKinematics(values)*/;
				return 0;
			}
			
			//else color the candidate points
			KthReal minr=100000;
			KthReal weightNF1 = 0.5;//0.75
			int localminima=1;
			int bestdcost;
			int bestNF1cost;
			int besti=-1;

			vector<int> indexi;
			vector<KthReal> rNF1(_maxLookAtPoints);
			vector<KthReal> rDist(_maxLookAtPoints);
			KthReal r;
			//store all those lookat points that improve the NF1 value
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				if(col[i]==false)
				{
					if(NF1cost[i]>0 && NF1cost[i]<currentNF1cost)
					{
						rNF1[i] = (KthReal)(NF1cost[i]-minNF1cost)/(maxNF1cost-minNF1cost);
						indexi.push_back(i);
						//if(NF1cost[i]<currentNF1cost) indexi.push_back(i);
					}
					else rNF1[i] = -1;
				}
				else rNF1[i] = -1;
			}
			//among those that imnprove the NF1 value chose the one that has a better 
			for(int i=0; i<_maxLookAtPoints;i++) rDist[i] = 1.0;
			for(int j=0; j<indexi.size();j++)
			{
				int i = indexi[j];
				rDist[i] = 1-((KthReal)(dcost[i]-mindcost)/(maxdcost-mindcost));
			}
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				if(rNF1[i]==-1) //collision or non-NF1 value
				{
					color[0]=0.8;
					color[1]=0.5;
					color[2]=0.5;
				}
				else
				{
					r = weightNF1*rNF1[i]+(1-weightNF1)*rDist[i];
					if(r<minr)
					{
						minr=r;
						*bestAlpha = alpha[i];
						*bestBeta = beta[i];
						bestdcost=dcost[i];
						bestNF1cost=NF1cost[i];
						localminima=0;
						besti = i;
					}
					color[0]=0.0;
					color[1]=(0.2+(1-r)*0.8);
					color[2]=0.8*r;
				}
				_wkSpace->getObstacle(_counterFirstPoint+i)->getElement()->setColor(color);
			}
			if(besti!=-1)
			{
				//Goal is white
				color[0]=1.0;
				color[1]=1.0;
				color[2]=1.0;
				_wkSpace->getObstacle(_counterFirstPoint+besti)->getElement()->setColor(color);
			}

			/*
			KthReal rNF1;
			KthReal rDist;
			KthReal r;
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				if(col[i]==false)
				{
					//color between green and blue for cells with NF1 value
					//more green those cells with lowest NF1 value (that is zero for the goal cell)
					//cells with a cost greater than the current are not considered
					if(NF1cost[i]>0){
						if(NF1cost[i]>currentNF1cost) r=1;
						else 
						{
							//rNF1 varies between 0 and 1. Lower values indicate that we are closer to the goal
							rNF1 = (KthReal)(NF1cost[i]-minNF1cost)/(maxNF1cost-minNF1cost);
							//rDist varies between 0 and 1. The distance cost equals the positive distance
							//to the walls. As defined, rDist is lower for regions far away from the walls.
							rDist = 1-((KthReal)(dcost[i]-mindcost)/(maxdcost-mindcost));
							//r combines rNF1 and rDist. Lower values indicate a good candidate.
							//good candidates will be green, bad candidates blue.
							r = weightNF1*rNF1+(1-weightNF1)*rDist;
							if(r<minr)
							{
								minr=r;
								*bestAlpha = alpha[i];
								*bestBeta = beta[i];
								bestdcost=dcost[i];
								bestNF1cost=NF1cost[i];
								localminima=0;
							}
						}
						//r=rNF1;
						//draw green the point with lower value of the NF1 function
						color[0]=0.0;
						color[1]=(0.2+(1-r)*0.8);
						color[2]=0.8*r;
					}
				}
				//collision point
				else
				{
					color[0]=0.8;
					color[1]=0.5;
					color[2]=0.5;
				}
				_wkSpace->getObstacle(_counterFirstPoint+i)->getElement()->setColor(color);
			}
			*/
			
			//return to current position
			/*vector<KthReal>  values;
			values.push_back(alpha0);
			values.push_back(beta0);
			values.push_back(0);
			_wkSpace->getRobot(0)->ConstrainedKinematics(values);*/


			if(localminima) return -2;
			else{
				//cout<<"currrent= "<<NF1cost[60]<<", "<<dcost[60]<<", "<<alpha[60]<<", "<<beta[60]<<endl;
				//cout<<"best    = "<<bestNF1cost<<", "<<bestdcost<<", "<<*bestAlpha<<", "<<*bestBeta<<endl;
				return freepoints;
			}
		}

//draws a point at the configuration stepsahead from the current one
bool GUIBROgridPlanner::testLookAtPoint(int numPoint, KthReal alpha, 
										KthReal xi, KthReal stepsahead, int *dcost, int *NF1cost)
{
			mt::Transform linkTransf;
			mt::Point3 pos;
			mt::Rotation rot;
			KthReal p[3];
			Vector3 zaxis;

			//store current configuration
			RobConf* rConf = _wkSpace->getRobot(0)->getCurrentPos(); 
			RobConf currentRobConf;
			currentRobConf.setSE3(rConf->getSE3());
			currentRobConf.setRn(rConf->getRn());

			//Advance a deltaz step
			vector<KthReal>  values;
			values.push_back(alpha);
			values.push_back(xi);
			values.push_back(-stepsahead);
			_wkSpace->getRobot(0)->ConstrainedKinematics(values);
			
			//move the broncoscope at the new position
			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			int nlinks = _wkSpace->getRobot(0)->getNumLinks();
			mt::Transform totip;	
			totip.setRotation(mt::Rotation(0.0,0.0,0.0,1));
			totip.setTranslation(mt::Vector3( _wkSpace->getRobot(0)->getLink(nlinks-1)->getA(),0,0) );
			mt::Transform tipTransf = linkTransf*totip;
			pos = tipTransf.getTranslation();

			//compute the cost at the current position
			bool colltest = collisionCheck(dcost,NF1cost);

			//draw the obstacle at the extrem of the broncoscope at the new position
			p[0]=pos[0];
			p[1]=pos[1];
			p[2]=pos[2];
			_wkSpace->getObstacle(numPoint)->getElement()->setPosition(p);


			//return to current position
			_wkSpace->getRobot(0)->Kinematics(currentRobConf);
			//restore alpha0, beta0
			((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->registerValues();

			return colltest;
		}
	
//! function to find a solution path
 /**/
bool GUIBROgridPlanner::trySolve()
		{	
			static gridVertex vinit=-1;
			static gridVertex vgoal=-1;
			gridVertex vi, vg;

			if(findGraphVertex(_goal,&vg)==false || findGraphVertex(_init,&vi)==false)
			{
				cout<<"ERROR: No graph vertex found for init and goal samples"<<endl;
				vi=-1;
				vg=-1;
				_solved=false;
				return _solved;
			}
			

			//if init or goal changed then recompute NF1
			if(vi != vinit || vg != vgoal)
			{
				vinit = vi;
				vgoal = vg;

				_simulationPath.clear();
				_cameraPath.clear();
				_path.clear();
				_solved = false;
				  
				//Comnpute NF1
				cout<<"START computing NF1 function"<<endl;
				
				grid->computeNF1(vg);
				cout<<"END computing NF1 function"<<endl;
			}

			if(_onlyNF1) 
			{
				_solved=false;
				return _solved;
			}
			//End Comnpute NF1/////////////////////////////////


			//Start finding a path
			Sample *smp = new Sample(_wkSpace->getDimension());
			
			mt::Transform T_Ry;
			mt::Transform T_tz;
			T_Ry.setRotation( mt::Rotation(mt::Vector3(0,1,0),-M_PI/2) );
			T_tz.setTranslation( mt::Vector3(0,0,-(_wkSpace->getRobot(0)->getLink(_wkSpace->getRobot(0)->getNumLinks()-1)->getA()+1.1)) );
			
			_wkSpace->moveTo(_init);
			//restore alpha,beta
			((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->registerValues();

			_simulationPath.push_back(_init);	
			addCameraMovement(_wkSpace->getRobot(0)->getLinkTransform(_wkSpace->getRobot(0)->getNumLinks()-1)*T_Ry*T_tz);

			FILE *fp;
			fp = fopen ("advanceToBest.txt","wt");
			if(fp==NULL) cout<<"Cannot open advanceToBest.txt for writting..."<<endl;
			
			KthReal steps = _stepsAdvance;
			int r;
			do{
				r=advanceToBest(steps,smp,fp);
				if(r>0)
				{				
					_path.push_back(smp);
					_samples->add(smp);
					_simulationPath.push_back(smp);
					addCameraMovement(_wkSpace->getRobot(0)->getLinkTransform(_wkSpace->getRobot(0)->getNumLinks()-1)*T_Ry*T_tz);
					smp = new Sample(_wkSpace->getDimension());
				}
				else steps /=2.0;
			}while(r!=-1 && steps>0.1);
			fclose(fp);

			int currentNF1cost;
			int currentdcost;
			bool colltest = collisionCheck(&currentdcost,&currentNF1cost);
			cout<<"Reached NF1 value is: "<<currentNF1cost<<endl;

			if(_simulationPath.size()>1) _solved = true;
			else _solved = false;

			return _solved;



/* THe following code tests the restoration of a configuration
			mt::Transform linkTransf;
			mt::Point3 pos;

			_wkSpace->moveTo(_goal);
			
			//get a copy of current conf
			RobConf* rConf = _wkSpace->getRobot(0)->getCurrentPos(); 
			RobConf currentRobConf;
			currentRobConf.setSE3(rConf->getSE3());
			currentRobConf.setRn(rConf->getRn());

			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos = linkTransf.getTranslation();
			cout<<"current tip position = ("<<pos[0]<<", "<<pos[1]<<", "<<pos[2]<<")"<<endl;
			
			cout<<"apply constrained kinematic"<<endl;
			vector<KthReal>  values;
			values.push_back(0.0);
			values.push_back(0.0);
			values.push_back(10);
			_wkSpace->getRobot(0)->ConstrainedKinematics(values);

			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos = linkTransf.getTranslation();
			cout<<"New tip position = ("<<pos[0]<<", "<<pos[1]<<", "<<pos[2]<<")"<<endl;
 
			cout<<"Restore initial position"<<endl;
			if(_wkSpace->getRobot(0)->Kinematics(currentRobConf))
			{
				linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
				pos = linkTransf.getTranslation();
				cout<<"Old tip position = ("<<pos[0]<<", "<<pos[1]<<", "<<pos[2]<<")"<<endl;
			} 
			else cout<<"Kinematics returned FALSE!"<<endl;
		return false;
*/



		}
/**/

/*******************
**** What follows is a version of the trySolve function that adds samples along the
***** cells of the grid that connect the init and the goal cells using the NF1 navigation function
***** It is implementes just to test the correct expansion of the NF1 function *****
********************/
/*
		bool GUIBROgridPlanner::trySolve()
		{

			_solved = false;

			gridVertex vi,vg,vc,vmin;
			if(findGraphVertex(_goal,&vg)==false || findGraphVertex(_init,&vi)==false)
			{
				cout<<"ERROR: No graph vertex found for init and goal samples"<<endl;
				return _solved;
			}

			if(grid->computeNF1(vg))
			{
				
				filteredGridGraph *fg = grid->getFilteredGraph();	
				PotentialMap pm = grid->getpotmat();

				vc = vi;
				//if navigation function did'nt arrive at initial cell, return false
				if(pm[vi] == -1) 
				{
					cout<<"CONNECTION NOT POSSIBLE: Navigation function could not reach the goal..."<<endl;
					return false;
				}

				//otherwise follow the negated values
				_path.clear();
				clearSimulationPath();
				graph_traits<filteredGridGraph>::adjacency_iterator avi, avi_end;


				Sample *smp;
				unsigned int i,j,k;
				int *maxSteps = grid->getDiscretization();
				vector<KthReal> coords(7);
				while(vc != vg)
				{			
					i = grid->getLocations()->at(vc).x;
					j = grid->getLocations()->at(vc).y;
					k = grid->getLocations()->at(vc).z;
					coords[0] = (((KthReal)i-0.5)/maxSteps[0]);
					coords[1] = (((KthReal)j-0.5)/maxSteps[1]);
					coords[2] = (((KthReal)k-0.5)/maxSteps[2]);
					coords[3] = _init->getCoords()[3];
					coords[4] = _init->getCoords()[4];
					coords[5] = _init->getCoords()[5];
					coords[6] = _init->getCoords()[6];
					smp = new Sample(7);
					smp->setCoords(coords);
					_path.push_back(smp);
				    _samples->add(smp);
					vmin = vc;
					for(tie(avi,avi_end)=adjacent_vertices(vc, *fg); avi!=avi_end; ++avi)
					{
						KthReal pneigh = pm[*avi];
						KthReal pcurr = pm[vmin];
						if(pneigh < pcurr) vmin = *avi; 
						//if(pm[*avi] < pm[vmin]) vmin = *avi; 
					}
					vc = vmin;
				}
				i = grid->getLocations()->at(vg).x;
				j = grid->getLocations()->at(vg).y;
				k = grid->getLocations()->at(vg).z;
				coords[0] = (((KthReal)i-0.5)/maxSteps[0]);
				coords[1] = (((KthReal)j-0.5)/maxSteps[1]);
				coords[2] = (((KthReal)k-0.5)/maxSteps[2]);
				coords[3] = _init->getCoords()[3];
				coords[4] = _init->getCoords()[4];
				coords[5] = _init->getCoords()[5];
				coords[6] = _init->getCoords()[6];
				smp = new Sample(7);
				smp->setCoords(coords);
				_path.push_back(smp);
				_samples->add(smp);
				_solved = true;
				return _solved;
			}
			else
			{
				_solved = false;
			}

			return _solved;

		}
		*/
/**************************************************
*** END trySolve to test NF1 
**************************************************/





	  }
}
