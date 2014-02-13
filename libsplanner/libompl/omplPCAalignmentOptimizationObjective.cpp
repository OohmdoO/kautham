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
 

#if defined(KAUTHAM_USE_OMPL)

#include <ompl/base/SpaceInformation.h>
#include <ompl/base/OptimizationObjective.h>
#include <ompl/base/ProjectionEvaluator.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include "omplPCAalignmentOptimizationObjective.h"


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <ompl/tools/config/MagicConstants.h>
#include <ompl/base/ScopedState.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>



using namespace boost::numeric::ublas;
namespace ob = ompl::base;
namespace om = ompl::magic;


namespace Kautham {
  namespace omplplanner{

    //http://savingyoutime.wordpress.com/2009/09/21/c-matrix-inversion-boostublas/
    /* Matrix inversion routine.
        Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
    template<class T>
        bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse)
        {
        typedef permutation_matrix<std::size_t> pmatrix;

        // create a working copy of the input
        matrix<T> A(input);

        // create a permutation matrix for the LU-factorization
        pmatrix pm(A.size1());

        // perform LU-factorization
        int res = lu_factorize(A, pm);
        if (res != 0)
            return false;

        // create identity matrix of "inverse"
        inverse.assign(identity_matrix<T> (A.size1()));

        // backsubstitute to get the inverse
        lu_substitute(A, pm, inverse);

        return true;
    }


  //! Constructor
    PCAalignmentOptimizationObjective::PCAalignmentOptimizationObjective(const ob::SpaceInformationPtr &si, int dim) :
    ob::OptimizationObjective(si)
    {
        description_ = "PCA alignment";
        PCAdataset=false;
        dimension = dim;
        wpenalization = 1.0;

        //de moment ho fixem a 2
        dimension = 2;
        setPCAdata(0);
    }

    //! void destructor
    PCAalignmentOptimizationObjective::~PCAalignmentOptimizationObjective(){

    }



    void PCAalignmentOptimizationObjective::setPCAdata(int option)//ob::ProjectionMatrix M, ob::EuclideanProjection v){
    {
        //de moment ho fixo a ma
        Matrix pca(dimension,dimension);
        Matrix invpca(dimension,dimension);
        ob::EuclideanProjection v(dimension);
        if(option==0)
        {
            pca(0,0) = sqrt(2.0)/2.0;
            pca(0,1) = sqrt(2.0)/2.0;
            pca(1,0) = -sqrt(2.0)/2.0;
            pca(1,1) = sqrt(2.0)/2.0;
            InvertMatrix(pca, invpca);
            pcaM.mat = invpca;
            pcaM.print();
        }
        else
        {
            pca(0,0) = sqrt(2.0)/2.0;
            pca(0,1) = -sqrt(2.0)/2.0;
            pca(1,0) = sqrt(2.0)/2.0;
            pca(1,1) = sqrt(2.0)/2.0;
            InvertMatrix(pca, invpca);
            pcaM.mat = invpca;
            pcaM.print();
        }

        v[0] = 1.0;
        v[1] = 0.1;
        lambda = v;
        PCAdataset=true;
    }


    ob::Cost PCAalignmentOptimizationObjective::motionCost(const ob::State *s0, const ob::State *s1, const ob::State *s2) const
    {
        ob::StateSpacePtr space = getSpaceInformation()->getStateSpace();

        ob::ScopedState<ob::CompoundStateSpace> ss1(space);
        ob::ScopedState<ob::CompoundStateSpace> ss2(space);
        ss1 = *s1;
        ss2 = *s2;

        //Get the SE3 subspace of robot 0
        ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));

        ob::ScopedState<ob::SE3StateSpace> s1se3(ssRobot0SE3);
        ss1 >> s1se3;
        ob::ScopedState<ob::SE3StateSpace> s2se3(ssRobot0SE3);
        ss2 >> s2se3;

        //convert to a vector of 7 components
        vector<double> s1_se3coords;
        vector<double> s2_se3coords;
        s1_se3coords.resize(2);
        s2_se3coords.resize(2);
        s1_se3coords[0] = s1se3->getX();
        s1_se3coords[1] = s1se3->getY();
        s2_se3coords[0] = s2se3->getX();
        s2_se3coords[1] = s2se3->getY();


        //vector from s1 to s2 in state space: from12 = s2-s1
        vector<double> from12(dimension);
        double modul12=0.0;
        for(int i=0; i<dimension;i++)
        {
            from12[i] = s2_se3coords[i] - s1_se3coords[i];
            modul12 += from12[i]*from12[i];
        }
        modul12 = sqrt(modul12);

        //vector from s1 to s2 using the pca reference frame: vpca = M*v
        ob::EuclideanProjection to12(dimension);
        pcaM.project(&from12[0],to12);

        //to[i] es la projeccio de l'edge en la direccio de l'eigenvector i (PMDi)
        double alpha = acos(fabs(to12[0]/modul12));//angle entre l'edge i el main PMD
        //std::cout<<alpha<<" "<<modul<<std::endl;

        //prova
        double modulto12=0.0;
        for(int i=0; i<dimension;i++)
        {
            modulto12 += to12[i]*to12[i]*lambda[i]*lambda[i];
        }
        modulto12 = sqrt(modulto12);
        //std::cout<<alpha<<" "<<modul12/modulto12<<std::endl;
        alpha = (modul12/modulto12)-1;

        //end prova




        //Compute now the possible penalization due to a big change in orientation
        //double orientationpenalization=1.0;
        double orientationpenalization=0.0;
        if(s0!=NULL)
        {
            ob::ScopedState<ob::CompoundStateSpace> ss0(space);
            ss0 = *s0;
            ob::ScopedState<ob::SE3StateSpace> s0se3(ssRobot0SE3);
            ss0 >> s0se3;
            vector<double> s0_se3coords;
            s0_se3coords.resize(2);
            s0_se3coords[0] = s0se3->getX();
            s0_se3coords[1] = s0se3->getY();
            //vector from s0 to s1 in state space: from01 = s1-s0
            vector<double> from01(dimension);
            double modul01=0.0;
            for(int i=0; i<dimension;i++)
            {
                from01[i] = s1_se3coords[i] - s0_se3coords[i];
                modul01 += from01[i]*from01[i];
            }
            modul01 = sqrt(modul01);

            //if angle between vector 01 and vector 12 is not between -90 and +90 set a cost penalization
            double cosbeta = (from01[0]*from12[0]+from01[1]*from12[1])/(modul01*modul12);
            //if(cosbeta<0)
            //{
            //    orientationpenalization = wpenalization;
            //}
            orientationpenalization = acos(cosbeta)*wpenalization*modul12;
        }

        double distcost = wdistance*modul12;
        double orientcost = alpha*wfix*modul12;//(1.0-wdistance)*alpha*(wfix+(1-wfix)*modul12);
        //orientcost *= orientationpenalization;
        //return ob::Cost(distcost+orientcost);
        return ob::Cost(distcost+orientcost+orientationpenalization);
        //return ob::Cost(distcost+orientcost*orientationpenalization);

        //return ob::Cost(alpha*modul12 * orientationpenalization);
        //return ob::Cost(modul12*(wdistance+alpha*(1.0-wdistance)*orientationpenalization));

    }


    ob::Cost PCAalignmentOptimizationObjective::motionCost(const ob::State *s1, const ob::State *s2) const
    {
        /*
        ob::StateSpacePtr space = getSpaceInformation()->getStateSpace();

        ob::ScopedState<ob::CompoundStateSpace> ss1(space);
        ob::ScopedState<ob::CompoundStateSpace> ss2(space);
        ss1 = *s1;
        ss2 = *s2;

        //Get the SE3 subspace of robot 0
        ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::ScopedState<ob::SE3StateSpace> s1se3(ssRobot0SE3);
        ss1 >> s1se3;
        ob::ScopedState<ob::SE3StateSpace> s2se3(ssRobot0SE3);
        ss2 >> s2se3;

        //convert to a vector of 7 components
        vector<double> s1_se3coords;
        vector<double> s2_se3coords;
        s1_se3coords.resize(2);
        s2_se3coords.resize(2);
        s1_se3coords[0] = s1se3->getX();
        s1_se3coords[1] = s1se3->getY();
        s2_se3coords[0] = s2se3->getX();
        s2_se3coords[1] = s2se3->getY();


        //vector from s1 to s2 using the pca reference frame: vpca = M*v
        ob::EuclideanProjection to1(dimension);
        ob::EuclideanProjection to2(dimension);
        pcaM.project(&s1_se3coords[0],to1);
        pcaM.project(&s2_se3coords[0],to2);

        double dist1=0.0;
        double dist2=0.0;
        for(int i=0; i<dimension;i++)
        {
            dist1 += (to1[i]/lambda[i])*(to1[i]/lambda[i]);
            dist2 += (to2[i]/lambda[i])*(to2[i]/lambda[i]);
        }
        dist1 = sqrt(dist1);
        dist2 = sqrt(dist2);

        return ob::Cost(fabs(dist2-dist1));
        //return ob::Cost(fabs(to2[1]-to1[1]));

*/




/**/
        vector<double> from(dimension);
        //vector from s1 to s2 in state space: from = s2-s1

        ob::StateSpacePtr space = getSpaceInformation()->getStateSpace();

        ob::ScopedState<ob::CompoundStateSpace> ss1(space);
        ob::ScopedState<ob::CompoundStateSpace> ss2(space);
        ss1 = *s1;
        ss2 = *s2;

        //Get the SE3 subspace of robot 0
        ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::ScopedState<ob::SE3StateSpace> s1se3(ssRobot0SE3);
        ss1 >> s1se3;
        ob::ScopedState<ob::SE3StateSpace> s2se3(ssRobot0SE3);
        ss2 >> s2se3;

        //convert to a vector of 7 components
        vector<double> s1_se3coords;
        vector<double> s2_se3coords;
        s1_se3coords.resize(2);
        s2_se3coords.resize(2);
        s1_se3coords[0] = s1se3->getX();
        s1_se3coords[1] = s1se3->getY();
        s2_se3coords[0] = s2se3->getX();
        s2_se3coords[1] = s2se3->getY();

        double modul=0.0;
        for(int i=0; i<dimension;i++)
        {
            from[i] = s2_se3coords[i] - s1_se3coords[i];
            modul += from[i]*from[i];
        }
        modul = sqrt(modul);
        //from[0] /= modul;
        //from[1] /= modul;


        //vector from s1 to s2 using the pca reference frame: vpca = M*v
        ob::EuclideanProjection to(dimension);
        pcaM.project(&from[0],to);

        //to[i] es la projeccio de l'edge en la direccio de l'eigenvector i (PMDi)
        double alpha = acos(fabs(to[0]/modul));//angle entre l'edge i el main PMD

        //std::cout<<alpha<<" "<<modul<<std::endl;


        double distcost = wdistance*modul;
        double orientcost = alpha*wfix*modul;//(1.0-wdistance)*alpha*(wfix+(1-wfix)*modul);
        return ob::Cost(distcost+orientcost);

        //return ob::Cost(alpha*modul);


        //return ob::Cost(alpha*modul+0.1);
        //return ob::Cost(alpha*(0.1+modul));
        //return ob::Cost(alpha);
        //double vv=sin(alpha);
        //double vvv=vv*modul;
        //return ob::Cost(vvv);

/**/


        //return ob::Cost(alpha);
        //return ob::Cost(fabs(to[1]));

        /*
        //scale the projections by the eignevalues lambda
        vector<double> wcost(dimension);
        double maxcost=0.0;
        for(int i=0; i<to.size();i++)
            wcost[i] = fabs(to[i]*lambda[i]);
        //the cost is the maximum scales projection
        for(int i=0; i<wcost.size();i++)
            if(wcost[i]>maxcost) maxcost=wcost[i];

        return ob::Cost(1.0/maxcost);
        */
    }

    ob::Cost PCAalignmentOptimizationObjective::motionCostHeuristic(const ob::State *s1, const ob::State *s2) const
    {
        //????aixo no ho tinc clar!!1
        return ob::Cost(motionCost(s1, s2));
    }



    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////

    //! Constructor
      PCAalignmentOptimizationObjective2::PCAalignmentOptimizationObjective2(const ob::SpaceInformationPtr &si, int dim) :
      ob::OptimizationObjective(si)
      {
          description_ = "PCA alignment2";
          PCAdataset=false;
          dimension = dim;

          //de moment ho fixem a 2
          dimension = 2;
          setPCAdata(0);
      }

      //! void destructor
      PCAalignmentOptimizationObjective2::~PCAalignmentOptimizationObjective2(){

      }

      void PCAalignmentOptimizationObjective2::setPCAdata(int option)//ob::ProjectionMatrix M, ob::EuclideanProjection v){
      {
          //de moment ho fixo a ma
          Matrix pca(dimension,dimension);
          Matrix invpca(dimension,dimension);
          ob::EuclideanProjection v(dimension);
          if(option==0)
          {
              pca(0,0) = sqrt(2.0)/2.0;
              pca(0,1) = sqrt(2.0)/2.0;
              pca(1,0) = -sqrt(2.0)/2.0;
              pca(1,1) = sqrt(2.0)/2.0;
              InvertMatrix(pca, invpca);
              pcaM.mat = invpca;
              pcaM.print();
          }
          else
          {
              pca(0,0) = sqrt(2.0)/2.0;
              pca(0,1) = -sqrt(2.0)/2.0;
              pca(1,0) = sqrt(2.0)/2.0;
              pca(1,1) = sqrt(2.0)/2.0;
              InvertMatrix(pca, invpca);
              pcaM.mat = invpca;
              pcaM.print();
          }

          v[0] = 1.0;
          v[1] = 0.1;
          lambda = v;
          PCAdataset=true;
      }

      ob::Cost PCAalignmentOptimizationObjective2::motionCost(const ob::State *s1, const ob::State *s2) const
      {
          vector<double> from(dimension);
          //vector from s1 to s2 in state space: from = s2-s1

          ob::StateSpacePtr space = getSpaceInformation()->getStateSpace();

          ob::ScopedState<ob::CompoundStateSpace> ss1(space);
          ob::ScopedState<ob::CompoundStateSpace> ss2(space);
          ss1 = *s1;
          ss2 = *s2;

          //Get the SE3 subspace of robot 0
          ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
          ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
          ob::ScopedState<ob::SE3StateSpace> s1se3(ssRobot0SE3);
          ss1 >> s1se3;
          ob::ScopedState<ob::SE3StateSpace> s2se3(ssRobot0SE3);
          ss2 >> s2se3;

          //convert to a vector of 7 components
          vector<double> s1_se3coords;
          vector<double> s2_se3coords;
          s1_se3coords.resize(2);
          s2_se3coords.resize(2);
          s1_se3coords[0] = s1se3->getX();
          s1_se3coords[1] = s1se3->getY();
          s2_se3coords[0] = s2se3->getX();
          s2_se3coords[1] = s2se3->getY();

          double modul=0.0;
          for(int i=0; i<dimension;i++)
          {
              from[i] = s2_se3coords[i] - s1_se3coords[i];
              modul += from[i]*from[i];
          }
          modul = sqrt(modul);
          //from[0] /= modul;
          //from[1] /= modul;


          //vector from s1 to s2 using the pca reference frame: vpca = M*v
          ob::EuclideanProjection to(dimension);
          pcaM.project(&from[0],to);

          //to[i] es la projeccio de l'edge en la direccio de l'eigenvector i (PMDi)
          double alpha = acos(fabs(to[0]/modul));//angle entre l'edge i el main PMD

          //std::cout<<alpha<<" "<<modul<<std::endl;


          double distcost = wdistance*modul;
          double orientcost = (1.0-wdistance)*tan(alpha)*(wfix+(1-wfix)*modul);

          distcost=1.0;
          orientcost=alpha*modul;
          return ob::Cost(distcost+orientcost);

          //return ob::Cost(alpha*modul);
      }

      /*
      ob::Cost PCAalignmentOptimizationObjective2::stateCost(const ob::State *s1) const
      {
          ob::StateSpacePtr space = getSpaceInformation()->getStateSpace();

          ob::ScopedState<ob::CompoundStateSpace> ss1(space);
          ss1 = *s1;

          //Get the SE3 subspace of robot 0
          ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
          ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
          ob::ScopedState<ob::SE3StateSpace> s1se3(ssRobot0SE3);
          ss1 >> s1se3;

          //convert to a vector of 7 components
          vector<double> s1_se3coords;
          s1_se3coords.resize(2);
          s1_se3coords[0] = s1se3->getX();
          s1_se3coords[1] = s1se3->getY();


          //vector from s1 to s2 using the pca reference frame: vpca = M*v
          ob::EuclideanProjection to(dimension);
          pcaM.project(&s1_se3coords[0],to);

          double dist=0.0;
          for(int i=0; i<dimension;i++)
          {
              dist += (to[i]/lambda[i])*(to[i]/lambda[i]);
          }
          dist = sqrt(dist);
          return ob::Cost(dist);
      }
      */



  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////

  //! Constructor
    PCAalignmentOptimizationObjective3::PCAalignmentOptimizationObjective3(const ob::SpaceInformationPtr &si, int dim) :
    ob::MechanicalWorkOptimizationObjective(si)
    {
        description_ = "PCA alignment2";
        PCAdataset=false;
        dimension = dim;

        //de moment ho fixem a 2
        dimension = 2;
        setPCAdata(0);
    }

    //! void destructor
    PCAalignmentOptimizationObjective3::~PCAalignmentOptimizationObjective3(){

    }

    void PCAalignmentOptimizationObjective3::setPCAdata(int option)//ob::ProjectionMatrix M, ob::EuclideanProjection v){
    {
        //de moment ho fixo a ma
        Matrix pca(dimension,dimension);
        Matrix invpca(dimension,dimension);
        ob::EuclideanProjection v(dimension);
        bari.resize(dimension);

        if(option==0)
        {
            pca(0,0) = sqrt(2.0)/2.0;
            pca(0,1) = sqrt(2.0)/2.0;
            pca(1,0) = -sqrt(2.0)/2.0;
            pca(1,1) = sqrt(2.0)/2.0;
            InvertMatrix(pca, invpca);
            pcaM.mat = invpca;
            pcaM.print();
        }
        else
        {
            pca(0,0) = sqrt(2.0)/2.0;
            pca(0,1) = -sqrt(2.0)/2.0;
            pca(1,0) = sqrt(2.0)/2.0;
            pca(1,1) = sqrt(2.0)/2.0;
            InvertMatrix(pca, invpca);
            pcaM.mat = invpca;
            pcaM.print();
        }

        v[0] = 1.0;
        v[1] = 0.1;
        lambda = v;

        bari[0]=0.5;
        bari[1]=0.5;
        PCAdataset=true;
    }

    ob::Cost PCAalignmentOptimizationObjective3::motionCost(const ob::State *s1, const ob::State *s2) const
    {
      // Only accrue positive changes in cost
      double positiveCostAccrued = std::max(stateCost(s2).v - stateCost(s1).v, 0.0);
      return ob::Cost(wfix*positiveCostAccrued/si_->distance(s1,s2) + wdistance*si_->distance(s1,s2));
    }

    ob::Cost PCAalignmentOptimizationObjective3::stateCost(const ob::State *s1) const
    {
        ob::StateSpacePtr space = getSpaceInformation()->getStateSpace();

        ob::ScopedState<ob::CompoundStateSpace> ss1(space);
        ss1 = *s1;

        //Get the SE3 subspace of robot 0
        ob::StateSpacePtr ssRobot0 = ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::StateSpacePtr ssRobot0SE3 =  ((ob::StateSpacePtr) space->as<ob::CompoundStateSpace>()->getSubspace(0));
        ob::ScopedState<ob::SE3StateSpace> s1se3(ssRobot0SE3);
        ss1 >> s1se3;

        //convert to a vector of 7 components
        vector<double> s1_se3coords;
        s1_se3coords.resize(2);
        s1_se3coords[0] = s1se3->getX();
        s1_se3coords[1] = s1se3->getY();

        vector<double> from;
        from.resize(dimension);
        for(int i=0; i<dimension;i++)
        {
            from[i] = s1_se3coords[i] - bari[i];
        }

        //vector from s1 to s2 using the pca reference frame: vpca = M*v
        ob::EuclideanProjection to(dimension);
        pcaM.project(&from[0],to);

        double dist=0.0;
        for(int i=0; i<dimension;i++)
        {
            dist += (to[i]/lambda[i])*(to[i]/lambda[i]);
        }
        dist = sqrt(dist);
        return ob::Cost(dist);
    }
  }
}


#endif // KAUTHAM_USE_OMPL
