#include <iostream>
#include <fstream>
#include "MAPFInstance.h"
#include "AStarPlanner.h"
#include <tuple>

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
    for (int i = 0; i < ins.num_of_agents; i++) {
        list<Constraint> constraints;
        // TODO: Define constraints
        //  constraints for Q1
        // constraints.push_back(Constraint(0, ins.goal_locations[0], VERTEX_CONSTRAINT, 4));
        // for (int loc : ins.get_adjacent_locations(ins.start_locations[1])) {
        //     if (loc != ins.start_locations[1]) {
        //         constraints.push_back(Constraint(1, ins.start_locations[1], loc, 1));
        //     }
        // }
        //  constraints for Q2
        // constraints.push_back(Constraint(0, ins.goal_locations[0], VERTEX_CONSTRAINT, 10));
        //  constraints for Q3
        constraints.push_back(Constraint(1, 10, 11, 2));
        constraints.push_back(Constraint(1, 10, VERTEX_CONSTRAINT, 2));
        constraints.push_back(Constraint(1, 10, 9, 2));
        //  Replace the following line with something like paths[i] = a_star.find_path(i, constraints);
        paths[i] = a_star.find_path(i, constraints);

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
