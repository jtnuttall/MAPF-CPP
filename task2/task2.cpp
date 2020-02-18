#include <iostream>
#include <fstream>
#include "MAPFInstance.h"
#include "AStarPlanner.h"
#include <tuple>
#include <set>

int main(int argc, char *argv[]) {
    MAPFInstance ins;
    string input_file = argv[1];
    string output_file = argv[2];
    if (ins.load_instance(input_file)) {
        ins.print_instance();
    } else {
        cout << "Fail to load the instance " << input_file << endl;
        exit(-1);
    }

    AStarPlanner a_star(ins);
    vector<Path> paths(ins.num_of_agents);

    // assign priority ordering to agents
    // By default, we use the index ordering of the agents where
    // the first always has the highest priority.
    list<int> priorities;
    for (int i = 0; i < ins.num_of_agents; i++) {
        priorities.push_back(i);
    }
    // reverse(priorities.begin(), priorities.end());

    set<Constraint> constraints;

    // plan paths
    for (int i : priorities) {
        paths[i] = a_star.find_path(i, constraints.begin(), constraints.end());

        for (int a = 0; a < ins.num_of_agents; ++a) {
            if (a != i) {
                int t = 1;
                for (t; t < paths[i].size(); ++t) {
                    /* Vertex constraints */
                    // int tv = paths[i][t] != ins.goal_locations[i] ? t : ALL_FUTURE_TIMESTEPS;
                    constraints.emplace(a, paths[i][t], VERTEX_CONSTRAINT, t);
                    /* Edge constraints */
                    //constraints.emplace(a, paths[i][t-1], paths[i][t], t);
                    constraints.emplace(a, paths[i][t], paths[i][t-1], t);
                }

                if (!paths[i].empty()) {
                    constraints.emplace(a, paths[i].back(), VERTEX_CONSTRAINT, -t);
                }
            }
        }

        if (paths[i].empty()) {
            cout << "Fail to find any solutions for agent " << i << endl;
            return 0;
        }
    }

    // print paths
    cout << "Paths:" << endl;
    int sum = 0;
    for (int i = 0; i < ins.num_of_agents; i++) {
        cout << "a" << i << ": " << paths[i] << endl;
        sum += paths[i].size();
    }
    cout << "Sum of cost: " << sum << endl;

    // save paths
    ofstream myfile (output_file.c_str(), ios_base::out);
    if (myfile.is_open()) {
        for (int i = 0; i < ins.num_of_agents; i++) {
            myfile << paths[i] << endl;
        }
        myfile.close();
    } else {
        cout << "Fail to save the paths to " << output_file << endl;
        exit(-1);
    }
    return 0;
}