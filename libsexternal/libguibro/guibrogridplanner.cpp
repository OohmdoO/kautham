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

/* Author: Alexander Perez, Jan Rosell */



#include <time.h>
#include <libproblem/ivworkspace.h>
#include <libproblem/workspace.h>
#include <libsampling/sampling.h>
#include <libsampling/robconf.h>
#include <libsplanner/libioc/localplanner.h>
#include "guibrogridplanner.h"
#include "consbronchoscopykin.h"
#include <cctype>


using namespace Kautham;

   namespace GUIBRO{
	//! Constructor
    GUIBROgridPlanner::GUIBROgridPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, WorkSpace *ws):
              iocPlanner(stype, init, goal, samples, sampler, ws)
	{
		//set intial values
	    _gen = new LCPRNG(3141592621, 1, 0, ((unsigned int)time(NULL) & 0xfffffffe) + 1);//LCPRNG(15485341);//15485341 is a big prime number
        _drawnLink = _wkSpace->getRobot(0)->getNumLinks()-1; //the path of tip is defaulted
	  
	  //define look-ahead points
		_counterFirstPoint = _wkSpace->getNumObstacles();
		KthReal pos[3] = {0.0,0.0,0.0};
		KthReal ori[4] = {1.0,0.0,0.0,0.0};
		KthReal scale = 1;
		bool obscollisions = false;
        Robot *obs;
		//add points to look-ahead points as obstacles
		_maxLookAtPoints = 121;
		for(int i=0;i<_maxLookAtPoints;i++)
		{
            obs = new Robot("",scale,IVPQP);
			_wkSpace->addObstacle(obs);
		}


	  _showObstacle = new int[_wkSpace->getNumObstacles()];
	  for(int i=0; i<_wkSpace->getNumObstacles();i++) 
		  _showObstacle[i]=-1;

	  //interface to show obstacles (not considering look-ahead points
	  char str[50];
	  for(int i=0; i<_counterFirstPoint;i++) 
	  {
		if(i==_counterFirstPoint-1 || i==0)
		{		  
			//traqueobronquial tree is assumed to be the first object. It is shown
			//nodule is assumed to be the last obstacle. Is is shown.
			sprintf(str,"Show Obstacle %d (0/1)",i);
			addParameter(str, 1);//shown
		}
		else{
		  //the box and the grid are not shown
		  sprintf(str,"Show Obstacle %d (0/1)",i);
		  addParameter(str, 0);//not shown
		}
	  }


	  //interface to select goal obstacle (nodule)
	  _obstaclenodule=NULL;
	  _nodule = _counterFirstPoint-1;//nodule is assumed to be the last obstacle.
	  addParameter("Select Nodule",_nodule);

	  //interface to show look-ahead points
	  _showPoints = 0;
	  addParameter("Show Points (0/1)",_showPoints);

	  _stepsAdvance = 10.0;
	  addParameter("Steps Advance",_stepsAdvance);

	  _onlyNF1=0;
	  addParameter("Only NF1(0/1)",_onlyNF1);

	  
	  _randomness=1;
	  addParameter("Add Randomness (0/1)",_randomness);

	  
		_weightNF1 = 0.6;
	    addParameter("weight NF1",_weightNF1);
		_weightDist = 0.2; 
	    addParameter("weight Dist",_weightDist);
	   _weightAlpha = 0.2;
	    addParameter("weight Alpha",_weightAlpha);

		//set intial values from parent class data
		_speedFactor = 1;
		_solved = false;


	  
		_guiName = _idName = "GUIBRO Grid Planner";


		////add parameters defining the bronchoscope
		//_radius = 1.0;


        KthReal ssize=1.0;
        _locPlanner->setStepSize(ssize);
		addParameter("Step Size", ssize);

		addParameter("Speed Factor", _speedFactor);

        addParameter("Drawn Path Link",_drawnLink);

		_maxNumSamples=10000;
		removeParameter("Step Size");
		//removeParameter("Max. Samples");
		//removeParameter("Neigh Thresshold");
		//removeParameter("Drawn Path Link");
		//removeParameter("Max. Neighs");

		_wkSpace->getRobot(0)->setLinkPathDrawn(_drawnLink);


		//read dimensions.txt file
		int nx=100,ny=100,nz=100;
		float vx=0.5,vy=0.5,vz=0.5;
		float dx,dy,dz;
		FILE *fp;
		string filedim = _wkSpace->getDimensionsFile();
		fp = fopen(filedim.c_str(),"rt");
		if( fp != NULL){
		  //read first line: size of CT in voxels
		  fscanf(fp,"%d %d %d\n",&nx,&ny,&nz);
		  //read second line: size of each voxelsin mm
		  fscanf(fp,"%f %f %f\n",&vx,&vy,&vz);
		  //read third line: size of CT in mm
		  fscanf(fp,"%f %f %f\n",&dx,&dy,&dz);

		  _wkSpace->getRobot(0)->setLimits(0, 0, dx);
		  _wkSpace->getRobot(0)->setLimits(1, 0, dy);
		  _wkSpace->getRobot(0)->setLimits(2, 0, dz);

		  //read fourth line: location of center pf trachea in mmm
		  float tx,ty,tz;
		  fscanf(fp,"%f %f %f\n",&tx,&ty,&tz);
		  _tx = (KthReal)(tx/dx);
		  _ty = (KthReal)(ty/dy);
		  _tz = (KthReal)(tz/dz);

		  //read fourth line: location of nodule in mm
		  float nodulex,noduley,nodulez,noduleradius;
		  fscanf(fp,"%f %f %f %f\n",&nodulex,&noduley,&nodulez,&noduleradius);
		  fclose(fp);
    

		  //write (or rewrite) file box.iv
		  string filebox = _wkSpace->getDirCase() + "MODELS/box.iv";
		  fp = fopen(filebox.c_str(),"wt");
		  fprintf(fp,"#Inventor V2.1 ascii\n");
		  fprintf(fp,"Separator{\n");
		  fprintf(fp,"\t Material {\n");
		  fprintf(fp,"\t\t diffuseColor 1.0 0.0 0.0\n");
		  fprintf(fp,"\t\t transparency 0.5\n");
		  fprintf(fp,"\t }\n");
		  fprintf(fp,"\t Translation {\n");
		  fprintf(fp,"\t\t translation %f %f %f\n",dx/2,dy/2,dz/2);
		  fprintf(fp,"\t }\n");
		  fprintf(fp,"\t Cube {\n");
		  fprintf(fp,"\t\t width %f\n",dx);
		  fprintf(fp,"\t\t height %f\n",dy);
		  fprintf(fp,"\t\t depth %f\n",dz);
		  fprintf(fp,"\t }\n");
		  fprintf(fp,"}\n");
		  fclose(fp);

		  //write (or rewrite) file nodule.iv
		  string filenodule = _wkSpace->getDirCase() + "MODELS/nodule.iv";
		  fp = fopen(filenodule.c_str(),"wt");
		  fprintf(fp,"#Inventor V2.1 ascii\n");
		  fprintf(fp,"DEF nodule Separator {\n");
		  fprintf(fp,"\t Material {\n");
		  fprintf(fp,"\t\t diffuseColor 1.0 0.5 0.5\n");
		  fprintf(fp,"\t\t specularColor 0.5 0.5 0.5\n");
		  fprintf(fp,"\t\t transparency 0.5\n");
		  fprintf(fp,"\t }\n");
		  fprintf(fp,"\t Sphere {\n");
		  fprintf(fp,"\t\t radius %f\n",noduleradius);
		  fprintf(fp,"\t }\n");
		  fprintf(fp,"}\n");
		  fclose(fp);
		}
		else{
			cout<<"ERROR opening dimension.txt file"<<endl;
		}


		KthReal thresholdDist = 0;//-5; //to consider a grpah including cells from distance -5 upwards.
        grid = new workspacegridPlanner(stype, init, goal, samples, sampler, ws, thresholdDist);
		grid->setImageSize(nx,ny,nz);
		grid->setVoxelSize(vx,vy,vz);
		grid->discretizeCspace();

		pparse = new PathParse(9);
		//pparse->setDimPoint(9);


 //read the bronchsocope diameter from the name of the robot in the file *.rob
		//the name is to be composed of a string plus a number in thenth of mm
		//and convert to radius in mm
		string s=_wkSpace->getRobot(0)->getName();
		s.erase(std::remove_if(s.begin(), s.end(), (int(*)(int))std::isalpha), s.end()); 
		_bronchoscopeRadius = atoi(s.c_str()) / 20.;
  		_bronchoscopeRadius = _bronchoscopeRadius <= 1.0 ? 1.0 : _bronchoscopeRadius ;
    }

	//! void destructor
	GUIBROgridPlanner::~GUIBROgridPlanner(){
			
	}


//! Collision-check based on the spheres centered at each link origin.
	//!Bronchoscope links are defined as a cylinder of heigth 1mm and two spheres of radius 1mm centered
	//!at the bottom and top of the cylinder. The bottom sphere of one link coincides with the
	//!top sphere of the previous link.
	//!For collision-check purposes the bottom sphere of each link is considered.
	bool GUIBROgridPlanner::collisionCheck(KthReal *distcost, KthReal *NF1cost,  bool onlytip)
	{
		//number maximum of touching links
		int maxTouching = 5;

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
		//transform of the origin of the base
		mt::Transform baseTransf;
		//position of the origin of each link
		mt::Point3 pos;
		//position of the origin of the base
		mt::Point3 basePos;
		//indices and label of the cell closest to the origin of the link
		unsigned int i,j,k,label;
		//distance of the occupied cell to the walls of the bronchi 
		int dist;
		//flag indicating if the bronchoscope is in collision or not
		bool collision=false;
		//flag indicating if the sphere centered at the occupied cell is free or not
		bool freesphere;
		//threshold distance, measured in cells,
		//int threshold = (int)(sqrt(3.0)*maxSize/radius);//sqrt(3) is added to consider the distance to the vertex of the voxel
		int threshold = (int)(_bronchoscopeRadius/maxSize); //+1;

		//int threshold = 2;
		//reset the cost
		*distcost=0.0;
		if(onlytip==false)
		{
			_touchinglabels.clear();
			_collisionlabels.clear();
			_freelabels.clear();
			_outofboundslabels.clear();

			for(int n=0;n<nlinks;n++)
			{
				linkTransf = _wkSpace->getRobot(0)->getLinkTransform(n);
				pos = linkTransf.getTranslation();
				//i = (pos[0]-O[0]+voxelSize[0]/2)/voxelSize[0];
				//j = (pos[1]-O[1]+voxelSize[1]/2)/voxelSize[1];
				//k = (pos[2]-O[2]+voxelSize[2]/2)/voxelSize[2];
				i = (pos[0]-O[0]+voxelSize[0]/2)/voxelSize[0]+1;
				j = (pos[1]-O[1]+voxelSize[1]/2)/voxelSize[1]+1;
				k = (pos[2]-O[2]+voxelSize[2]/2)/voxelSize[2]+1;
				label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;
				dist = -10*threshold; //value that will take thos spheres out of bounds
				
				freesphere=grid->getDistance(label, &dist);
				if(freesphere){//within bounds
					if(dist<threshold) 
					{
						_collisionlabels.push_back(label);
						collision = true;
					}
					else if(dist==threshold){//if more than maxTouching spheres are touching then consider it is a collision
						_touchinglabels.push_back(label);
					}
					else
					{
						_freelabels.push_back(label);
					}
				}
				else 
				{
					//out of bounds considered as collision
					_outofboundslabels.push_back(label);
					collision = true;
				}
				
				*distcost += (KthReal)dist;
			}
		}
		
		//for the tip of the bronchoscope: 
		int n=nlinks-1;
		linkTransf = _wkSpace->getRobot(0)->getLinkTransform(n);
		//Evaluate the sphere of the tip
		mt::Transform totip;	
		totip.setRotation(mt::Rotation(0.0,0.0,0.0,1));
		totip.setTranslation(mt::Vector3(_wkSpace->getRobot(0)->getLink(n)->getA(),0,0) );
		mt::Transform tipTransf = linkTransf*totip;
		pos = tipTransf.getTranslation();
		i = (pos[0]-O[0]+voxelSize[0]/2)/voxelSize[0] + 1;
		j = (pos[1]-O[1]+voxelSize[1]/2)/voxelSize[1] + 1;
		k = (pos[2]-O[2]+voxelSize[2]/2)/voxelSize[2] + 1;
		label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;
		dist = -10*threshold; //value that will take thos spheres out of bounds
		freesphere=grid->getDistance(label, &dist);

		//more constrained for the tip, if it is more near to the walls as detected by the gradient of the distance
		/*
		mt::Point3 f, pgrid;
		pgrid[0] = voxelSize[0]*(i-1);
		pgrid[1] = voxelSize[1]*(j-1);
		pgrid[2] = voxelSize[2]*(k-1);
		if(grid->computeDistanceGradient(label,&f)==true)
		{		
			//dot product f.v
			if(f.dot(pos-pgrid) < 0) threshold += 1;
		}
		*/
		
		


		if(freesphere==true) //within bounds 
		{
			if(dist<threshold)//below  threshold
			{
				_collisionlabels.push_back(label);
				collision = true;
			}
			else if(dist==threshold)//equal to threshold
			{
				_touchinglabels.push_back(label);
				if(_touchinglabels.size()>maxTouching) 
				{
					//many links touching considered as collision
					collision = true;
				}
			}
			else//above threshold
			{
				_freelabels.push_back(label);
			}
		}
		else //outof bounds
		{
			_outofboundslabels.push_back(label);
			collision = true;
		}
		*distcost += (KthReal)dist;
		

		//Evaluate the NF1 function
		grid->getNF1value(label, NF1cost);
		
		//_touchinglabels only stores the touching labels when the bronchoscope is
		//in a touching situation, i.e. all links are either free or touching
//		if(_touchinglabels.size()<maxTouching)
//			_touchinglabels.clear();

		return collision;
	}

//compute distance cost as the dot product between the motion direction 
//and the gradient of the ditance at the tip
//The best points have results in a greater value since the motion tends to move the tip towards 
	//a place with more clearance
void GUIBROgridPlanner::computedcost(mt::Point3 posini,mt::Point3 posend, KthReal *dcost)
	{
		//compute motion vector
		mt::Vector3 v;
		v = posend-posini;

		//compute label of tip at the end position
		KthReal *O = grid->getOrigin();
		KthReal *voxelSize = grid->getVoxelSize();
		int *steps = grid->getDiscretization();
		int i = (posend[0]-O[0]+voxelSize[0]/2)/voxelSize[0] + 1;
		int j = (posend[1]-O[1]+voxelSize[1]/2)/voxelSize[1] + 1;
		int k = (posend[2]-O[2]+voxelSize[2]/2)/voxelSize[2] + 1;
		unsigned long int label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;

		//compute ditance gradient at that label
		mt::Point3 f;
		if(grid->computeDistanceGradient(label,&f)==true)
		{		
			//dot product f.v
			*dcost = f.dot(v);
			if(*dcost<0) *dcost=0.0;
		}
		else *dcost=0.0;
	}



	//! comply moves the bronchoscope to comply to a multicontact situation
	bool GUIBROgridPlanner::comply(KthReal *distcost, KthReal *NF1cost, bool onlytip )
	{
		bool c=collisionCheck(distcost, NF1cost,  onlytip);


		return c;

/* NOT CLEAR
		//If there are not collision or outofbounds labels, and at least there is a touching label
		//then comply at the touch label, otherwise return c (either true or false)
		
		if( _freelabels.size()>0 && _touchinglabels.size()>0 &&
			_collisionlabels.size()==0 && _outofboundslabels.size()>0) 
		{
			//Comply to the labels that are touching, if possible
			KthReal *voxelSize = grid->getVoxelSize();
			KthReal maxSize = voxelSize[0];
			if(voxelSize[1]>maxSize) maxSize = voxelSize[1];
			if(voxelSize[2]>maxSize) maxSize = voxelSize[2];

			RobConf* currentPos = _wkSpace->getRobot(0)->getCurrentPos();
			RnConf& rnConf = currentPos->getRn();
			RobConf robotConf;
			vector<KthReal> coords(7);
			mt::Point3 f,fk;
			f[0] = 0.0;
			f[1] = 0.0;
			f[2] = 0.0;

			for(int k=0; k<_touchinglabels.size();k++)
			{
				if(grid->computeDistanceGradient(_touchinglabels[k],&fk))
				{
					f[0] += fk[0];
					f[1] += fk[1];
					f[2] += fk[2];
				}
			}
			KthReal length = maxSize * abs(f.dot(fk));
			f=f.normalize();

			mt::Transform baseTransf = _wkSpace->getRobot(0)->getLinkTransform(0);
			mt::Rotation baseRot = baseTransf.getRotation();
			mt::Point3 basePos = baseTransf.getTranslation();
			for(int i =0; i < 3; i++)
				coords[i] = basePos[i] + length*f[i];
			for(int i =0; i < 4; i++)
				coords[i+3] = baseRot[i];
				
			robotConf.setSE3(coords);
			robotConf.setRn(rnConf);
			_wkSpace->getRobot(0)->Kinematics(robotConf);

			bool newc = collisionCheck(distcost, NF1cost, onlytip, radius);
			if(newc)
			{
				//complying motion resulted in collision, then restore the last position
				for(int i =0; i < 3; i++)
					coords[i] = basePos[i];
				for(int i =0; i < 4; i++)
					coords[i+3] = baseRot[i];
				
				robotConf.setSE3(coords);
				robotConf.setRn(rnConf);
				_wkSpace->getRobot(0)->Kinematics(robotConf);
				//once restored the value of c is that computed at the beginning
			}
			else c=newc;
		}

		return c;
*/
	}


	//! findGraphVertex retunrns true if the sample s corresponds to a configuration of the 
	//! bronchoscope with the base of the last link occupying a cell of the grid of the bronchi
	bool GUIBROgridPlanner::findGraphVertex(KthReal x, KthReal y, KthReal z, KthReal R, gridVertex *v)
	{		
		//origin of the regular grid in world coordinates
		KthReal *O = grid->getOrigin();
	
		//size of the voxels in the grid
		KthReal *voxelSize = grid->getVoxelSize();
		
		//number of cells per axis
		int *steps = grid->getDiscretization();

		int i = (x-O[0]+voxelSize[0]/2)/voxelSize[0]+1;
		int j = (y-O[1]+voxelSize[1]/2)/voxelSize[1]+1;
		int k = (z-O[2]+voxelSize[2]/2)/voxelSize[2]+1;
		unsigned int label =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;

		//The center of the nodule lies in the grid
		if(grid->getGraphVertex(label, v)==true)
			return true;
		
		//else, verify if any cell of the grid belong to the sphere-nodule
		//find label of the bottom-left corner of a cube centered at x,y,z
		i = (x-R-O[0]+voxelSize[0]/2)/voxelSize[0]+1; if(i<1) i=1;
		j = (y-R-O[1]+voxelSize[1]/2)/voxelSize[1]+1; if(j<1) j=1;
		k = (z-R-O[2]+voxelSize[2]/2)/voxelSize[2]+1; if(k<1) k=1;
		unsigned long int labelMin =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;
		//find label of the top-right corner of a cube centered at x,y,z
		i = (x+R-O[0]+voxelSize[0]/2)/voxelSize[0]+1; if(i>steps[0]) i=steps[0];
		j = (y+R-O[1]+voxelSize[1]/2)/voxelSize[1]+1; if(j>steps[1]) j=steps[1];
		k = (z+R-O[2]+voxelSize[2]/2)/voxelSize[2]+1; if(k>steps[2]) k=steps[2];
		unsigned long int labelMax =steps[0]*steps[1]*(k-1)+steps[0]*(j-1)+i;
		
		KthReal ix,iy,iz,dist,distmin;
		distmin=1000000000.0;
		unsigned int imin=-1;
		int d;
		//sweep all cells of the cube centered at x,y,z and verify if a) they belong to the graph; 
		//b) they have a distance value = 0 (they belong to the wall of the bronchi;
		//c) they are at a distance < R from the center x,y,z
		//if several cells satisfy the conditions, store the one closer to x,y,z 
		for(unsigned int i=labelMin+1; i<labelMax;i++)
		{
			//verify if the cell with label i belongs to the graph
			if(grid->getDistance(i,&d)==true)
			{
				//and if it is a cell of the wall of the bronchi
				if(d==0)
				{
					int coordx,coordy,coordz;
					grid->getCoordinates(i,&coordx,&coordy,&coordz);
					ix = coordx*voxelSize[0]-voxelSize[0]/2+O[0];
					iy = coordy*voxelSize[1]-voxelSize[1]/2+O[1];
					iz = coordz*voxelSize[2]-voxelSize[2]/2+O[2];
					dist = sqrt((ix-x)*(ix-x)+(iy-y)*(iy-y)+(iz-z)*(iz-z));
					//if it belongs to the sphere-nodule centered at x,y,z
					if(dist<R)
					{
						//if it is closer to the center xyz
						if(dist<distmin) imin = i;
					}
				}
			}
		}
		if(imin!=-1)
		{
			grid->getGraphVertex(imin, v);
			return true;
		}
		return false;
	}

	//! findGraphVertex retunrns true if the sample s corresponds to a configuration of the 
	//! bronchoscope with the base of the last link occupying a cell of the grid of the bronchi
	bool GUIBROgridPlanner::findGraphVertex(Sample * s, gridVertex *v)
	{
		unsigned int label = findGridCell(s);
		if(grid->getGraphVertex(label, v)==false)
		{
			return false;
		}
		return true;
	}

	//! findGridCell retunrs the label of the grid cell ocupied by the base of the last link of 
	//! the bronchoscope when located at sample s.
	unsigned int GUIBROgridPlanner::findGridCell(Sample * s)
	{
		//origin of the regular grid in world coordinates
		KthReal *O = grid->getOrigin();
		
		//size of the voxels in the grid
		KthReal *voxelSize = grid->getVoxelSize();
		
		//number of cells per axis
		int *steps = grid->getDiscretization();
		
		//transform of the origin of each link
		mt::Transform linkTransf;
		//position of the origin of each link
		mt::Point3 pos;
		//indices and label of the cell closest to the origin of the last link
		unsigned int i,j,k,label;

		//move to s
		_wkSpace->moveRobotsTo(s);
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
        HASH_S_K::iterator it;
		
		/*it = _parameters.find("Step Size");
		if(it != _parameters.end())
			setStepSize(it->second);//also changes stpssize of localplanner
        else
          return false;
	    */

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

		

		it = _parameters.find("Select Nodule");
		if(it != _parameters.end())
		{
          _nodule = it->second;
		  if(_nodule>0 && _nodule<_counterFirstPoint)
			  _obstaclenodule = _wkSpace->getObstacle(_nodule);
		  else _obstaclenodule=NULL;
		}
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
            _showPoints = it->second;
			for(int i=_counterFirstPoint; i<_wkSpace->getNumObstacles();i++)
			{
				//if the points should be removed
				if(_showPoints ==0)
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

		
		it = _parameters.find("weight NF1");
		if(it != _parameters.end()){
          _weightNF1 = it->second;
		}else
          return false;

		it = _parameters.find("weight Dist");
		if(it != _parameters.end()){
          _weightDist = it->second;
		}else
          return false;

		it = _parameters.find("weight Alpha");
		if(it != _parameters.end()){
          _weightAlpha = it->second;
		}else
          return false;
	
		
		it = _parameters.find("Add Randomness (0/1)");
		if(it != _parameters.end()){
          _randomness = it->second;
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
        _wkSpace->moveRobotsTo(_simulationPath[step]);

		((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->registerValues();
		KthReal a = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(0);
		KthReal b = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(1);
        std::cout << "alpha = " << a<<" beta = "<<b<<std::endl;

      }else
        std::cout << "The problem is wrong solved. The solution path has less than two elements." << std::endl;
	}
  }


    //! advanceToBest 
	int GUIBROgridPlanner::advanceToBest(KthReal stepsahead, KthReal *bestAlpha_11, KthReal *bestBeta_11,Sample *smp, FILE *fp)
	{
		vector<KthReal>  values;
		KthReal currentNF1cost;
		KthReal currentdcost;
		

		KthReal a_11,b_11;
		//look "stepsahead" steps ahead and return the best alpha and beta
		int r = look(stepsahead, &a_11, &b_11);
		//if advance is possible
		if(r>0)
		{
			mt::Transform linkTransf;
			mt::Point3 pos,pos0;
			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos0 = linkTransf.getTranslation();

			//move one step ahead (i.e. Delta_z=1) interpolating de values of alpha and beta.
			//(i.e. the optimla values alpha, beta should be reached after stepahead steps)

			//current alpha and beta values
			KthReal alpha0_11 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(0);
			KthReal beta0_11 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(1);
			//Correction of the stepsahead value as a function of the beta value: when the bending required for the 
			//bronchoscope (i.e. the diference between the current and the optimum beta) is big then the steps to
			//go ahead is increased. This correction is also taken into account in function look.
			//KthReal s = stepsahead/cos(2*M_PI*(b_11-beta0_11));
			KthReal s = stepsahead;
			
			//Interpolation when s>1
			if(s>1)
			{
				*bestAlpha_11 = alpha0_11+(a_11-alpha0_11)/s;
				*bestBeta_11 = beta0_11+(b_11-beta0_11)/s;
				values.push_back(*bestAlpha_11);
				values.push_back(*bestBeta_11);
				values.push_back(-1);
			}
			//else reach thebest alpha and beta values in a step of s
			else
			{
				*bestAlpha_11 = a_11;
				*bestBeta_11 = b_11;
				values.push_back(*bestAlpha_11);
				values.push_back(*bestBeta_11);
				values.push_back(-s);
			}
			
			//advance
			_wkSpace->getRobot(0)->ConstrainedKinematics(values);
			comply(&currentdcost,&currentNF1cost);
			

			if(fp!=NULL)
			{	
				bool colltest = collisionCheck(&currentdcost,&currentNF1cost);
				fprintf(fp,"[%.2f %.2f %.2f %.2f %.2f %.2f %f %f],\n",
					s,*bestAlpha_11,*bestBeta_11, pos0[0],pos0[1],pos0[2],currentNF1cost,currentdcost);
				//fprintf(fp,"Apply steps= %.2f a= %.2f b= %.2f from (%.2f %.2f %.2f) Reached: NF1= %f Dist = %d col = FALSE\n",
				//	s,*bestAlpha*180/M_PI,*bestBeta*180/M_PI, pos0[0],pos0[1],pos0[2],currentNF1cost,currentdcost);
			}

			/*
			//verify if the interpolation step towards the best configuration is collision-free
			int currentNF1cost;
			int currentdcost;
			bool colltest = collisionCheck(&currentdcost,&currentNF1cost);

			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos = linkTransf.getTranslation();
			if(fp!=NULL)
			{
				if(colltest) fprintf(fp,"Apply steps= %.2f a= %.2f b= %.2f to go from (%.2f %.2f %.2f) to (%.2f %.2f %.2f) Reached: NF1= %d Dist = %d col = TRUE\n",
					s,*bestAlpha,*bestBeta, pos0[0],pos0[1],pos0[2],pos[0],pos[1],pos[2],currentNF1cost,currentdcost);
				else fprintf(fp,"Apply steps= %.2f a= %.2f b= %.2f to go from (%.2f %.2f %.2f) to (%.2f %.2f %.2f) Reached: NF1= %d Dist = %d col = FALSE\n",
					s,*bestAlpha,*bestBeta, pos0[0],pos0[1],pos0[2],pos[0],pos[1],pos[2],currentNF1cost,currentdcost);
			}
			else{
				if(colltest){ 
					cout<<"r= "<<r<< "Apply steps= "<<s<<" a= "<<*bestAlpha<<" b= "<<*bestBeta;
					cout<<" to go from ("<<pos0[0]<<pos0[1]<<pos0[2]<<") to ("<<pos[0]<<pos[1]<<pos[2];
					cout<<") Reached: NF1= "<<currentNF1cost<<" Dist = "<<currentdcost<<" col = TRUE"<<endl;
				}
				else{ 
					cout<<"r= "<<r<< "Apply steps= "<<s<<" a= "<<*bestAlpha<<" b= "<<*bestBeta;
					cout<<" to go from ("<<pos0[0]<<pos0[1]<<pos0[2]<<") to ("<<pos[0]<<pos[1]<<pos[2];
					cout<<") Reached: NF1= "<<currentNF1cost<<" Dist = "<<currentdcost<<" col = FALSE"<<endl;
				}
			}

			if(colltest)
			{
				cout<<"ERROR - this code should not be reached..."<<endl;
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
			*/

			//store the configuration reaches if a sample pointer smp was passed as a parameter
			if(smp!=NULL)
			{
                smp->setMappedConf(_wkSpace->getRobConfigMapping());
			}
		}
		//else: goal reached
		else if(r==-1)
		{
			cout<<"Goal already reached!!, no best configuration to move..."<<endl;
		}
		//else: advance not possible, i.e. we're stuck at a loca minima- any advance motion will increase the NF1 value
		else if(r==-2)
		{
			cout<<"Sorry, local minima. Cannot move to a better configuration..."<<endl;
		}
		//else: advance not possible. Any motion results in collision
		else{
			cout<<"Sorry, no free configuration available. Cannot move to a better configuration..."<<endl;
		}
		return r;
	}

//looks stepsahead for different values of alpha and beta and select the pair that has a better cost.
int GUIBROgridPlanner::look(KthReal stepsahead, KthReal *bestAlpha_11, KthReal *bestBeta_11)
		{
			//clock_t computelookatpointsentertime = clock();
			//store current position of alpha and beta
			KthReal alpha0_11 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(0);
			KthReal beta0_11 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(1);

			//vector of values alpha and beta to consider
			vector<KthReal> alpha_11(_maxLookAtPoints);
			vector<KthReal> beta_11(_maxLookAtPoints);	

			//working in degrees NO
			KthReal DeltaAlpha_RAD;//to be determined as a function of beta
			KthReal maxAlpha_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxAlpha();
			KthReal minAlpha_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMinAlpha();
			//KthReal maxAlpha = (180/M_PI)*((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxAlpha();
			//KthReal minAlpha = (180/M_PI)*((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMinAlpha();
			KthReal a_RAD;
			KthReal a0_RAD;
			if(alpha0_11>0) a0_RAD = maxAlpha_RAD*alpha0_11; //alpha_11 goes from -1 to +1 that corresponds to the range [minAlpha,maxAlpha]
			else a0_RAD = -minAlpha_RAD*alpha0_11; //recall that minAlpha is a negative value
			KthReal alphaRange_RAD;

			KthReal DeltaBeta_RAD;// = 2;//fixed step
			//if(stepsahead<2.5) DeltaBeta = 5.0; 
			//else if(stepsahead<5.0) DeltaBeta = 4.0; 
			//else if(stepsahead<7.5) DeltaBeta = 3.0; 
			//else DeltaBeta = 2.0; 
			//DeltaBeta = 5.0-(stepsahead-2.5)/2.5;
			
			KthReal maxBeta_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxBending();
			KthReal minBeta_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMinBending();
			//KthReal maxBeta = (180/M_PI)*((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxBending();
			//KthReal minBeta = (180/M_PI)*((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMinBending();
			KthReal b_RAD;
			KthReal b0_RAD;
			if(beta0_11>0) b0_RAD= maxBeta_RAD*beta0_11; //beta_11 goes from -1 to +1 that corresponds to the range [minBeta,maxBeta]
			else b0_RAD=-minBeta_RAD*beta0_11; //recall that minBeta is a negative value

			//DeltaBeta is the step considered for the screening of beta 
			//When stepsahead is big then DeltaBeta is small 
			//DeltaBeta = 10.0-stepsahead/2.5;
			//if(DeltaBeta<2.0) DeltaBeta=2.0;
			
			//DeltaBeta = fabs(maxBeta-b0)/10;
			//if(DeltaBeta>5) DeltaBeta=5;

			//beta is swept by 10 steps of DeltaBeta size centered at b0, i.e. the range b0-5*DeltaBeta and b0+5DeltaBeta
			//but if this range lies outside [minBeta,maxBeta] then the range swept is still of size 10*DeltaBeta
			//but not centered at b0, but starting at minBeta (or ending at maxBeta)
			DeltaBeta_RAD=5*M_PI/180;//5degrees
			int imin,imax;
			if(b0_RAD<0)
			{
				if((b0_RAD-5.0*DeltaBeta_RAD)<minBeta_RAD) imin=(b0_RAD-minBeta_RAD)/DeltaBeta_RAD;
				else imin=-5;
				imax=imin+10;
			}
			else
			{
				if((5.0*DeltaBeta_RAD-b0_RAD)>maxBeta_RAD) imax=(maxBeta_RAD-b0_RAD)/DeltaBeta_RAD;
				else imax=5;
				imin=imax-10;
			}

			//Compute the lookatPoints, i.e. the candidates points towards advance is considered, by 
			//sweeping alpha and beta ranges.

			//sweep 10*DeltaBeta degrees in beta, centered around beta0
			int k=0;
			KthReal randoffset;
			for(int i=imin;i<=imax;i++)
			{
				alphaRange_RAD = maxAlpha_RAD-minAlpha_RAD;
				DeltaAlpha_RAD = alphaRange_RAD/10;

				//sweep alphaRange degrees around alpha0
				//for(int j=-5;j<=5;j++)
				for(int j=0;j<=10;j++)
				{
					if(_randomness)
					{
						//if(j==-5) randoffset=0.5*(KthReal)_gen->d_rand();
						//else if(j==5) randoffset=-0.5*(KthReal)_gen->d_rand();
						if(j==0) randoffset=0.5*(KthReal)_gen->d_rand();
						else if(j==10) randoffset=-0.5*(KthReal)_gen->d_rand();
						else randoffset=-0.5+(KthReal)_gen->d_rand();
						a_RAD = minAlpha_RAD + (j+randoffset)*DeltaAlpha_RAD;

						if(i==imin) randoffset=0.5*(KthReal)_gen->d_rand();
						else if(i==imax) randoffset=-0.5*(KthReal)_gen->d_rand();
						else randoffset=-0.5+(KthReal)_gen->d_rand();
						b_RAD = b0_RAD + (i+randoffset)*DeltaBeta_RAD;
					}
					else 
					{
						//a = j*DeltaAlpha;
						a_RAD = minAlpha_RAD + j*DeltaAlpha_RAD;
						b_RAD = b0_RAD + i*DeltaBeta_RAD;
					}
					
					//store a look-at point (a,b), in the normalized form between -1 and 1
					if(a_RAD>0) alpha_11[k] = a_RAD/maxAlpha_RAD;
					else alpha_11[k] = -a_RAD/minAlpha_RAD;
					if(b_RAD>0) beta_11[k] = b_RAD/maxBeta_RAD;
					else  beta_11[k] = -b_RAD/minBeta_RAD;
					k++;
				}
			}
			for(int i=k;i<_maxLookAtPoints;i++)
			{
					alpha_11[i] = alpha0_11;
					beta_11[i] = beta0_11;
			}

			//Now, compute the cost of each of the lookat points
			vector<KthReal> dcost(_maxLookAtPoints); //distance cost vector
			vector<KthReal> NF1cost(_maxLookAtPoints); //NF1 value cost vector
			vector<bool> col(_maxLookAtPoints); //color of he points: it will depend on the cost

			int freepoints = 0;
			KthReal minNF1cost = 1000000.0;
			KthReal maxNF1cost = -1000000.0;
			KthReal mindcost = 1000000.0;
			KthReal maxdcost = -1000000.0;
			KthReal currentNF1cost;
			KthReal currentdcost;
			int pointminNF1cost = -1;

			//current cost 
			bool colltest = collisionCheck(&currentdcost,&currentNF1cost);
			if(currentNF1cost==0.0)
			{
				//reached
				return -1;
			}

			
			//clock_t computelookatpointsfinaltime = clock();
			//_computelookatpointstime += (double)(computelookatpointsfinaltime-computelookatpointsentertime)/CLOCKS_PER_SEC;
			//clock_t evallookatpointsentertime = clock();

			//sweep al the lookat points and compute their cost
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				//alpha = alpha0 + (KthReal)_gen->d_rand()*0.2-0.1;
				//xi = xi0 + (KthReal)_gen->d_rand()*0.2-0.1;

				//correct advance step as a function of beta
				//KthReal s = stepsahead/cos(2*M_PI*(beta_11[i]-beta0_11));
				KthReal s = stepsahead;

				//evaluate the i lookatpoint, i.e. verifies if it is collision-free and if so locates the point
				//at the tip poisition. retuns the distance cost and the NF1 cost

				
				//clock_t testlookatpointentertime = clock();
				col[i] = testLookAtPoint(_counterFirstPoint+i, alpha_11[i], beta_11[i], s, &dcost[i], &NF1cost[i]);
				//clock_t testlookatpointfinaltime = clock();
				//_testlookatpointtime += (double)(testlookatpointfinaltime-testlookatpointentertime)/CLOCKS_PER_SEC;

				//restore alpha0, beta0
				//((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->setvalues(alpha0,0);
				//((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->setvalues(beta0,1);

				//for those points collision-free, find the minimum and maximum values of the distance and  NF1 costs
				if(col[i]==false)
				{
					//find extreme values for NF1 cost
					/**/
					if(NF1cost[i]>0.0 && NF1cost[i]<=currentNF1cost){
						if(NF1cost[i]<minNF1cost){
							minNF1cost = NF1cost[i];
							pointminNF1cost = _counterFirstPoint+i;
						}
						else if(NF1cost[i]>maxNF1cost){
							maxNF1cost = NF1cost[i];
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
					}
					/**/
					
					/*if(NF1cost[i]>0){
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
					*/
					//cout<<"Point "<<i<<"("<<alpha[i]<<","<<xi[i]<<") collision "<<col<<" cost = "<<cost<<endl; 
					freepoints++;
				}
			}

			//if no lookat point has a NF1 value, return 0 (we are looking at the parenchima (outside the bronchi)
			KthReal color[3];
			if(pointminNF1cost==-1) {
				for(int i=0; i<_maxLookAtPoints;i++)
				{
					color[0]=0.8;
					color[1]=0.2;
					color[2]=0.2;
                    for (uint j = 0;  j < _wkSpace->getObstacle(_counterFirstPoint+i)->getNumLinks(); j++) {
                        _wkSpace->getObstacle(_counterFirstPoint+i)->getLink(j)->getElement()->setColor(color);
                    }
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
			int localminima=1;
			KthReal bestdcost;
			KthReal bestNF1cost;
			int besti=-1;

			vector<int> indexi;
			vector<KthReal> rNF1(_maxLookAtPoints);
			vector<KthReal> rDist(_maxLookAtPoints);
			vector<KthReal> rAlpha(_maxLookAtPoints);
			KthReal r;
			//Sweep the lookatpoints and store all those that improve the NF1 value in vector indexi
			//compute rNF1 the normalized value of the NF1 cost
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				if(col[i]==false)
				{
					if(NF1cost[i]>0.0 && NF1cost[i]<currentNF1cost)
					{
						rNF1[i] = (NF1cost[i]-minNF1cost)/(maxNF1cost-minNF1cost);
						indexi.push_back(i);
						//if(NF1cost[i]<currentNF1cost) indexi.push_back(i);
					}
					//those with greater NF1 value are discarded
					else rNF1[i] = -1;
				}
				//those in collision are are discarded
				else rNF1[i] = -1;
			}
			//among those that imnprove the NF1 value chose the one that has a better 
			//clearance and a smaller alpha value (when beta is small)
			//First compute rDist, the noramilzed value corresponding to the Distance cost
			for(int i=0; i<_maxLookAtPoints;i++) rDist[i] = 1.0;
			for(int j=0; j<indexi.size();j++)
			{
				int i = indexi[j];
				//lower values of rdist are better, then it has to be defined as:
				rDist[i] = 1-((dcost[i]-mindcost)/(maxdcost-mindcost));
			}			

			//Then compute rAlpha, the value that weights the amount of alpha angle
			//rAlpha is big if abs(alpha) is big and beta is small, and is small if alpha is small and/or beta is big
			for(int i=0; i<_maxLookAtPoints;i++) rAlpha[i] = 1.0;
			for(int j=0; j<indexi.size();j++)
			{
				int i = indexi[j];
				//rAlpha[i] = (1-abs(beta[i]))*abs(alpha[i]);
				//rAlpha[i] = (1-abs(beta[i]))*abs(alpha[i]-alpha0);//NO!!

				//we want small alphas when beta is small AND we do not want grat increments of alpha
				rAlpha[i] = 0.5*(1-abs(beta_11[i])*abs(alpha_11[i])+0.5*abs(alpha_11[i]-alpha0_11))/2;
				//Adding a cost when either alpha or beta are negative
				//if(beta[i]<0 || alpha[i]<0) rAlpha[i]+=0.1;

				/*
				//rAlpha weights three terms:
				KthReal w1,w2,w3;
				//w1:when beta is small we prefer small alpha values, i.e. the more comfortable posture
				//if beta is not small then alpha will take the required value, i.e. no many options will be possible
				//in constrained situationsa.
				w1=1.0/2.0;
				//w2: we do not want big changes of alpha, only if they approach us to a small values of alpha
				if(abs(alpha[i])>abs(alpha0)) w2=1.0/2.0;
				else  w2=0.0;
				//w3: we do not want negative values of alpha or beta
				if(beta[i]<0 || alpha[i]<0)  w3=0;//1.0/3.0;
				else  w3=0.0;
				rAlpha[i] = w1*(1-abs(beta[i])*abs(alpha[i])+w2*abs(alpha[i]-alpha0))+w3;
				*/
			}

			//now compute the cost of each point as a function of rNF1, rDist and rAlpha, taking into 
			//account the weights
			for(int i=0; i<_maxLookAtPoints;i++)
			{
				//draw light red if in collision or non-NF1 value
				if(rNF1[i]==-1) 
				{
					color[0]=0.8;
					color[1]=0.5;
					color[2]=0.5;
				}
				//else compute the weighted average of rDist,rNF1 and rAlpha and color accordingly
				else
				{
					r = getWeightNF1()*rNF1[i]+getWeightDist()*rDist[i]+getWeightAlpha()*rAlpha[i];
					//keep track of the best option:
					if(r<minr)
					{
						minr=r;
						*bestAlpha_11 = alpha_11[i];
						*bestBeta_11 = beta_11[i];
						bestdcost=dcost[i];
						bestNF1cost=NF1cost[i];
						localminima=0;
						besti = i;
					}
					color[0]=0.0;
					color[1]=(0.2+(1-r)*0.8);
					color[2]=0.8*r;
				}
                for (uint j = 0;  j < _wkSpace->getObstacle(_counterFirstPoint+i)->getNumLinks(); j++) {
                    _wkSpace->getObstacle(_counterFirstPoint+i)->getLink(j)->getElement()->setColor(color);
                }
			}
			//draw the best point in White
			if(besti!=-1)
			{
				color[0]=1.0;
				color[1]=1.0;
				color[2]=1.0;
                for (uint j = 0;  j < _wkSpace->getObstacle(_counterFirstPoint+besti)->getNumLinks(); j++) {
                    _wkSpace->getObstacle(_counterFirstPoint+besti)->getLink(j)->getElement()->setColor(color);
                }
			}

			//clock_t evallookatpointsfinaltime = clock();
			//_evallookatpointstime += (double)(evallookatpointsfinaltime-evallookatpointsentertime)/CLOCKS_PER_SEC;



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




//Evaluates  the configuration stepsahead from the current one and draws a point at it.
//retunrs the distance cost and the NF1 cost
bool GUIBROgridPlanner::testLookAtPoint(int numPoint, KthReal alpha_11, 
										KthReal beta_11, KthReal stepsahead, KthReal *dcost, KthReal *NF1cost)
{

			mt::Transform linkTransf;
			mt::Point3 pos, posini;
			mt::Rotation rot;
			KthReal p[3];
			Vector3 zaxis;
			vector<KthReal>  values;
			bool colltest=false;

		//store current configuration
			RobConf* rConf = _wkSpace->getRobot(0)->getCurrentPos(); 
			RobConf currentRobConf;
			currentRobConf.setSE3(rConf->getSE3());
			currentRobConf.setRn(rConf->getRn());

		//Advance stepsahead steps, one by one	
			KthReal alpha0_11 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(0);
			KthReal beta0_11 = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getvalues(1);

			 
			KthReal Delta_a = (alpha_11-alpha0_11)/stepsahead;
			KthReal Delta_b = (beta_11-beta0_11)/stepsahead;
			values.push_back(alpha0_11);
			values.push_back(beta0_11);
			values.push_back(-1);
			
		    /**/
			clock_t advancecollcheckentertime = clock();
			for(int i=1; i<=stepsahead; i++)
			{
				values[0] += Delta_a;
				values[1] += Delta_b;
				//move the broncoscope at the new position
				_wkSpace->getRobot(0)->ConstrainedKinematics(values);

				//linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
				//mt::Transform tipTransf = linkTransf*totip;
				//pos = tipTransf.getTranslation();

				//compute the cost at the current position
				//colltest = collisionCheck(dcost,NF1cost, true);//test only the tip
				colltest = comply(dcost,NF1cost, true);//test only the tip
				if(colltest==true) break;
			}
			clock_t advancecollcheckfinaltime = clock();
			_advancecollchecktime += (double)(advancecollcheckfinaltime-advancecollcheckentertime)/CLOCKS_PER_SEC;
		    /**/

			//return to current position
			_wkSpace->getRobot(0)->Kinematics(currentRobConf);
			//restore alpha0, beta0
			((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->registerValues();
			
			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			int nlinks = _wkSpace->getRobot(0)->getNumLinks();
			mt::Transform totip;	
			totip.setRotation(mt::Rotation(0.0,0.0,0.0,1));
			totip.setTranslation(mt::Vector3( _wkSpace->getRobot(0)->getLink(nlinks-1)->getA(),0,0) );	
			posini = (linkTransf*totip).getTranslation();

			
			//Advance to final value
			values[0]=alpha_11;
			values[1]=beta_11;
			values[2] = -stepsahead;
			

			//Advance to final value
			_wkSpace->getRobot(0)->ConstrainedKinematics(values);
			
			//move the broncoscope at the new position
			linkTransf = _wkSpace->getRobot(0)->getLastLinkTransform();
			pos = (linkTransf*totip).getTranslation();

			if(colltest==false){
				//compute the cost at the last position
				//colltest = collisionCheck(dcost,NF1cost);
				colltest = comply(dcost,NF1cost);
			}

			//recompute the dcost in a different manner
			computedcost(posini,pos,dcost);

			//computes the position of the point at the extrem of the broncoscope at the new position
			p[0]=pos[0];
			p[1]=pos[1];
			p[2]=pos[2];
            _wkSpace->getObstacle(numPoint)->getLink(0)->getElement()->setPosition(p);

			//return to current position
			_wkSpace->getRobot(0)->Kinematics(currentRobConf);
			//restore alpha0, beta0
			((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->registerValues();

			return colltest;
		}
	

void GUIBROgridPlanner::verifyInvJacobian(int linktested, KthReal vx,KthReal vy, KthReal vz,
										   KthReal curralpha_11, KthReal currbeta_11, KthReal Dalpha, KthReal Dbeta)
{	
//Primer canvio de base vx,vy,vz per posar-la en la base del link linktested
		mt::Transform linkTransf;
		linkTransf = _wkSpace->getRobot(0)->getLinkTransform(linktested);
		mt::Rotation linkRot;

		linkRot = linkTransf.getRotation();
		mt::Point3 v0(vx, vy, vz);
		mt::Point3 v;
		v = linkRot(v0);   

		v[0]=0;
		v[1]=1;
		v[2]=0;


//ini prova de invJacobian	
		KthReal Da,Dx,Dz;

		KthReal DalphaRAD, curralphaRAD;
		KthReal maxAlpha_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxAlpha();
		KthReal minAlpha_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMinAlpha();
		if(curralpha_11>0) {
			DalphaRAD= maxAlpha_RAD*Dalpha;  
			curralphaRAD= maxAlpha_RAD*curralpha_11; 
		}
		else {
			DalphaRAD=-minAlpha_RAD*Dalpha; 
			curralphaRAD=-minAlpha_RAD*curralpha_11; 
		}
			
		KthReal  currxiRAD,DxiRAD;
		KthReal maxBeta_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMaxBending();
		KthReal minBeta_RAD = ((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->getMinBending();
		if(currbeta_11>0) {
			DxiRAD= maxBeta_RAD*Dbeta/(_wkSpace->getRobot(0)->getNumJoints()-1); 
			currxiRAD= maxBeta_RAD*currbeta_11/(_wkSpace->getRobot(0)->getNumJoints()-1);  
		}
		else {
			DxiRAD=-minBeta_RAD*Dbeta/(_wkSpace->getRobot(0)->getNumJoints()-1);  
			currxiRAD=-minBeta_RAD*currbeta_11/(_wkSpace->getRobot(0)->getNumJoints()-1);
		}


		//(Da,Dx,Dz) should be proportional to (DalphaRAD, DxiRAD, -1)

		if(((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->ApplyInverseJ(linktested,curralphaRAD,currxiRAD,v[0],v[1],v[2],&Da,&Dx,&Dz))
		{
			KthReal k1,k2,k3;
			k1=Da/DalphaRAD;
			k2=Dx/DxiRAD;
			k3=Dz/(-1);
			cout<<"InvJ: Apply ("<<vx<<", "<<vy<<", "<<vz<<") to link "<<linktested<<" results "<<Da<<", "<<Dx<<", "<<Dz<<")"<<endl;
		}
		else cout<<"InvJ Error"<<endl;
		//end  prova de invJacobian
}


//! function to find a solution path
 /**/
bool GUIBROgridPlanner::trySolve()
		{	

		cout<<"ENTERING TRYSOLVE!!!"<<endl;
	    clock_t entertime = clock();


			static gridVertex vinit=-1;
			static gridVertex vgoal=-1;
			gridVertex vi, vg;

			//first find if the init and goal configurations are valid, i.e. correspond to configurations with the tip 
			//of bronchoscope occupying a cell of the grid of the bronchi
			/*
			if(findGraphVertex(_goal,&vg)==false || findGraphVertex(_init,&vi)==false)
			{
				cout<<"ERROR: No graph vertex found for init and goal samples"<<endl;
				vi=-1;
				vg=-1;
				_solved=false;
				return _solved;
			}
			*/

			std::vector<KthReal>& initcoords = _init->getCoords();
			initcoords.at(6) = 0.5; //alpha forced to zero
			initcoords.at(7) = 0.5; //beta forced to zero	
			//write the initial position of the bronchoscope (xyz of the Init sample)
			initcoords.at(0) = _tx; //x  //0.546 0.614 0.759 0.0 0.0 0.244 0.5 0.5
			initcoords.at(1) = _ty; //y
			initcoords.at(2) = _tz; //z
			_init->setCoords(initcoords);


			_wkSpace->collisionCheck(_init);	//to set the mapped config

			//verify correctness of init sample
			if(findGraphVertex(_init,&vi)==false)
			{
				cout<<"ERROR: No graph vertex found for init sample"<<endl;
				vi=-1;
				vg=-1;
				_solved=false;
				return _solved;

			}
			//select goal sample - or goal nodule
			else
			{
				//if no nodule selected
				if(_obstaclenodule==NULL)
				{
					//verify correctness of goal sample
					if(findGraphVertex(_goal,&vg)==false)
					{
						cout<<"ERROR: No graph vertex found for goal sample"<<endl;
						vi=-1;
						vg=-1;
						_solved=false;
						return _solved;
					}
				}
				//a nodule is selected as a goal
				else
				{
                    KthReal x=_obstaclenodule->getLink(0)->getElement()->getPosition()[0];
                    KthReal y=_obstaclenodule->getLink(0)->getElement()->getPosition()[1];
                    KthReal z=_obstaclenodule->getLink(0)->getElement()->getPosition()[2];
                    KthReal R=_obstaclenodule->getLink(0)->getElement()->getScale();
					if(findGraphVertex(x,y,z,R,&vg)==false)
					{
						cout<<"ERROR: No graph vertex found for goal sample"<<endl;
						vi=-1;
						vg=-1;
						_solved=false;
						return _solved;
					}
				}
			}
			
			
			_simulationPath.clear();
			_cameraPath.clear();
			_path.clear();
			_solved = false;

			//if init or goal changed then recompute NF1
			if(vi != vinit || vg != vgoal)
			{
				vinit = vi;
				vgoal = vg;
				  
				//Comnpute NF1
				cout<<"START computing NF1 function"<<endl;
				grid->computeNF1(vg);
				cout<<"END computing NF1 function"<<endl;
			}

			//move to init
			_wkSpace->moveRobotsTo(_init);
			//restore alpha,beta corresponding to init
			((ConsBronchoscopyKin*)_wkSpace->getRobot(0)->getCkine())->registerValues();

			//weights for the selection of the best motion
			//const KthReal wNF1value=0.6;
			//const KthReal wDistvalue=0.4;
			//const KthReal wAlphavalue = 0.0;//0.2;
			KthReal wNF1=_weightNF1; 
			KthReal wDist=_weightDist;//these values are now set as parameteres of the planner.
			KthReal wAlpha=_weightAlpha;
			setWeights(wNF1,wDist,wAlpha);

			//we are done if only queried for the NF1 function values
			if(_onlyNF1) 
			{
				_solved=false;
				return _solved;
			}
			//End Comnpute NF1


			//////////////////////
			//Start finding a path
            Sample *smp = new Sample(_wkSpace->getNumRobControls());
			
			

			//store first config of the path
			_simulationPath.push_back(_init);	
			_path.push_back(_init);
						
			//store corresponding camara position 
			mt::Transform T_Ry;
			mt::Transform T_tz;
			mt::Transform T_cam;
			T_Ry.setRotation( mt::Rotation(mt::Vector3(0,1,0),-M_PI/2) );
			KthReal offset = _bronchoscopeRadius * 1.1;
			T_tz.setTranslation( mt::Vector3(0,0,-(_wkSpace->getRobot(0)->getLink(_wkSpace->getRobot(0)->getNumLinks()-1)->getA()+offset)) );
			T_cam = _wkSpace->getRobot(0)->getLinkTransform(_wkSpace->getRobot(0)->getNumLinks()-1)*T_Ry*T_tz;
			Planner::addCameraMovement(T_cam);

			//Recording info
			FILE *fp;
			char str[50];
			sprintf(str,"results%.0f%.0f%.0f.m",10*wNF1,10*wDist,10*wAlpha);
			fp = fopen (str,"wt");
			if(fp==NULL) cout<<"Cannot open file .m for writting..."<<endl;
			
			fprintf(fp,"r%.0f%.0f%.0f=[",10*wNF1,10*wDist,10*wAlpha);
			
			KthReal steps = _stepsAdvance; //step length initialized to preset value
			int r; //flag returned by the advanceToBest function
			vector<KthReal> beta_11; //vector of beta values of the solution path
			vector<KthReal> alpha_11; //vector of alpha values of the solution path
			KthReal bestAlpha_11, bestBeta_11; //best alpha, beta values chosen by advanceToBest function 
			double *ppoint;
			vector<double*> path2guide; //
			KthReal currentNF1cost;
			KthReal currentdcost;
			int j=0;
			const int repeattimes=4;
			int repeat=repeattimes;

			//Start moving towards goal	
			_computelookatpointstime=0;
			_evallookatpointstime=0;
			_testlookatpointtime=0;
			_advancecollchecktime=0;
			clock_t advancetobestentertime;
			clock_t advancetobestfinaltime;
			KthReal advancetobesttime=0;
			do{
				//advance from the current configuration towards the best, following NF1 
				//and taking into account the bronchoscope kinematics
				
				advancetobestentertime = clock();

				mt::Transform linkTransf;
				mt::Point3 pos,pos0;
				int linktested = 10;
				linkTransf = _wkSpace->getRobot(0)->getLinkTransform(linktested);
				pos0 = linkTransf.getTranslation();

				r=advanceToBest(steps,&bestAlpha_11,&bestBeta_11,smp,fp);

				advancetobestfinaltime = clock();
				advancetobesttime += (double)(advancetobestfinaltime-advancetobestentertime)/CLOCKS_PER_SEC;
				//if advanced possible
				if(r>0)
				{		

					//store chosen alpha and beta values
					alpha_11.push_back(bestAlpha_11);
					beta_11.push_back(bestBeta_11);	

					/*
					linkTransf = _wkSpace->getRobot(0)->getLinkTransform(linktested);
					pos = linkTransf.getTranslation();
					if(j!=0)
					{
						verifyInvJacobian(linktested,pos[0]-pos0[0],pos[1]-pos0[1],pos[2]-pos0[2],
							bestAlpha, bestBeta, bestAlpha-alpha.at(alpha.size()-2),bestBeta-beta.at(beta.size()-2));
					}
					*/



					//store the chosen sample into the solution path
					_path.push_back(smp);

					//add the sample to the SampleSet
					_samples->add(smp);

					//store the simulation path for visualization purposes. In this planner it is equal to the
					//solution path _path
					_simulationPath.push_back(smp);

					//store the camera path for visualization purposes
					mt::Transform Tcamera = _wkSpace->getRobot(0)->getLinkTransform(_wkSpace->getRobot(0)->getNumLinks()-1)*T_Ry*T_tz;
					addCameraMovement(Tcamera);
					
					//store the paht to write a file to be used by the haptic guiding application
					//each point contains the position and orientation of the base of the bronchoscope and 
					//the alpha and beta values
					ppoint = new double[9];
					ppoint[0] = Tcamera.getTranslation().at(0);
					ppoint[1] = Tcamera.getTranslation().at(1);
					ppoint[2] = Tcamera.getTranslation().at(2);
					ppoint[3] = Tcamera.getRotation().at(0);
					ppoint[4] = Tcamera.getRotation().at(1);
					ppoint[5] = Tcamera.getRotation().at(2);
					ppoint[6] = Tcamera.getRotation().at(3);
					ppoint[7] = bestAlpha_11;
					ppoint[8] = bestBeta_11;
					path2guide.push_back(ppoint);

					//create the next sample
                    smp = new Sample(_wkSpace->getNumRobControls());

					//info for debug purposes:
					collisionCheck(&currentdcost,&currentNF1cost);
					cout<<"sample "<<j<<" steps = "<<steps<<" alpha_11 = "<<bestAlpha_11<<
						" beta_11 = "<< bestBeta_11<<" dist = "<<currentdcost<< " NF1value= "<<currentNF1cost<<endl;
					j++;

					//if the steps used to advance was smaller than the preset value _stepsAdvance, then
					//start recuperating the original value by doubling the step used.
					//also give more weight to NF1 than to Dist. 
					if(steps<_stepsAdvance) 
					{
						steps *=2.0;//restore step value
						//restore weights	
						//wNF1=_weightNF1; 
						//wDist=_weightDist;
						//wAlpha=_weightAlpha;
						//setWeights(wNF1,wDist,wAlpha);
						
					}
					//initialize again repeat counter
					repeat=repeattimes;
				}
				//If advance was not possible then repeat with a smaller step advance and lower the weight of the NF1 function
				//and increment the weight of the Distance.
				
				else
				{
					//if no repeat resulted in success, then reduce steps
					if(repeat==0){
						steps /=2.0;
						repeat=repeattimes;
						
						//use obnly NF1 weight		
						//wNF1=1.0; 
						//wDist=0.0;
						//wAlpha=0.0;
						//setWeights(wNF1,wDist,wAlpha);
					}
					else repeat--;
				}

			//Keep advancing until local minima
			}while(r!=-1 && steps>0.5);


			fprintf(fp,"]\n");
			fclose(fp);

			bool colltest = collisionCheck(&currentdcost,&currentNF1cost);
			cout<<"Reached NF1 value is: "<<currentNF1cost<<endl;

			if(_simulationPath.size()>1) _solved = true;
			else _solved = false;

			if(_solved) 
			{
				string file2guide = _wkSpace->getDirCase() + "HAPTIC/path2guide.xml";
				pparse->savePath2File(file2guide.c_str(), path2guide);
			}
			
			clock_t finaltime = clock();
			cout<<"TIME TO COMPUTE THE PATH = "<<(double)(finaltime-entertime)/CLOCKS_PER_SEC<<endl;
			cout<<"CALLS TO ADVANCE_TO_BEST = "<<_simulationPath.size()<<endl;
			cout<<"TIME TO ADVANCE_TO_BEST = "<<advancetobesttime/_simulationPath.size()<<endl;
			//cout<<"TIME TO COMPUTE LOOKATPOINTS = "<<_computelookatpointstime/_simulationPath.size()<<endl;
			//cout<<"TIME TO EVAL LOOKATPOINTS (TEST AND ORDER) = "<<_evallookatpointstime/_simulationPath.size()<<endl;
			//cout<<"TIME TO TEST LOOKATPOINTS = "<<_testlookatpointtime/_simulationPath.size()<<endl;
			//cout<<"TIME TO ADVANCE-COLLCHECK = "<<_advancecollchecktime/_simulationPath.size()<<endl;
			

			return _solved;



/* THe following code tests the restoration of a configuration
			mt::Transform linkTransf;
			mt::Point3 pos;

			_wkSpace->moveRobotsTo(_goal);
			
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

