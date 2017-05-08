/*************************************************************************\
   Copyright 2017 Institute of Industrial and Control Engineering (IOC)
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

/* Author: Isiah Zaplana, Aliakbar Akbari, Muhayyudin */


#if defined(KAUTHAM_USE_OMPL)
#if defined(KAUTHAM_USE_ODE)

#include <iostream>
#include <math.h>

#include <eigen3/Eigen/Eigen>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

#include "YumiKinematics.h"


std::vector< std::vector<double> > YumiKinematics::AnalyticalIKSolver(Eigen::Matrix4f Pose, double theta3)
{
    std::vector< std::vector<double> > SolutionSet1;
    std::vector< std::vector<double> > SolutionSet2;
    std::vector< std::vector<double> > Solutions;
    //  std::cout<<"Computing Theta4"<<std::endl;
    SolutionSet1 = ComputeTheta4(Pose, theta3,  0);
    //   std::cout<<"Computing Theta4"<<std::endl;
    SolutionSet2 = ComputeTheta4(Pose, theta3,  1);
    //    std::cout<<"computed Theta4"<<std::endl;


    for(unsigned int i=0;i<SolutionSet1.size();i++)
        Solutions.push_back(SolutionSet1[i]);

    for(unsigned int i=0;i<SolutionSet2.size();i++)
        Solutions.push_back(SolutionSet2[i]);
    return Solutions;
}

std::vector< std::vector<double> > YumiKinematics::ComputeTheta4(Eigen::Matrix4f Pose, double theta3, int byn)
{
    std::vector< std::vector<double> > SolutionSet1;
    std::vector< std::vector<double> > SolutionSet2;
    std::vector< std::vector<double> > Solutions;
    double theta4;
    double a=130014.5;
    double b=-41836.5;
    double c= Pose(0,3)*Pose(0,3)+Pose(1,3)*Pose(1,3)+(Pose(2,3)-166)*(Pose(2,3)-166)-136757.75;
    if(byn==0)
    {
        theta4= atan2(c,sqrt(a*a+b*b-c*c))-atan2(a,b);
        //    std::cout<<"Computing first Theta4 0 "<<theta4<<std::endl;
        SolutionSet1 = ComputeTheta2(Pose, theta3, theta4,0);
        //        std::cout<<"Computing SolutionSet1 0 "<<std::endl;
        //            for(int i=0; i<SolutionSet1.size(); i++)
        //            {
        //                for(int j=0; j<SolutionSet1[i].size(); j++)
        //                {
        //                std::cout<<SolutionSet1[i][j]<<" ";
        //                }
        //                std::cout<<std::endl;
        //            }
        SolutionSet2 = ComputeTheta2(Pose, theta3, theta4,1);
        //     std::cout<<"Computed Theta4"<<std::endl;

    }
    if(byn==1)
    {
        theta4= atan2(c,-sqrt(a*a+b*b-c*c))-atan2(a,b);
        //  std::cout<<"value of c, a, b"<<c<<a<<b<<std::endl;
        //    std::cout<<"value of theta4 1 "<<theta4<<std::endl;
        SolutionSet1 = ComputeTheta2(Pose, theta3, theta4,0);
        SolutionSet2 = ComputeTheta2(Pose, theta3, theta4,1);
    }

    for(unsigned int i=0;i<SolutionSet1.size();i++)
        Solutions.push_back(SolutionSet1[i]);

    for(unsigned int i=0;i<SolutionSet2.size();i++)
        Solutions.push_back(SolutionSet2[i]);
    return Solutions;

}

std::vector< std::vector<double> > YumiKinematics::ComputeTheta2(Eigen::Matrix4f Pose, double theta3, double theta4,int byn)
{
    std::vector< std::vector<double> > SolutionSet1;
    std::vector< std::vector<double> > SolutionSet2;
    std::vector< std::vector<double> > Solutions;
    double theta2;
    double a=251.5+265*cos(theta4)-40.5*sin(theta4);
    double b=40.5*cos(theta3)-40.5*cos(theta3)*cos(theta4)-265*cos(theta3)*sin(theta4);
    double c=Pose(2,3)-166;

    //  std::cout<<"Computing Theta2 a "<<a<<std::endl;
    //  std::cout<<"Computing Theta2 b "<<b<<std::endl;
    //  std::cout<<"Computing Theta2 c "<<c<<std::endl;

    if(byn==0)
    {
        theta2=atan2(c,sqrt(a*a+b*b-c*c))-atan2(a,b);
        //    std::cout<<"Computing Theta2 0 "<<theta2<<std::endl;
        SolutionSet1 = ComputeTheta1(Pose,theta2,theta3,theta4,0);
        //      std::cout<<"Computing Theta1"<<std::endl;
        SolutionSet2 = ComputeTheta1(Pose,theta2,theta3,theta4,1);
        //     std::cout<<"Computedc Theta1"<<std::endl;

    }
    if(byn==1)
    {
        theta2=atan2(c,-sqrt(a*a+b*b-c*c))-atan2(a,b);
        //    std::cout<<"value of a, b"<<" /// " <<a<<" /// "<<b<<std::endl;
        //   std::cout<<"value of theta2"<<theta4<<std::endl;
        SolutionSet1 = ComputeTheta1(Pose,theta2,theta3,theta4,0);
        SolutionSet2 = ComputeTheta1(Pose,theta2,theta3,theta4,1);
    }
    for(unsigned int i=0;i<SolutionSet1.size();i++)
        Solutions.push_back(SolutionSet1[i]);

    for(unsigned int i=0;i<SolutionSet2.size();i++)
        Solutions.push_back(SolutionSet2[i]);
    return Solutions;
}

std::vector< std::vector<double> > YumiKinematics::ComputeTheta1(Eigen::Matrix4f Pose, double theta2, double theta3, double theta4,int byn)
{
    std::vector<double> Solution1;
    std::vector<double> Solution2;
    std::vector< std::vector<double> > Solutions;
    double theta1;
    double a=40.5*cos(theta2)*cos(theta3)-251.5*sin(theta2)-265*cos(theta4)*sin(theta2)+40.5*sin(theta2)*sin(theta4)-40.5*cos(theta2)*
            cos(theta3)*cos(theta4)-265*cos(theta2)*cos(theta3)*sin(theta4);
    double b=265*sin(theta3)*sin(theta4)-40.5*sin(theta3)+40.5*sin(theta3)*cos(theta4);
    //   std::cout<<"value of theta1"<<" /// " <<a<<" /// "<<b<<std::endl;

    double c=Pose(0,3);
    if(byn==0)
    {
        theta1=atan2(c,sqrt(a*a+b*b-c*c))-atan2(a,b);
        //      std::cout<<"first value of a, b"<<" /// " <<a<<" /// "<<b<<std::endl;

        //     std::cout<<"Computing first theta "<<theta1<<std::endl;
        Solution1 = ComputeRotation(Pose,  theta1, theta2, theta3,  theta4, 0);
        // std::cout<<"Computing rotation"<<std::endl;
        Solution2 = ComputeRotation(Pose,  theta1, theta2, theta3,  theta4, 1);
        // std::cout<<"Computed rotation"<<std::endl;
    }
    if(byn==1)
    {
        theta1=atan2(c,-sqrt(a*a+b*b-c*c))-atan2(a,b);
        //      std::cout<<"value of theta1"<<theta1<<std::endl;

        //std::cout<<"Computing rotation"<<std::endl;
        Solution1 = ComputeRotation(Pose,  theta1, theta2, theta3,  theta4, 0);
        // std::cout<<"Computing rotation"<<std::endl;
        Solution2 = ComputeRotation(Pose,  theta1, theta2, theta3,  theta4, 1);
        // std::cout<<"Computed rotation"<<std::endl;
    }
    Solutions.push_back(Solution1);
    Solutions.push_back(Solution2);

    return Solutions;
}

std::vector<double>  YumiKinematics::ComputeRotation(Eigen::Matrix4f Pose, double theta1, double theta2, double theta3, double theta4,int byn)
{
    //  std::cout<<"Entered in Rot function"<<std::endl;
    //  std::cout<<"Theta1 theta2 theta3 "<<theta1<<theta2<<theta3<<theta4<<std::endl;

    std::vector<double> q;
    Eigen::Matrix3f M;
    Eigen::Matrix3f R;
    Eigen::Matrix3f Rpos;

    //std::cout<<"Reading Pose"<<std::endl;
    Rpos(0,0) = Pose(0,0); Rpos(0,1) = Pose(0,1); Rpos(0,2) = Pose(0,2);
    Rpos(1,0) = Pose(1,0); Rpos(1,1) = Pose(1,1); Rpos(1,2) = Pose(1,2);
    Rpos(2,0) = Pose(2,0); Rpos(2,1) = Pose(2,1); Rpos(2,2) = Pose(2,2);
    //std::cout<<"Reading Pose"<<std::endl;

    R(0,0) = cos(theta1)*cos(theta2)*cos(theta3)*cos(theta4)-cos(theta4)*sin(theta1)*sin(theta3)-cos(theta1)*sin(theta2)*sin(theta4);
    R(0,1) = cos(theta1)*cos(theta4)*sin(theta3)-sin(theta1)*sin(theta2)*sin(theta4)+cos(theta2)*cos(theta3)*cos(theta4)*sin(theta1);
    R(0,2) = cos(theta2)*sin(theta4)+cos(theta3)*cos(theta4)*sin(theta2);
    //std::cout<<"Reading Pose"<<std::endl;

    R(1,0) = sin(theta1)*sin(theta3)*sin(theta4)-cos(theta1)*cos(theta4)*sin(theta2)-cos(theta1)*cos(theta2)*cos(theta3)*sin(theta4);
    R(1,1) = -cos(theta4)*sin(theta1)*sin(theta2)-cos(theta1)*sin(theta3)*sin(theta4)-cos(theta2)*cos(theta3)*sin(theta1)*sin(theta4);
    R(1,2) = cos(theta2)*cos(theta4)-cos(theta3)*sin(theta2)*sin(theta4);
    //std::cout<<"Reading Pose"<<std::endl;

    R(2,0) = cos(theta3)*sin(theta1)+cos(theta1)*cos(theta2)*sin(theta3);
    R(2,1) = cos(theta2)*sin(theta1)*sin(theta3)-cos(theta1)*cos(theta3);
    R(2,2) = sin(theta2)*sin(theta3);
    //std::cout<<"Reading Pose"<<std::endl;

    M = R*Rpos;
    //  std::cout<<M<<std::endl;
    if(byn==0)
    {
        double theta6 = atan2(sqrt(M(0,2)*M(0,2)+M(2,2)*M(2,2)),M(1,2));
        double theta7 = atan2(M(2,2)/sin(theta6),-(M(0,2))/sin(theta6));
        double theta5 = atan2(M(1,1)/sin(theta6),M(1,0)/sin(theta6));
        q.push_back(theta1); q.push_back(theta2); q.push_back(theta3); q.push_back(theta4);
        q.push_back(theta5); q.push_back(theta6); q.push_back(theta7);
    }
    if(byn==1)
    {
        double theta6 = atan2(-sqrt(M(2,2)*M(2,2)+M(0,2)*M(0,2)),M(1,2));
        double theta7 = atan2(M(2,2)/sin(theta6),-(M(0,2))/sin(theta6));
        double theta5 = atan2(M(1,1)/sin(theta6),M(1,0)/sin(theta6));
        //   std::cout<<"Theta6 theta7 theta5 "<<theta6<<theta7<<theta5<<std::endl;
        q.push_back(theta1); q.push_back(theta2); q.push_back(theta3); q.push_back(theta4);
        q.push_back(theta5); q.push_back(theta6); q.push_back(theta7);
    }

    return q;

}

Eigen::MatrixXf YumiKinematics::Jacobian(Eigen::VectorXf Q)
{
    Eigen::Matrix4f A1,A2,A3,A4,A5,A6,A7;
    Eigen::VectorXf q(Q.size());
    q(0)=Q(0); q(1)=Q(1); q(2)=Q(2); q(3)=Q(3); q(4)=Q(4); q(5)=Q(5); q(6)=Q(6);
    A1 << cos(q(0)), -sin(q(0)), 0, 0, sin(q(0)), cos(q(0)), 0, 0, 0, 0, 1, 166, 0, 0, 0, 1;
    A2 << cos(q(1)), -sin(q(1)), 0, 30, 0, 0, -1, 0, sin(q(1)), cos(q(1)), 0, 0, 0, 0, 0, 1;
    A3 << cos(q(2)), -sin(q(2)), 0, -30, 0, 0, 1, 0.2515e3, -sin(q(2)), -cos(q(2)), 0, 0, 0, 0, 0, 1;
    A4 << cos(q(3)), -sin(q(3)), 0, 40.5, 0, 0, -1, 0, sin(q(3)), cos(q(3)), 0, 0, 0, 0, 0, 1;
    A5 << cos(q(4)), -sin(q(4)), 0, -40.5, 0, 0, 1, 265, -sin(q(4)), -cos(q(4)), 0, 0, 0, 0, 0, 1;
    A6 << cos(q(5)), -sin(q(5)), 0, 27, 0, 0, -1, 0, sin(q(5)), cos(q(5)), 0, 0, 0, 0, 0, 1;
    A7 << cos(q(6)), -sin(q(6)), 0, -27, 0, 0, 1, 36, -sin(q(6)), -cos(q(6)), 0, 0, 0, 0, 0, 1;

    Eigen::Matrix4f T1 = A1;
    Eigen::Matrix4f T2 = T1*A2;
    Eigen::Matrix4f T3 = T2*A3;
    Eigen::Matrix4f T4 = T3*A4;
    Eigen::Matrix4f T5 = T4*A5;
    Eigen::Matrix4f T6 = T5*A6;
    Eigen::Matrix4f T7 = T6*A7;

    Eigen::Vector3f o1(T1(0,3),T1(1,3),T1(2,3));
    Eigen::Vector3f z1(T1(0,2),T1(1,2),T1(2,2));
    Eigen::Vector3f o2(T2(0,3),T2(1,3),T2(2,3));
    Eigen::Vector3f z2(T2(0,2),T2(1,2),T2(2,2));
    Eigen::Vector3f o3(T3(0,3),T3(1,3),T3(2,3));
    Eigen::Vector3f z3(T3(0,2),T3(1,2),T3(2,2));
    Eigen::Vector3f o4(T4(0,3),T4(1,3),T4(2,3));
    Eigen::Vector3f z4(T4(0,2),T4(1,2),T4(2,2));
    Eigen::Vector3f o5(T5(0,3),T5(1,3),T5(2,3));
    Eigen::Vector3f z5(T5(0,2),T5(1,2),T5(2,2));
    Eigen::Vector3f o6(T6(0,3),T6(1,3),T6(2,3));
    Eigen::Vector3f z6(T6(0,2),T6(1,2),T6(2,2));
    Eigen::Vector3f o7(T7(0,3),T7(1,3),T7(2,3));
    Eigen::Vector3f z7(T7(0,2),T7(1,2),T7(2,2));

    Eigen::Vector3f J1 = z1.cross(o7-o1);
    Eigen::Vector3f J2 = z2.cross(o7-o2);
    Eigen::Vector3f J3 = z3.cross(o7-o3);
    Eigen::Vector3f J4 = z4.cross(o7-o4);
    Eigen::Vector3f J5 = z5.cross(o7-o5);
    Eigen::Vector3f J6 = z6.cross(o7-o6);
    Eigen::Vector3f J7 = z7.cross(o7-o7);

    Eigen::MatrixXf J(6,7);
    J(0,0) = J1(0); J(0,1) = J2(0); J(0,2) = J3(0); J(0,3) = J4(0); J(0,4) = J5(0); J(0,5) = J6(0); J(0,6) = J7(0);
    J(1,0) = J1(1); J(1,1) = J2(1); J(1,2) = J3(1); J(1,3) = J4(1); J(1,4) = J5(1); J(1,5) = J6(1); J(1,6) = J7(1);
    J(2,0) = J1(2); J(2,1) = J2(2); J(2,2) = J3(2); J(2,3) = J4(2); J(2,4) = J5(2); J(2,5) = J6(2); J(2,6) = J7(2);
    J(3,0) = z1(0); J(3,1) = z2(0); J(3,2) = z3(0); J(3,3) = z4(0); J(3,4) = z5(0); J(3,5) = z6(0); J(3,6) = z7(0);
    J(4,0) = z1(1); J(4,1) = z2(1); J(4,2) = z3(1); J(4,3) = z4(1); J(4,4) = z5(1); J(4,5) = z6(1); J(4,6) = z7(1);
    J(5,0) = z1(2);J (5,1) = z2(2); J(5,2) = z3(2); J(5,3) = z4(2); J(5,4) = z5(2); J(5,5) = z6(2); J(5,6) = z7(2);
    return J;
}


Eigen::VectorXf YumiKinematics::NumericalIKSolver(Eigen::Matrix4f desire_Pose, Eigen::Matrix4f current_Pose, Eigen::VectorXf qini, double threshold, double e)
{
    Eigen::MatrixXf J;
    Eigen::VectorXf q;
    Eigen::VectorXf error(6);
    q = qini;
    Eigen::Vector3f pd (desire_Pose(0,3),desire_Pose(1,3) ,desire_Pose(2,3));
    Eigen::Matrix3f Rd;
    Rd << desire_Pose(0,0), desire_Pose(0,1), desire_Pose(0,2),
            desire_Pose(1,0), desire_Pose(1,1), desire_Pose(1,2),
            desire_Pose(2,0), desire_Pose(2,1), desire_Pose(2,2);

    Eigen::Quaternionf uqd(Rd);

    //Euler Angles conversion
    Eigen::Vector3f rd;
    rd(0) = atan2(Rd(3,1),Rd(3,2));
    rd(1) = acos(Rd(3,3));
    rd(2) = -atan2(Rd(1,3),Rd(2,3));
    unsigned int t=0;
    while (e>=threshold)
    {
        Eigen::Vector3f pe (current_Pose(0,3),current_Pose(1,3) ,current_Pose(2,3));
        Eigen::Matrix3f Re;
        Re << current_Pose(0,0), current_Pose(0,1), current_Pose(0,2),
                current_Pose(1,0), current_Pose(1,1), current_Pose(1,2),
                current_Pose(2,0), current_Pose(2,1), current_Pose(2,2);
        Eigen::Quaternionf uqe(Re);
        //Euler Angles conversion
        Eigen::Vector3f re;
        re(0) = atan2(Re(3,1),Re(3,2));
        re(1) = acos(Re(3,3));
        re(2) = -atan2(Re(1,3),Re(2,3));

        Eigen::Vector3f u1(uqd.x(),uqd.y(),uqd.z());
        Eigen::Vector3f u2(uqe.x(),uqe.y(),uqe.z());
        Eigen::Matrix3f m;
        m<< 0,-uqd.z(),uqd.y(),
                uqd.z(),0,-uqd.x(),
                -uqd.y(),uqd.x(),0;

        Eigen::Vector3f errorP;
        errorP=pd-pe;
        Eigen::Vector3f errorO;
        errorO=uqe.w()*u1-uqd.w()*u2-m*u2;
        error(0)=errorP(0); error(1)=errorP(1); error(2)=errorP(2);
        error(3)=errorO(0); error(4)=errorO(1); error(5)=errorO(2);
        e = (errorO.squaredNorm()+errorP.squaredNorm())/2;
        J=Jacobian(q);
        Eigen::MatrixXf pJ;
        pJ= pseudoInverse(J);
        q=q+pJ*0.1*error;
        //t=t+1
    }
    return q;
}

Eigen::Matrix4f YumiKinematics::ForwardKinematics(Eigen::VectorXf Q)
{
    Eigen::Matrix4f A1,A2,A3,A4,A5,A6,A7;
    Eigen::VectorXf q(Q.size());
    q(0)=Q(0); q(1)=Q(1); q(2)=Q(2); q(3)=Q(3); q(4)=Q(4); q(5)=Q(5); q(6)=Q(6);
    // In mm
//    A1 << cos(q(0)), -sin(q(0)), 0, 0, sin(q(0)), cos(q(0)), 0, 0, 0, 0, 1, 166, 0, 0, 0, 1;
//    A2 << cos(q(1)), -sin(q(1)), 0, 30, 0, 0, -1, 0, sin(q(1)), cos(q(1)), 0, 0, 0, 0, 0, 1;
//    A3 << cos(q(2)), -sin(q(2)), 0, -30, 0, 0, 1, 0.2515e3, -sin(q(2)), -cos(q(2)), 0, 0, 0, 0, 0, 1;
//    A4 << cos(q(3)), -sin(q(3)), 0, 40.5, 0, 0, -1, 0, sin(q(3)), cos(q(3)), 0, 0, 0, 0, 0, 1;
//    A5 << cos(q(4)), -sin(q(4)), 0, -40.5, 0, 0, 1, 265, -sin(q(4)), -cos(q(4)), 0, 0, 0, 0, 0, 1;
//    A6 << cos(q(5)), -sin(q(5)), 0, 27, 0, 0, -1, 0, sin(q(5)), cos(q(5)), 0, 0, 0, 0, 0, 1;
//    A7 << cos(q(6)), -sin(q(6)), 0, -27, 0, 0, 1, 36, -sin(q(6)), -cos(q(6)), 0, 0, 0, 0, 0, 1;

    // In m
    A1 << cos(q(0)), -sin(q(0)), 0, 0,  sin(q(0)), cos(q(0)), 0, 0,  0, 0, 1, 0.166,  0, 0, 0, 1;
    A2 << cos(q(1)), -sin(q(1)), 0, 0.030,  0, 0, -1, 0,  sin(q(1)), cos(q(1)), 0, 0,  0, 0, 0, 1;
    A3 << cos(q(2)), -sin(q(2)), 0, -0.030,  0, 0, 1, 0.002515e3,  -sin(q(2)), -cos(q(2)), 0, 0,  0, 0, 0, 1;
    A4 << cos(q(3)), -sin(q(3)), 0, 0.0405,  0, 0, -1, 0,  sin(q(3)), cos(q(3)), 0, 0,  0, 0, 0, 1;
    A5 << cos(q(4)), -sin(q(4)), 0, -0.0405, 0, 0, 1,  0.265, -sin(q(4)), -cos(q(4)), 0,  0, 0, 0, 0, 1;
    A6 << cos(q(5)), -sin(q(5)), 0, 0.027,  0, 0, -1, 0,  sin(q(5)), cos(q(5)), 0, 0,  0, 0, 0, 1;
    A7 << cos(q(6)), -sin(q(6)), 0, -0.027,  0, 0, 1, 0.036,  -sin(q(6)), -cos(q(6)), 0, 0,  0, 0, 0, 1;

    Eigen::Matrix4f T1 = A1;
    Eigen::Matrix4f T2 = T1*A2;
    Eigen::Matrix4f T3 = T2*A3;
    Eigen::Matrix4f T4 = T3*A4;
    Eigen::Matrix4f T5 = T4*A5;
    Eigen::Matrix4f T6 = T5*A6;
    Eigen::Matrix4f T7 = T6*A7;
    return T7;
}


#endif
#endif
