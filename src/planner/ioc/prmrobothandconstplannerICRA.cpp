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

/* Author: Alexander Perez, Jan Rosell, Nestor Garcia Hidalgo */


//FIXME: this planner is done for a single TREE robot (associtated to wkSpace->robots[0])


#include <stdio.h>
#include <time.h>

#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotation.h>

#include <kautham/problem/ivpqpelement.h>
#include <kautham/planner/ioc/prmrobothandconstplannerICRA.h>
#include <kautham/planner/ioc/constlinearlocplan.h>

 
 namespace Kautham {
  namespace IOC{
		
  PRMRobotHandConstPlannerICRA::PRMRobotHandConstPlannerICRA(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler,
           WorkSpace *ws, int cloudSize, KthReal cloudRad)
      :PRMHandPlanner(stype, init, goal, samples, sampler, ws,  cloudSize,  cloudRad)
	{
    _idName = "PRM RobotHand-Const ICRA";
    _sampletimeok = 0;
    _sampletimeko = 0;
    _connecttime = 0;
    //_samples->setTypeSearch(BRUTEFORCE);//ANNMETHOD is the default

    setCameraMovements(true);
    mt::Point3 mtCamTra(140,0,0);//150
    mt::Unit3 mtCamRot(0,1,0);
    float angle = -M_PI/3;

    //The camera is located at the TCP with the orientation mtCamRot
    //The value of the translation mtCamTra is used to compute the staring point in function computeStaringPoint.
    _cameraTransform.setTranslation(mtCamTra); 
    _cameraTransform.setRotation(mt::Rotation(mtCamRot,angle));

    //_numberSamples = numSam;
    //addParameter("Number Samples", _numberSamples);


    //change the linear loc plan defined in class prm with the constrained one
    //default ssize = 0.05
    KthReal ssize = 0.05;
    addParameter("Step Size", ssize);
    _locPlanner =  new ConstLinearLocalPlanner(CONTROLSPACE, NULL,NULL, _wkSpace, ssize);

    _constrained = 1; //0:no constaints, 1:spherical constraints; 2: nothing yet
    addParameter("Consider Constraints",_constrained);
    if(_constrained==0) ((ConstLinearLocalPlanner*)_locPlanner)->setConstrained(false);
    else ((ConstLinearLocalPlanner*)_locPlanner)->setConstrained(true);

    _planeHeight = 500.0;
    addParameter("Plane Height",_planeHeight);


	((ConstLinearLocalPlanner*) _locPlanner)->setCameraTransform(_cameraTransform);
	  
	  //Add the stick that simulates the line of sight along the x axis of the tcp
	static const char *kg_str[] = {
          "#Inventor V2.1 ascii\n",
          "DEF root Separator {\n",
          "  Separator {\n",
          "    Transform {\n",
          "      translation 700 0 0\n",
          "      rotation 0 0 1  1.570796\n",
          "      scaleFactor 1 1 1\n",
          "      scaleOrientation 0 0 1  0\n",
          "      center 0 0 0\n",
          "    }\n",
          "    Material {\n",
          "      diffuseColor 1 0.30000001 0.30000001\n",
          "      transparency 0.0\n",
          "    }\n",
          "    Cylinder {\n",
          "      radius 0.5\n",
          "      height 1400\n",
          "    }\n",
          "    Translation {\n",
          "      translation 0 560 0\n",
          "    }\n",
          "    Material {\n",
          "      diffuseColor 0.3 1.0 0.30000001\n",
          "      transparency 0.0\n",
          "    }\n",
          "    Sphere {\n",
          "      radius 1.0\n",
          "    }\n",
          "  }\n",
          "}\n",
          NULL
      };

      Robot* tmpRob = this->_wkSpace->getRobot(0);
      Link* tmpLink = tmpRob->getLink(7);
      SoSeparator* tmpModel = ((IVPQPElement*)tmpLink->getElement())->ivModel(false);

      SoInput in;
      in.setStringArray(kg_str);
      //SoTranslation *camTran = new SoTranslation();
      SoRotation *camRot = new SoRotation();
      /*camTran->translation.setValue(_cameraTransform.getTranslation().at(0),
                                   _cameraTransform.getTranslation().at(1),
                                   _cameraTransform.getTranslation().at(2));
	  */
      SbVec3f *soCamRot = new SbVec3f();
      soCamRot->setValue(mtCamRot.at(0), mtCamRot.at(1), mtCamRot.at(2) );
      camRot->rotation.setValue(*soCamRot,angle);

      //tmpModel->addChild(camTran);
      tmpModel->addChild(camRot);
      SoSeparator *sep = SoDB::readAll(&in);
      sep->ref();
      while (sep->getNumChildren() > 0)
      {
         tmpModel->addChild(sep->getChild(0));
         sep->removeChild(0);
      }
      sep->unref();
	  //END: Add the stick that simulates the line of sight along the x axis of the tcp

	}


  PRMRobotHandConstPlannerICRA::~PRMRobotHandConstPlannerICRA(){
			
	}

    bool PRMRobotHandConstPlannerICRA::setParameters()
	{
      PRMHandPlanner::setParameters();
      try{
       
		/*
        HASH_S_K::iterator it = _parameters.find("Number Samples");
        if(it != _parameters.end())
          _numberSamples = it->second;
        else
          return false; 
		*/
		HASH_S_K::iterator it = _parameters.find("Consider Constraints");
        if(it != _parameters.end())
		{
			_constrained = it->second;
			if(_constrained==0) ((ConstLinearLocalPlanner*)_locPlanner)->setConstrained(false);
			else ((ConstLinearLocalPlanner*)_locPlanner)->setConstrained(true);

    }
        else if(it != _parameters.end())
        {
          _planeHeight = it->second;
        }
        else
          return false;

      }catch(...){
        return false;
      }
      return true;
    }



  bool PRMRobotHandConstPlannerICRA::trySolve()
	{	 
		
		//compute the tranform of the tcp of the robot at the goal configuration
		_wkSpace->getRobot(0)->Kinematics(goalSamp()->getMappedConf().at(0).getRn()); 
		std::vector<KthReal> tmpcoordTCP7; tmpcoordTCP7.resize(7); 
		mt::Transform goaltransf = _wkSpace->getRobot(0)->getLinkTransform(6);
		mt::Point3 goaltrans = goaltransf.getTranslation();
		mt::Rotation goalrot = goaltransf.getRotation();
		for(int k=0;k<3;k++) tmpcoordTCP7[k] =  goaltrans[k];
		for(int k=0;k<4;k++) tmpcoordTCP7[3+k] =  goalrot[k];

		//compute the staring point(ox,oy,oz)
		computeStaringPoint(tmpcoordTCP7[0],tmpcoordTCP7[1],tmpcoordTCP7[2],
							tmpcoordTCP7[3],tmpcoordTCP7[4],tmpcoordTCP7[5],tmpcoordTCP7[6]);
	

		if(_samples->changed())
		{
			PRMPlanner::clearGraph(); //also puts _solved to false;
		}

		//if(_solved) {
		//	cout << "PATH ALREADY SOLVED"<<endl;
		//	return true;
		//}
		
		cout<<"ENTERING TRYSOLVE!!!"<<endl;
	    clock_t entertime = clock();
		

      std::vector<KthReal> coord; coord.resize(_wkSpace->getNumRobControls());
      std::vector<KthReal> coordarm; coordarm.resize(6);

	
		//Create graph with initial and goal sample
		if( !_isGraphSet )
		{
			_samples->findBFNeighs(_neighThress, _kNeighs);//BF???? maybe ANN gives problems here...8/2010
			connectSamples();
			PRMPlanner::loadGraph();
			if(PRMPlanner::findPath()) return true;
		}

		//Randomly set the coordinates of the hand joints at a autocollision-free conf
		int found=0;
		do{
			if(_gen->d_rand()<0.6){//sample around goal
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp()) == true) {
						clock_t itime = clock();
						PRMPlanner::connectLastSample( goalSamp() );
						clock_t etime = clock();
						_connecttime += (double)(etime-itime)/CLOCKS_PER_SEC;
					}
				}
			}
			else{//sample along path between ini and goal
				double step = _gen->d_rand();
				double r = 1+4*_gen->d_rand();//random value between 1 and 5
				Sample *smp;
				Sample *smpConnect;
				smp = initSamp()->interpolate(goalSamp(),(KthReal)step);
				if(step<0.5) smpConnect = initSamp();
				else smpConnect = goalSamp();
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(smp, r*_cloudRadius) == true) {
						clock_t itime = clock();
						PRMPlanner::connectLastSample( smpConnect );
						clock_t etime = clock();
						_connecttime += (double)(etime-itime)/CLOCKS_PER_SEC;
					}
				}
			}

			/*
	
			double r = _gen->d_rand();
			if(r<0.4){
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp()) == true) PRMPlanner::connectLastSample( goalSamp() );
				}
			}
			else if(r<0.5) {
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp(), 2*_cloudRadius ) == true) PRMPlanner::connectLastSample( goalSamp() );
				}
			}
			else if(r<0.6) {
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp(), 3*_cloudRadius) == true) PRMPlanner::connectLastSample( goalSamp() );
				}
			}
			else if(r<0.7) {
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(initSamp(), 4*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp(), 4*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
			}
			else if(r<0.8) {	
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(initSamp(), 3*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp(), 5*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
			}
			else if(r<0.9) {
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(initSamp(), 2*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp(), 6*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
			}
			else if(r<0.10) {
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(initSamp(), _cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
				for(int h=0;h<_cloudSize;h++){
					if(getSampleInRegion(goalSamp(), 7*_cloudRadius) == true) PRMPlanner::connectLastSample( initSamp() );
				}
			}
			*/

			if(goalSamp()->getConnectedComponent() == initSamp()->getConnectedComponent()) 
			{
				found = 1; //break do-while
			}

        }while(_samples->getSize() < _maxNumSamples && found==0);

		if(found==1 && PRMPlanner::findPath())
		{
			clock_t finaltime = clock();
			PRMPlanner::smoothPath(true, false);//maintain the first edge - do not smooth
			clock_t finalsmoothtime = clock();

			cout << "PRM Free Nodes = " << _samples->getSize() << endl;
			printConnectedComponents();
				
			cout<<"TIME TO COMPUTE THE PATH = "<<(double)(finaltime-entertime)/CLOCKS_PER_SEC<<endl;
			cout<<"TIME TO SMOOTH THE PATH = "<<(double)(finalsmoothtime - finaltime)/CLOCKS_PER_SEC<<endl;
			cout<<"SAMPLING TIME (ok) = "<<_sampletimeok<<endl;
			cout<<"SAMPLING TIME (ko) = "<<_sampletimeko<<endl;
			cout<<"CONNECTING TIME  = "<<_connecttime<<endl;
				
			return true;
		}
	  	  

		cout << "PRM Free Nodes = " << _samples->getSize() << endl;
		cout<<"PATH NOT FOUND"<<endl;
		printConnectedComponents();
	  
		clock_t finaltime = clock();
		cout<<"ELAPSED TOTAL TIME = "<<(double)(finaltime-entertime)/CLOCKS_PER_SEC<<endl;
			cout<<"SAMPLING TIME (ok) = "<<_sampletimeok<<endl;
			cout<<"SAMPLING TIME (ko) = "<<_sampletimeko<<endl;
			cout<<"CONNECTING TIME  = "<<_connecttime<<endl;
		return false;
    }

 

	
  //!Given the goal configuration, computes the point of a plane parallel to the world plane x-z to where the x-axis of the
	//!palm is pointing to
  void PRMRobotHandConstPlannerICRA::computeStaringPoint(KthReal px,KthReal py, KthReal pz, KthReal q1, KthReal q2, KthReal q3, KthReal q4)
	{
	//Compute the staring point as the point inside the hand workspace. It is located at a distance of 100mm form the 
		//origin of the tcp and following the x-axis of the camera.

		//retrive the rotation R
            vector<KthReal> tmp(7);
            tmp.at(0) = px; tmp.at(1) = py; tmp.at(2) = pz;
            tmp.at(3) = q1; tmp.at(4) = q2; tmp.at(5) = q3; tmp.at(6) = q4;
            mt::Rotation R(tmp.at(3), tmp.at(4), tmp.at(5), tmp.at(6));

		//change from TCP to camera
			mt::Rotation RC = _cameraTransform.getRotation();
			R = R*RC; //rotation of camera
			
			mt::Point3 P;
			P = R(_cameraTransform.getTranslation());
			KthReal ox = px + P[0];
			KthReal oy = py + P[1];
			KthReal oz = pz + P[2];
			((ConstLinearLocalPlanner*) _locPlanner)->setStaringPoint(ox,oy,oz);

		
		//WHAT FOLLOWS IS THE COMPUTATION OF THE INTERSECTION POINT OF THE x-axis 
		//of the camera with the plane paralele to the xy plane and witht the heihght fixed by _planeHeight
		//It is the way the staring point used to be computed...
/*
		//retrive the rotation R
            vector<KthReal> tmp(7);
            tmp.at(0) = px; tmp.at(1) = py; tmp.at(2) = pz;
            tmp.at(3) = q1; tmp.at(4) = q2; tmp.at(5) = q3; tmp.at(6) = q4;
            mt::Rotation R(tmp.at(3), tmp.at(4), tmp.at(5), tmp.at(6));

		//change from TCP to camera
			mt::Rotation RC = _cameraTransform.getRotation();
			R = R*RC; //rotation of camera
		


			//hand coordinate system
			mt::Vector3 vx(1.0, 0.0, 0.0);
			vx = R(vx);  
			mt::Vector3 vy(0.0, 1.0, 0.0);
			vy = R(vy);  
			mt::Vector3 vz(0.0, 0.0, 1.0);
			vz = R(vz);  

			//vx-vy plane
			KthReal A0=vz[0];
			KthReal B0=vz[1];
			KthReal C0=vz[2];
			KthReal D0=-A0*px-B0*py-C0*pz;
			
			//vx-vz plane
			KthReal A1=vy[0];
			KthReal B1=vy[1];
			KthReal C1=vy[2];
			KthReal D1=-A1*px-B1*py-C1*pz;

			//plane parallel to the x-y plane
			KthReal A2=0.0;
			KthReal B2=0.0;
			KthReal C2=1.0;
			KthReal D2= - _planeHeight;

			//Staring point is the intersection of the three planes
			KthReal delta = mt::determinant(mt::Matrix3x3(A0, B0, C0, A1, B1, C1, A2, B2, C2));
			KthReal deltaX = mt::determinant(mt::Matrix3x3(-D0, B0, C0, -D1, B1, C1, -D2, B2, C2));
			KthReal deltaY = mt::determinant(mt::Matrix3x3(A0, -D0, C0, A1, -D1, C1, A2, -D2, C2));
			KthReal deltaZ = mt::determinant(mt::Matrix3x3(A0, B0, -D0, A1, B1, -D1, A2, B2, -D2));

			KthReal ox = deltaX/delta;
			KthReal oy = deltaY/delta;
			KthReal oz = deltaZ/delta;
			((ConstLinearLocalPlanner*) _locPlanner)->setStaringPoint(ox,oy,oz);
			*/
	}

	 
    void PRMRobotHandConstPlannerICRA::saveData()
	  {
	  
		////////////
		//PRINT INFO CURRENT CONF

		  mt::Transform TCPtransf = _wkSpace->getRobot(0)->getLinkTransform(6);
			  mt::Point3 TCPtrans = TCPtransf.getTranslation();
			  mt::Rotation TCProt = TCPtransf.getRotation();
			  mt::Scalar rx,ry,rz;
			  TCProt.getYpr(rz,ry,rx);

			  cout << "TOOL TRSF:"<<endl;
			  cout << "TCP position = ("<<TCPtrans[0]<< ", "<<TCPtrans[1]<< ", "<<TCPtrans[2]<< ")"<<endl;
			  cout << "TCP orientation = ("<<rx*180/M_PI<< ", "<<ry*180/M_PI<< ", "<<rz*180/M_PI<< ")"<<endl;


			  RobConf *jnts = _wkSpace->getRobot(0)->getCurrentPos();
			  cout << "joints = (";
			  for(int j =0; j < 6; j++)
				cout << (180.0+jnts->getRn().getCoordinate(j)*180.0/PI)/360.0<< " ";
			  cout << ")"<<endl;

			  //find the configuration parameters
			  RnConf rc;
			  int conf[3];
			  //defs taken from <hardlab/robot/txrobot.h>
				enum shoulder{righty=0, lefty, sfree};
				enum elbow{epositive=0, enegative, efree};
				//enum wrist{wpositive=1, wnegative=-1, wfree};
				enum wrist{wpositive=0, wnegative, wfree};
				//
			  rc = _wkSpace->getRobot(0)->getCurrentPos()->getRn();
			  KthReal ifRig = 425*sin(rc.getCoordinate(1))
                          + 425*sin(rc.getCoordinate(1) + rc.getCoordinate(2))
                          + 50;
				if(ifRig >= 0.) //Shoulder Lefty
					 conf[0] = lefty;
				 else
					conf[0] = righty;

				if(rc.getCoordinate(2) >= 0.) //Elbow Positive
					conf[1] = epositive;
				else
					conf[1] = enegative;

				if(rc.getCoordinate(4) >= 0.) //Wrist Positive
					conf[2] = wpositive;
				else
					conf[2] = wnegative;
			  cout << "lefty/righty = "<<conf[0]<< ", epos/eneg = "<<conf[1]<< ", wpos/wneg = "<<conf[2]<< endl;

		////////////
		//STORE DATA
		 
		cout << "-----------------------------------------------------------"<<endl<<flush;
		cout << "     SAVING SOLUTION DATA									"<<endl<<flush;
		cout << "-----------------------------------------------------------"<<endl<<flush;
		
		if(_path.size() != 0 )
		{
			//open files for writting simulated solution path
			RobConf* joints;
			FILE *fpr,*fph;
			fpr = fopen ("robotconf.txt","wt");
			if(fpr==NULL) cout<<"Cannot open robotconf.txt for writting..."<<endl;
			fph = fopen ("handconf.txt","wt");
			if(fph==NULL) cout<<"Cannot open handconf.txt for writting..."<<endl;
		
			//write to file
            for(unsigned i=0; i<_path.size(); i++){
				_wkSpace->moveRobotsTo(_path[i]); //to load the config because it may be lost in computeorientation applied after interpolate in moveAlongPath
				joints = &(_path[i]->getMappedConf().at(0));
				writeFiles(fpr,fph,joints);
			}
			fclose(fpr);
			fclose(fph);
		}

	    //write samples with neighs and distances
		_samples->writeSamples();

		/*
		  cout << "DELETING PRM"<<endl<<flush;
		  PRMPlanner::clearGraph();
		  _solved=false;
		  */
		  
	}

//    void PRMRobotHandConstPlannerICRA::setIniGoal() {
//      if(_wkSpace->getNumRobControls()==11)
//      {
//		  //SET GOAL CONFIGURATION
//        std::vector<KthReal> c(11);

//		/* for solutions 1 to 3
//        c[0] = (KthReal)0.803;//0.303;
//        c[1] = (KthReal)0.405;
//        c[2] = (KthReal)0.262;
//        c[3] = (KthReal)0.5;
//        c[4] = (KthReal)0.6;
//        c[5] = (KthReal)0.109;//
//		*/
//        c[0] = (KthReal)0.805;//0.806;//0.303;
//        c[1] = (KthReal)0.395;//0.399833333;
//        c[2] = (KthReal)0.262;//0.260138889;
//        c[3] = (KthReal)0.5;//0.502944444;
//        c[4] = (KthReal)0.595;//0.594;
//        c[5] = (KthReal)0.236;//0.237166667;
//        c[6] = (KthReal)0.559;
//        c[7] = (KthReal)0.523;
//        c[8] = (KthReal)0.728;
//        c[9] = (KthReal)0.821;
//        c[10] = (KthReal)0.928;//0.169;
		
//        _goal = new Sample(_wkSpace->getNumRobControls());

//        goalSamp()->setCoords(c);
//        _samples->add(_goal);
//       if( _wkSpace->collisionCheck(goalSamp()))
//       {
//        cout<<"BE CAREFUL GOAL SAMPLE NOT FREE"<<endl;
//       }
//	  //END SET GOAL CONFIGURATION

//	   //COMPUTE STARING POINT
//	   	//compute the tranform of the tcp of the robot at the goal configuration
//		_wkSpace->getRobot(0)->Kinematics(goalSamp()->getMappedConf().at(0).getRn());
//		std::vector<KthReal> tmpcoordTCP7; tmpcoordTCP7.resize(7);
//		mt::Transform goaltransf = _wkSpace->getRobot(0)->getLinkTransform(6);
//		mt::Point3 goaltrans = goaltransf.getTranslation();
//		mt::Rotation goalrot = goaltransf.getRotation();
//		for(int k=0;k<3;k++) tmpcoordTCP7[k] =  goaltrans[k];
//		for(int k=0;k<4;k++) tmpcoordTCP7[3+k] =  goalrot[k];

//		//compute the staring point(ox,oy,oz)
//		computeStaringPoint(tmpcoordTCP7[0],tmpcoordTCP7[1],tmpcoordTCP7[2],
//							tmpcoordTCP7[3],tmpcoordTCP7[4],tmpcoordTCP7[5],tmpcoordTCP7[6]);
//	   //END COMPUTE STARING POINT


//		  //SET INIT CONFIGURATION
//        c[0] = 0.71;//0.762;//0.71;//0.21;
//        c[1] = 0.506;
//        c[2] = 0.150;//0.178;//0.262;
//        c[3] = 0.418;//0.5;
//        c[4] = 0.584;//0.208;//0.508;
//        c[5] = 0.177;
//        c[6] = 0.251;
//        c[7] = 0.036;
//        c[8] = 0.226;
//        c[9] = 0.518;
//        c[10] = 0.21;
		
		
//		//same as goal, to verify problems
//		/*
//        c[0] = (KthReal)0.803;//0.303;
//        c[1] = (KthReal)0.405;
//        c[2] = (KthReal)0.262;
//        c[3] = (KthReal)0.5;
//        c[4] = (KthReal)0.6;
//        c[5] = (KthReal)0.109;//
//        c[6] = (KthReal)0.559;
//        c[7] = (KthReal)0.523;
//        c[8] = (KthReal)0.728;
//        c[9] = (KthReal)0.821;
//        c[10] = (KthReal)0.169;
//		*/

//        _init = new Sample(_wkSpace->getNumRobControls());

//        initSamp()->setCoords(c);
//		_wkSpace->collisionCheck(initSamp()); //to load config

//		//MODIFY INIT CONFIG TO SATISFY CONSTRAINTS
//		KthReal low[6];
//		KthReal high[6];
//        std::vector<KthReal> coord(6);
//		for(int k = 0; k < 6; k++)
//		{
//			low[k] = *_wkSpace->getRobot(0)->getLink(k+1)->getLimits(true);
//			high[k] = *_wkSpace->getRobot(0)->getLink(k+1)->getLimits(false);
//		}
//		//denormalize the joint values
//		for(int k = 0; k < 6; k++)
//		{
//				coord[k] = low[k] + c[k]*(high[k] - low[k]);
//		}
//		//compute the position of the tcp (rewriting coordarm vector)
//		RnConf armconf(6);
//		armconf.setCoordinates(coord);
//		_wkSpace->getRobot(0)->Kinematics(armconf);
//		mt::Transform ctransform = _wkSpace->getRobot(0)->getLinkTransform(6);
//		mt::Point3 ctrans = ctransform.getTranslation();
//		for(int k=0;k<3;k++) coord[k] =  ctrans[k];

//		//compute the orientation that satisfies the constraints
//		KthReal thetaini = 0.0;
//		((ConstLinearLocalPlanner*) _locPlanner)->computeorientation(coord[0],coord[1],coord[2],&coord[3],&coord[4],&coord[5], thetaini);

//		//CALL TO INVERSE KINEMATICS
//		//coord sends se3 coordinates and is reloaded with joint values
//		//c then stores the normalized values
//		//The configuration desired is that of the initSamp before correcting the orientation
//		if(((ConstLinearLocalPlanner*)_locPlanner)->ArmInverseKinematics(coord, initSamp(), true)==true) {
//			for(int k = 0; k < 6; k++) c[k]=(coord[k]-low[k])/(high[k]-low[k]);
//		}
//		//END MODIFY INIT CONFIG TO SATISFY CONSTRAINTS


//        initSamp()->setCoords(c);
//        _samples->add(_init);
//        if( _wkSpace->collisionCheck(initSamp()))
//        {
//          cout<<"BE CAREFUL INIT SAMPLE NOT FREE"<<endl;
//        }
//		//END SET INIT CONFIGURATION
//      }
//	  else
//	  {
//        std::vector<KthReal> c(6);
//        c[0] = (KthReal)0.803;//0.303;
//        c[1] = (KthReal)0.405;
//        c[2] = (KthReal)0.262;
//        c[3] = (KthReal)0.5;
//        c[4] = (KthReal)0.6;
//        c[5] = (KthReal)0.109;
//        _goal = new Sample(_wkSpace->getNumRobControls());
//        goalSamp()->setCoords(c);
//        _samples->add(_goal);
//       if( _wkSpace->collisionCheck(goalSamp()))
//       {
//        cout<<"BE CAREFUL GOAL SAMPLE NOT FREE"<<endl;
//       }

//        c[0] = 0.71;//0.21;
//        c[1] = 0.503;
//        c[2] = 0.262;
//        c[3] = 0.5;
//        c[4] = 0.508;
//        c[5] = 0.251;
//        _init = new Sample(_wkSpace->getNumRobControls());
//        initSamp()->setCoords(c);
//        _samples->add(_init);
//        if( _wkSpace->collisionCheck(initSamp()))
//        {
//          cout<<"BE CAREFUL INIT SAMPLE NOT FREE"<<endl;
//        }
//	  }
//    }

	
 //!resample around the goal configuration
//    bool PRMRobotHandConstPlannerICRA::getSampleAround(vector<KthReal> tcp, KthReal R)
//	{
//
//	}

//!resample around the a configuration smp
    bool PRMRobotHandConstPlannerICRA::getSampleInRegion(Sample *smp, double R)
	{
		//compute random arm configurations only every cloudsize times
		static int count = 0;
        static std::vector<KthReal> coord(_wkSpace->getNumRobControls());
		static bool validarmconf=false;
		static KthReal rd[6];
		static KthReal rdtheta;


		clock_t initime = clock();


		bool invKinSolved=false;
		if(count == _cloudSize) count = 0;
		if(count == 0){
			rd[0] = (KthReal)_gen->d_rand();
			rd[1] = (KthReal)_gen->d_rand();
			rd[2] = (KthReal)_gen->d_rand();
			rd[3] = (KthReal)_gen->d_rand();
			rd[4] = (KthReal)_gen->d_rand();
			rd[5] = (KthReal)_gen->d_rand();
			rdtheta = (KthReal)_gen->d_rand();
			validarmconf = false;
		}
		count++;

		double radius;
		if(R==0) radius = _cloudRadius;  
		else radius = R;

		KthReal thetaradius;
	    int trials, maxtrials;
		bool autocol = true;//flag to test autocollisions
        Sample *tmpSample;
        tmpSample = new Sample(_wkSpace->getNumRobControls());


		//compute the xyz of the tcp of the robot at the smp configuration
		  _wkSpace->getRobot(0)->Kinematics(smp->getMappedConf().at(0).getRn()); 
		  std::vector<KthReal> tmpcoordTCP7; tmpcoordTCP7.resize(7); 
		  mt::Transform goaltransf = _wkSpace->getRobot(0)->getLinkTransform(6);
		  mt::Point3 goaltrans = goaltransf.getTranslation();
		  mt::Rotation goalrot = goaltransf.getRotation();
		  for(int k=0;k<3;k++) tmpcoordTCP7[k] =  goaltrans[k];
		  for(int k=0;k<4;k++) tmpcoordTCP7[k+3] =  goalrot[k];

		
		trials=0;
		maxtrials=100;
		do{
			if(validarmconf==false)//first time: coord[0..5] must be set
			{
				
				//Set the coordinates of the robot joints 
				//Randomly set the coordinates of the robot joints at a autocollision-free conf
				//random position around smp position
				for(int k = 0; k < 3; k++) coord[k] = tmpcoordTCP7[k] + radius*(2*rd[k]-1);				
				if(_constrained){
					//SIMILAR theta as smp configuration
					if(smp == goalSamp()) thetaradius = PI/4;
					else thetaradius = PI/2;
					KthReal thetagoal = ((ConstLinearLocalPlanner*) _locPlanner)->getTheta(tmpcoordTCP7);
					thetagoal = thetagoal + thetaradius*(2*rdtheta-1);  
					((ConstLinearLocalPlanner*) _locPlanner)->computeorientation(coord[0],coord[1],coord[2],&coord[3],&coord[4],&coord[5],thetagoal);
				}
				else{
					for(int k = 3; k < 6; k++) coord[k] = rd[k];	
				}

				//CALL TO INVERSE KINEMATICS for the arm
				//coord sends se3 coordinates and is reloaded with joint values
				bool maintainSameWrist;
				//if(_gen->d_rand() < 0.5) maintainSameWrist=true;
				//else maintainSameWrist = false;
				maintainSameWrist=true;
				invKinSolved=((ConstLinearLocalPlanner*)_locPlanner)->ArmInverseKinematics(coord, goalSamp(), maintainSameWrist);
			
				if(invKinSolved==true) 
				{
					//load arm coordinates (normalized)
					KthReal low[6];
					KthReal high[6];
					for(int k=0; k < 6; k++)
					{
						//normalize
						low[k] = *_wkSpace->getRobot(0)->getLink(k+1)->getLimits(true);
						high[k] = *_wkSpace->getRobot(0)->getLink(k+1)->getLimits(false);
						coord[k]=(coord[k]-low[k])/(high[k]-low[k]);
					}

					//Set the new sample with the arm coorinates and check for autocollision.	
                    for(unsigned k=6; k < _wkSpace->getNumRobControls(); k++)	coord[k]=0.0;//dummmy
					_wkSpace->getRobot(0)->control2Pose(coord); 
					autocol = _wkSpace->getRobot(0)->autocollision(1);//test for the trunk
					if(autocol==true) {
						rd[0] = (KthReal)_gen->d_rand();
						rd[1] = (KthReal)_gen->d_rand();
						rd[2] = (KthReal)_gen->d_rand();
						rd[3] = (KthReal)_gen->d_rand();
						rd[4] = (KthReal)_gen->d_rand();
						rd[5] = (KthReal)_gen->d_rand();
						rdtheta = (KthReal)_gen->d_rand();
						invKinSolved=false;
						continue; //try another arm configuration...
					}
					
					//if found, compute hand coordinates 
				    //SAMPLE HAND
					if(count==0){//a safe configuration ("home")
						coord[0] = 0.5;
						coord[1] = 0.5;
						coord[2] = 0.78;
						coord[3] = 0.47;
						coord[4] = 0.5;
					}
					else{//rand values
						int trialshand=0;
						int maxtrialshand=100;
						do{
                            for(unsigned k=6; k < _wkSpace->getNumRobControls(); k++)
							{
								coord[k] = (KthReal)_gen->d_rand();
							}
							//Set the new sample with the hand-arm coorinates and check for autocollision.				
							_wkSpace->getRobot(0)->control2Pose(coord); 
							autocol = _wkSpace->getRobot(0)->autocollision();
							trialshand++;
						}while(autocol==true && trialshand<maxtrialshand);
					}
				}
				trials++;
			}
			else{ //coord[0..5] already valid
				//SAMPLE HAND
				int trialshand=0;
				int maxtrialshand=100;
				do{
                    for(unsigned k=6; k < _wkSpace->getNumRobControls(); k++)
					{
						coord[k] = (KthReal)_gen->d_rand();
					}
					//Set the new sample with the hand-arm coorinates and check for autocollision.				
					_wkSpace->getRobot(0)->control2Pose(coord); 
					autocol = _wkSpace->getRobot(0)->autocollision();
					trialshand++;
				}while(autocol==true && trialshand<maxtrialshand);
			}


		}while(autocol==true && trials<maxtrials);

		//not valid configuration - an autocollision of the hand-arm system exisits
		if(autocol==true) {
			validarmconf = false;
			clock_t finaltime = clock();
			_sampletimeko += (double)(finaltime-initime)/CLOCKS_PER_SEC;
			return false;
		}

		//valid autocollision free hand-arm configuration. 
		//Set the new sample with the hand-arm coorinates and collision-check.
		tmpSample->setCoords(coord);
		if( !_wkSpace->collisionCheck(tmpSample)){ 
			//Free sample. Add to the sampleset _samples.
			if(_samples->getSize()< _maxNumSamples) _samples->add(tmpSample);
			validarmconf = true; //to be uesd for further calls to getSampleInRegion
			clock_t finaltime = clock();
			_sampletimeok += (double)(finaltime-initime)/CLOCKS_PER_SEC;
			return true;
		 }		
		else {
			//not valid configuration - a collision with the environment exisits
			validarmconf = false;
			clock_t finaltime = clock();
			_sampletimeko += (double)(finaltime-initime)/CLOCKS_PER_SEC;
			return false;
		}
	}

    //solveAndInherit is reimplemented from Planner, in order to use the correct
    //version of the moveAlongPath function
    bool PRMRobotHandConstPlannerICRA::solveAndInherit(){
      if(trySolve()){
        moveAlongPath(0); //reimplemented by PRMHandConstPlannerICRA
        _wkSpace->inheritSolution(_simulationPath);
        return true;
      }
      return false;
    }

    void PRMRobotHandConstPlannerICRA::writeFiles(FILE *fpr, FILE *fph, RobConf* joints)
	{
		//write arm coordinates
        unsigned j;
		for(j =0; j < 6; j++)
      fprintf(fpr,"%.2f ",joints->getRn().getCoordinate(j)*180.0/PI);
		fprintf(fpr,"\n");

		//write hand coordinates
    for(; j < joints->getRn().getDim(); j++)
		{
			if(j==6 || j==11 || j==15 || j==19 || j==23) continue;
      fprintf(fph,"%.2f ",joints->getRn().getCoordinate(j)*180.0/PI);
		}
		fprintf(fph,"\n");
	}

  void PRMRobotHandConstPlannerICRA::moveAlongPath(unsigned int step)
	{
      if(_solved)
      {
        if(_simulationPath.size() == 0 ) // Then calculate the simulation path based on stepsize
        {
      clearCameraMovement();
			mt::Unit3 yaxis(0,1,0);
			mt::Unit3 zaxis(0,0,1);
			mt::Scalar angle = -M_PI/2;
			mt::Rotation Ry(yaxis, angle);
			mt::Rotation Rz(zaxis, angle);
			mt::Rotation RyRz(Ry*Rz);
      mt::Transform TRyRz = mt::Transform(RyRz,mt::Point3(50.,0,0));
	  mt::Transform Trotcam = mt::Transform(_cameraTransform.getRotation(),mt::Point3(0,0,0));
      TRyRz = Trotcam * TRyRz;

          unsigned int maxsteps = 0;
          Sample* tmpSam;
          KthReal dist = (KthReal)0.0;
          if(_path.size() >=2 ){
            for(unsigned int i = 0; i < _path.size()-1; i++) {
           
              dist = wkSpace()->distanceBetweenSamples(*_path.at(i), *_path.at(i+1), Kautham::CONFIGSPACE);
              maxsteps = (dist/_locPlanner->stepSize()) + 2; //the 2 is necessary to always reduce the distance...???
			      
              _wkSpace->getRobot(0)->Kinematics(_path[i]->getMappedConf().at(0).getRn());
              std::vector<KthReal> tmpcoordTCP7; tmpcoordTCP7.resize(7);
              mt::Transform ctransfini = _wkSpace->getRobot(0)->getLinkTransform(6);
              mt::Point3 ctransini = ctransfini.getTranslation();
              mt::Rotation crotini = ctransfini.getRotation();
              for(int k=0;k<3;k++) tmpcoordTCP7[k] =  ctransini[k];
              for(int k=0;k<4;k++) tmpcoordTCP7[3+k] =  crotini[k];

              KthReal thetaini = ((ConstLinearLocalPlanner*) _locPlanner)->getTheta(tmpcoordTCP7);

              _wkSpace->getRobot(0)->Kinematics(_path[i+1]->getMappedConf().at(0).getRn());
              mt::Transform ctransfgoal = _wkSpace->getRobot(0)->getLinkTransform(6);
              mt::Point3 ctransgoal = ctransfgoal.getTranslation();
              mt::Rotation crotgoal = ctransfgoal.getRotation();
              for(int k=0;k<3;k++) tmpcoordTCP7[k] =  ctransgoal[k];
              for(int k=0;k<4;k++) tmpcoordTCP7[3+k] =  crotgoal[k];

              KthReal thetagoal = ((ConstLinearLocalPlanner*) _locPlanner)->getTheta(tmpcoordTCP7);
              KthReal stepsTheta = (thetagoal - thetaini)/maxsteps;


              for(unsigned int j = 0; j <= maxsteps; j++)
              {
                tmpSam = _path[i]->interpolate(_path[i+1],(KthReal)j/(KthReal)maxsteps);
                if(_constrained)
				{
					KthReal theta = thetaini + j*stepsTheta;
					if(((ConstLinearLocalPlanner*)_locPlanner)->correctorientation(tmpSam, theta, goalSamp())==true) _simulationPath.push_back(tmpSam);
				}
                else _simulationPath.push_back(tmpSam);

				//store camera transform
				_wkSpace->moveRobotsTo(tmpSam);
				mt::Transform ctransf = _wkSpace->getRobot(0)->getLinkTransform(6);
        mt::Point3 ctrans = ctransf.getTranslation();
        mt::Rotation crot = ctransf.getRotation();
        //addCameraMovement(ctransf*TRyRz);



              }//closes j-for loop
            }//closes i-for loop
          }//closes if _path.size() >=2
       }//closes if _simulationPath.size() == 0
       else{//simulation_path already calculated
			  if( _simulationPath.size() >= 2 ){
		        step = step % _simulationPath.size();
				_wkSpace->moveRobotsTo(_simulationPath[step]);
			  }else
				std::cout << "The problem is wrong solved. The solution path has less than two elements." << std::endl;
       }//closes else
      }//closes if-solved
  }//closes moveAlongPath

 }//closes namespace PRM
};//closes namespace Kautham
