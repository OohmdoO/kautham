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


//FIXME: this planner is done for a single TREE robot (associtated to wkSpace->robots[0])

#include <stdio.h>
#include <time.h>

#include "prmhandplannerICRA.h"
 
 namespace Kautham {

  namespace IOC{
		
	PRMHandPlannerICRA::PRMHandPlannerICRA(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler, 
           WorkSpace *ws, int numSam, int cloudSize, KthReal cloudRad, int numHC)
      :PRMHandPlanner(stype, init, goal, samples, sampler, ws, cloudSize,  cloudRad)
	{
      _idName = "PRM Hand ICRA";
			_numberSamples = numSam;
			_numberHandConf = numHC;
			addParameter("Number Samples", _numberSamples);
			addParameter("Ratio Hand/Arm Conf", _numberHandConf);

	}


	PRMHandPlannerICRA::~PRMHandPlannerICRA(){
			
	}

    bool PRMHandPlannerICRA::setParameters()
	{
      PRMHandPlanner::setParameters();
      try{
       
        HASH_S_K::iterator it = _parameters.find("Number Samples");
        if(it != _parameters.end())
          _numberSamples = it->second;
        else
          return false;

      }catch(...){
        return false;
      }
      return true;
    }

 	bool PRMHandPlannerICRA::trySolve()
	{

		if(_samples->changed())
		{
			PRMPlanner::clearGraph(); //also puts _solved to false;
		}

		if(_solved) {
			cout << "PATH ALREADY SOLVED"<<endl;
			return true;
		}
		
		cout<<"ENTERING TRYSOLVE!!!"<<endl;
	    clock_t entertime = clock();
		
		double radius;

      KthReal dist = 0;
      std::vector<KthReal> stps(_wkSpace->getNumRobControls());
      std::vector<KthReal> coord(_wkSpace->getNumRobControls());
      std::vector<KthReal> coordarm(_wkSpace->getNumRobControls());
      Sample *tmpSample;
	  vector<KthReal> coordvector;
	  int trials, maxtrials;

	
	  //Create graph with initial and goal sample
	  if( !_isGraphSet )
	  {
		_samples->findBFNeighs(_neighThress, _kNeighs);
        connectSamples();
		PRMPlanner::loadGraph();
		if(PRMPlanner::findPath()) return true;
	  }


	  
	  //compute the number of steps to interpolate the straight segment connecting cini and cgoal
	  //we assume each arm configuration will be associated with "_numberHandConf" hand configurations
      int maxsteps = (int)((double)_numberSamples/(double)(_cloudSize*_numberHandConf)); 
	  if(maxsteps<2) maxsteps=2;

	  //maxsteps is sweeped following the Van Der Corput sequence
	  //index is the index of the Van der Corput sequence, using b bites the sequence has 2^b elements
	  //dj is the bit j of the binary representation of the index
	  //rj are the elements of the sequence
	  int index;
	  double rj;
	  int dj;
	  //find how many bits are needed to code the maxsteps
	  int b= ceil(log10( (double) maxsteps) / log10( 2.0 ));
	  maxsteps = (0x01<<b);

      
	  //compute the stepsize in each robotjoint
	  //(FIXME: the last variables correspond to those of the arm because it is defines in this way in the 
	  //robot file, thus the following loop for the joints of the trunk go from n-trunk to n)
      for(int k =_wkSpace->getNumRobControls()-_wkSpace->getRobot(0)->getTrunk(); k < _wkSpace->getNumRobControls(); k++)
			stps[k] = (goalSamp()->getCoords()[k] - initSamp()->getCoords()[k]); // this is the delta step for each dimension
			
	  //Sampling the clouds...
	  //Iterate through all the steps of the straighth path, centering a cloud of robot configurations
	  //around each. Loop by setting a new sample per step until the clouds are done.
      tmpSample = new Sample(_wkSpace->getNumRobControls());
      for(int j=0; j < _cloudSize; j++){
			vector<Sample*>::iterator itSam;
			bool autocol;//flag to test autocollisions
			//set the radius of the sphere where to sample randomly
			radius = _cloudRadius;
	

			for(int i=0; i <= maxsteps; i++){
				//Set the coordinates of the robot joints 
				//Randomly set the coordinates of the robot joints at a autocollision-free conf

				//maxsteps is sweeped following the Van Der Corput sequence
			    //dj is the bit j of the binary representation of the index
			    //rj are the elements of the sequence
				//rj = 1, 0, 0.5, 0.25, 0.75, ...
				if(i==0) rj=1;
				else{
					index = i-1;
					rj=0;
					for(int jj = 0; jj < b ; jj++){
						dj = (index >> jj) & 0x01;
						rj += ((double)dj /  (double)(0x01<<(jj+1)) );
					}
				}
				//end computing the vandercorput step index
				

				trials=0;
				maxtrials=10;
				do{
					coordvector.clear();
                    for(int k = 0; k < _wkSpace->getNumRobControls()-_wkSpace->getRobot(0)->getTrunk(); k++)
					{
						coordvector.push_back(0.0); //dummy values - not used in call to autocollision with parameter 1
					}
                    for(int k =_wkSpace->getNumRobControls()-_wkSpace->getRobot(0)->getTrunk(); k < _wkSpace->getNumRobControls(); k++)
					{
						coordarm[k] = initSamp()->getCoords()[k] + rj*stps[k] + radius*(2*(KthReal)_gen->d_rand()-1);
						if(coordarm[k] > 1) coordarm[k]=1;
						if(coordarm[k] < 0) coordarm[k]=0;
						coordvector.push_back(coordarm[k]);
					}
					//Set the new sample with the hand-arm coorinates and check for autocollision.
					_wkSpace->getRobot(0)->control2Pose(coordvector); 
					autocol = _wkSpace->getRobot(0)->autocollision(1);//only test for the trunk
					trials++;
				}while(autocol==true && trials<maxtrials);
				if(autocol==true) continue; //arm configuration in autocollision - Do not consider

				//loop for several hand conf per arm conf in the cloud
				int numAdded=0; //counter of added confs for the current step (up to _numberHandConf)
				int maxh=10*_numberHandConf;
				for(int h=0; h< _numberHandConf || (numAdded==0 && h<maxh); h++)
				{
					//Randomly set the coordinates of the hand joints at a autocollision-free conf
					trials=0;
					maxtrials=10;
					do{
						coordvector.clear();
						//sample the hand coordinates
                        for(int k = 0; k < _wkSpace->getNumRobControls()-_wkSpace->getRobot(0)->getTrunk(); k++)
						{
							coord[k] = (KthReal)_gen->d_rand();
							coordvector.push_back(coord[k]);
						}
						//load the arm coordinates computed before
                        for(int k =_wkSpace->getNumRobControls()-_wkSpace->getRobot(0)->getTrunk(); k < _wkSpace->getNumRobControls(); k++)
						{
							coord[k]=coordarm[k];
							coordvector.push_back(coord[k]);
						}
						//Set the new sample with the hand-arm coorinates and check for autocollision.				
						_wkSpace->getRobot(0)->control2Pose(coordvector); 
						autocol = _wkSpace->getRobot(0)->autocollision();
						trials++;
					}while(autocol==true && trials<maxtrials);
					if(autocol==true) continue; //hand configuration in autocollision - Do not consider
				   
					//Set the new sample with the hand-arm coorinates and collision-check.
					tmpSample->setCoords(coord);
					if( !_wkSpace->collisionCheck(tmpSample)){ 
						//Free sample. Add to the sampleset _samples.
						_samples->add(tmpSample);
                                                tmpSample = new Sample(_wkSpace->getNumRobControls());
						numAdded++;

						//add to graph
						if(i==0) PRMPlanner::connectLastSample( initSamp() );
						else if (i==maxsteps) PRMPlanner::connectLastSample( goalSamp() );
						else PRMPlanner::connectLastSample( );

						if(goalSamp()->getConnectedComponent() == initSamp()->getConnectedComponent()) 
						{
							h =  _numberHandConf; //break for h
							i = maxsteps;		  //break for i
						}
					}
				}
			}

			if(PRMPlanner::findPath())
			{
				cout << "PRM Free Nodes = " << _samples->getSize() << endl;
				printConnectedComponents();
				
				clock_t finaltime = clock();
				cout<<"TIME TO COMPUTE THE PATH = "<<(double)(finaltime-entertime)/CLOCKS_PER_SEC<<endl;
				PRMPlanner::smoothPath();

				clock_t finalsmoothtime = clock();
				cout<<"TIME TO SMOOTH THE PATH = "<<(double)(finalsmoothtime - finaltime)/CLOCKS_PER_SEC<<endl;
				
				return true;
			}
	  }
	  	  
	  maxtrials = 1000; //_numberHandConf * _cloudSize;
	  trials = 0;

	  while(trials<maxtrials && goalSamp()->getConnectedComponent() != initSamp()->getConnectedComponent())
	  {
			trials++;
			
			if(getSampleInGoalRegion() == false) continue;
			
			//add to graph
			connectLastSample( goalSamp() );

			if(PRMPlanner::findPath())
			{
				cout << "PRM Free Nodes = " << _samples->getSize() << endl;
				cout<<"PATH POUND"<<endl;
				printConnectedComponents();
				
				clock_t finaltime = clock();
				cout<<"TIME TO COMPUTE THE PATH = "<<(double)(finaltime-entertime)/CLOCKS_PER_SEC<<endl;
				
				PRMPlanner::smoothPath();
				
				clock_t finalsmoothtime = clock();
				cout<<"TIME TO SMOOTH THE PATH = "<<(double)(finalsmoothtime - finaltime)/CLOCKS_PER_SEC<<endl;
				return true;
			}
	  }
	  

	  cout << "PRM Free Nodes = " << _samples->getSize() << endl;
	  cout<<"PATH NOT POUND"<<endl;
	  printConnectedComponents();
	  
	  clock_t finaltime = clock();
	  cout<<"ELAPSED TIME = "<<(double)(finaltime-entertime)/CLOCKS_PER_SEC<<endl;
	  return false;
    }


		
  }
};
