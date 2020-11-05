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

/* Author: Jan Rosell */

#include <ros/ros.h>
#include <kautham/kauthamshell.h>

using namespace std;
using namespace Kautham;


int main (int argc, char **argv) {
    ros::init(argc, argv, "kautham_node_vis");
    ros::NodeHandle n;

    ROS_INFO("Starting Kautham ROS Viewer");

//TO DO: name of robot to be read from kautham
    
    //Launch the load_robot_description.launch that loads the robot_descrition
    std::string name = "\"\\$(find kautham)/demos/models/robots/ur3_robotniq_A.urdf\"";    
    std::string launchstr = "roslaunch kautham load_robot_description.launch robot_filename:="+name;
    std::cout<<launchstr.c_str()<<std::endl;
    system(launchstr.c_str());
    
    //Modify the robot_:description to include the correct paths for the geometries
    //here it is assumed that the models are in the standard kautham directory
    std::string str;
    ros::param::get("robot_description", str);
    std::string substr1 = "filename=\"";
    std::string substr2 = "filename=\"package://kautham/demos/models/robots/";
    for (size_t index = str.find(substr1, 0); index != std::string::npos && substr1.length(); index = str.find(substr1, index + substr2.length() ) )
        str.replace(index, substr1.length(), substr2);
    ros::param::set("robot_description", str);
    //check:
    //std::string s;
    //ros::param::get("robot_description",s);
    //std::cout<<"robot = "<<s<<std::endl;  
    
//TO DO: pose of robot to be read from kautham
    
    //Modify the pose of the robot according to the kautham file
    std::string tf = "\"0.3 0.0 0.0  0.0 0.0 0.0 world base_link\"";
    std::string viewerlaunchstr = "roslaunch kautham viewer.launch basetf:="+tf;
    std::cout<<viewerlaunchstr.c_str()<<std::endl;
    system(viewerlaunchstr.c_str());

    ros::spin();

//TO DO put robot in namespaces /robot0/robot_description
    
    return 0;
}







