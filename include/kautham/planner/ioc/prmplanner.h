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



#if !defined(_PRMPLANNER_H)
#define _PRMPLANNER_H

#include <kautham/problem/workspace.h>
#include <kautham/sampling/sampling.h>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>
#include <kautham/planner/ioc/localplanner.h>
#include <kautham/planner/ioc/iocplanner.h>


using namespace std;

namespace Kautham {
/** \addtogroup Planner
 *  @{
 */
  namespace IOC{
    //Typedefs
    typedef Sample* location;
    typedef KthReal cost;
    typedef std::pair<int, int> prmEdge;
    typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost> > prmGraph;
    typedef prmGraph::vertex_descriptor prmVertex;
    typedef boost::property_map<prmGraph, boost::edge_weight_t>::type WeightMap;
    typedef prmGraph::edge_descriptor edge_descriptor;
    typedef prmGraph::vertex_iterator vertex_iterator;


//CLASS distance heuristic

    template <class Graph, class CostType, class LocMap>
    class distance_heuristic : public boost::astar_heuristic<Graph, CostType>
    {
        public:
        typedef typename boost::graph_traits<Graph>::vertex_descriptor prmVertex;

        distance_heuristic(LocMap l, prmVertex goal, LocalPlanner* lPlan) : m_location(l), m_goal(goal), locPlan(lPlan) {};
        CostType operator()(prmVertex u)
        {
            //returns euclidean distance form vertex u to vertex goal
            return locPlan->distance(m_location[u],m_location[m_goal]);
        }

        private:
            LocMap m_location;
            prmVertex m_goal;
            LocalPlanner* locPlan;
    };


    struct found_goal{}; // exception for termination

    //CLASS astar_goal_visitor
    // visitor that terminates when we find the goal
    template <class prmVertex>
    class astar_goal_visitor : public boost::default_astar_visitor
    {
        public:
            astar_goal_visitor(prmVertex goal) : m_goal(goal) {};

            template <class Graph>
            void examine_vertex(prmVertex u, Graph& g)
            {
                if(u == m_goal)throw found_goal();
            }

        private:
      prmVertex m_goal;
    };


    //CLASS PRMPlanner
    class PRMPlanner:public iocPlanner
    {
        public:
        PRMPlanner(SPACETYPE stype, Sample *init, Sample *goal, SampleSet *samples, Sampler *sampler,
          WorkSpace *ws);
        ~PRMPlanner();
            bool        setParameters();

      //! This method overwrites the original method in Planner class in order to improve the capabilities
      //! to store the data used in this kind of planner. This method invokes  the parent method to save
      //! the SampleSet and the planner's parameters and then It stores the connectivity of the samples
      //!to compound the PRM.
            bool        saveData(string path); // Overwriting the Planner::saveData(string) method.

      void        saveData(); // This is a convenient way to store solution path. Jan.

      bool        loadData(string path); // Overwriting the Planner::loadData(string) method.
            //void        setIniGoal();
            bool        trySolve();


            //!overloaded
        void moveAlongPath(unsigned int step);

        //!find path
        bool findPath();
        //!load boost graph data
        void loadGraph();
        //!connect samples
        bool connectSamples(bool assumeAllwaysFree = false);
          //! connects last sampled configuration & adds to graph
        void connectLastSample(Sample* connectToSmp = NULL, Sample* connectToSmp2 = NULL);
        //!delete g
        void clearGraph();
            void updateGraph();
            void smoothPath(bool maintainfirst=false, bool maintainlast=false);
            bool isGraphSet(){return _isGraphSet;}
            void printConnectedComponents();


     SoSeparator *getIvCspaceScene();//reimplemented
     void drawCspace();

        protected:




            vector<prmEdge*> edges;
      //!edge weights
      vector<cost> weights;
      //!bool to determine if the graph has been loaded
      bool _isGraphSet;
            KthReal _neighThress;
            unsigned     _kNeighs;
            std::map<int, SampleSet*> _ccMap;
      int _labelCC;
      int _drawnLink; //!>flag to show which link path is to be drawn
      KthReal _probabilityConnectionIniGoal; //probability to connect last samp`le to init and goal samp`les

      //!flag to store the type of sampler
      int _samplertype;
      //!maximum level used for the SDK sampler
      int _levelSDK;
      //!pointer to a SDK sampler
      SDKSampler* _samplerSDK;
      //!pointer to a Halton sampler
      HaltonSampler* _samplerHalton;
      //!pointer to a Gaussian sampler
      GaussianSampler* _samplerGaussian;
      //!pointer to a Gaussian sampler
      GaussianLikeSampler* _samplerGaussianLike;
      //!pointer to the sampler to be used to build the roadmap
      Sampler* _samplerUsed;

    void setsampler(int i);

        private:
      PRMPlanner();
      //!boost graph
      prmGraph *g;
      //!solution to query
      list<prmVertex> shortest_path;
      //!pointer to the samples of cspace to be used by the distance_heuristic function used in A*
      vector<location> locations;
      };
    }

  /** @}   end of Doxygen module "Planner */
}

#endif  //_PRMPLANNER_H
