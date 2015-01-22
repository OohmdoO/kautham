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


#include "gaussianLikesampler.h"
#include <iostream>
#include <fstream>
//#include <stdio.h>

namespace Kautham{
  


	GaussianLikeSampler::GaussianLikeSampler(char dim, char maxLevel, WorkSpace *w):Sampler()
	{
		sdkgen = new SDKSampler(dim,maxLevel);
		randgen = new RandomSampler(dim);
		haltgen = new HaltonSampler(dim);
		generator = new LCPRNG(3141592621, 1, 0, ((unsigned int)time(NULL) & 0xfffffffe) + 1);//LCPRNG(15485341);//15485341 is a big prime number
		ss = new SampleSet();
		ws = w;
		ss->setTypeSearch(ANNMETHOD);//(BRUTEFORCE);//
		ss->setWorkspacePtr(ws);
		_maxNeighs = 4;
		_maxSamples = 100000;
		ss->setANNdatastructures(_maxNeighs, _maxSamples);
		itfree=0;
	}

	Sample* GaussianLikeSampler::nextSample() {
		return nextSample(true);
		//return nextSample(false);
	}

	
	void GaussianLikeSampler::print() 
	{
	  Sample *smp;

	  ofstream fp;
	  fp.open( "ssGSDK.txt" );
	  fp.precision(2);
	  for(unsigned int j =0; j < ss->getSize(); j++){
	    smp = ss->getSampleAt(j);
	    vector<KthReal> &c = smp->getCoords();
	    for(unsigned int i=0; i<smp->getDim(); i++)
		fp << c.at(i) << " ";

	    fp << smp->getcolor() << std::endl ;
	  }

	  fp.close();
		
	  Sample *smpN;
	  
	  fp.open( "ssGSDK_N.txt" );
	  fp.precision( 2 );
	    for(unsigned int j =0; j < ss->getSize(); j++){
	      smp = ss->getSampleAt(j);
	      vector<KthReal> &c = smp->getCoords();
	      for(unsigned int i=0; i<smp->getDim(); i++)
		fp << c.at(i) << " ";

	      vector<unsigned int>* ne=smp->getNeighs();
	      fp << _maxNeighs << " ";
	      unsigned int k;
	      for(k=0; k<ne->size(); k++){
		smpN = ss->getSampleAt((*ne)[k]);
		vector<KthReal> &cN = smpN->getCoords();
		for(unsigned int i=0; i<smpN->getDim(); i++)
			fp << cN.at(i) << " ";
	      }
			//rellenar con la misma muestra hasta kneigh vecinos
	      for(; k<_maxNeighs; k++){
		for(unsigned int i=0; i<smp->getDim(); i++)
		  fp << c.at(i) << " ";
	      }
	      fp << std::endl ;
	     }
	     fp.close();
	}
	
	Sample *GaussianLikeSampler::setcolor(Sample *smp, vector<unsigned int> *N) 
	{
		KthReal transpThreshold = 0.5;//0.7;//0.3
        unsigned initialset = 0;//64;
			//Compute the transparency of smp  
			KthReal t = 0;
            for(unsigned i=0; i<N->size();i++)
			{
				t += (ss->getSampleAt((*N)[i]))->getcolor();
			}
			if( N->size() ) t /= N->size();
			smp->setTransparency(t);

			//If transparency below threshold then collision-check
			if(fabs((double)smp->getTransparency() ) < transpThreshold )
			{
				if(ws->collisionCheck(smp)) smp->setcolor(-1.0);
				else smp->setcolor(1.0);
			}
			else smp->setcolor(0.0);

			if(smp->isFree())
			{
				//first generate SDK samples (uniform)
				//if first samples, or sample without neighbors
				//then return the sample;
				//otherwise return the sample only if it has obstacle neighbors
				if(ss->getSize()<initialset || N->size()==0)
				{
					//make a copy
					//SDKSample *smpc = new SDKSample((SDKSample*)smp);
					Sample *smpc = new Sample(smp);
					//return a pointer to the copy
					return smpc;
				}
                else for(unsigned i=0; i<N->size();i++)
				{	
					Sample* tmpsmp = ss->getSampleAt((*N)[i]); 
					if(tmpsmp->getcolor()==-1) 
					{
						//make a copy
						//SDKSample *smpc = new SDKSample((SDKSample*)smp);
						Sample *smpc = new Sample(smp);
						//return a pointer to the copy
						return smpc;
					}
				}
			}
			return NULL;
	}




	Sample* GaussianLikeSampler::nextSample(bool random) 
	{
		vector<unsigned int> *N;
		//Sample *smpi;
		//KthReal thresholdDelta = 0.3;
		KthReal thresholdNeighs = 50000.0;//big htrehold in order to consider all k-neighs
		//int initialset = 0;//64;

		bool done = false;
		do{
			//Call the sdk sample generator
			Sample *smp = sdkgen->nextSample(random);
			//Sample *smp = randgen->nextSample();
			//Sample *smp = haltgen->nextSample();
			smp->clearNeighs();

			//Add smp to SampleSet ss
			ss->add(smp, false);
	
			//Find the set N of neighbors of smp in ss
			//Update the neighbor lists of the samples in N
			N = ss->findNeighs(smp, thresholdNeighs, _maxNeighs);

			//setcolor of sample depending on neighs
			Sample *ptrsmp=setcolor(smp, N);
			if(ptrsmp!=NULL) {
				ptrsmp->clearNeighs();
				vectfree.push_back(ptrsmp);
			}

			//update neighs
            for(unsigned i=0; i<N->size();i++)
			{	
				Sample* tmpsmp = ss->getSampleAt((*N)[i]); 
				if(tmpsmp->getcolor()==0)
				{
					Sample *ptrsmp=setcolor(tmpsmp,tmpsmp->getNeighs());
					if(ptrsmp!=NULL) {
						ptrsmp->clearNeighs();
						vectfree.push_back(ptrsmp);
					}
				}	
			}

			//return a free sample if available
			if(itfree< vectfree.size())
			{
				_current = vectfree[itfree];
				itfree++;
				done = true;
				break;
			}


		}while(done==false && ss->getSize()<_maxSamples);
		
                if(ss->getSize() < _maxSamples)
                    return _current;
                else
//                    exit(0);
                    return NULL;
	}


}




