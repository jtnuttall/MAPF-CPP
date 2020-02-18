#include "CBS.h"
#include <iostream>
#include <queue>

vector<Path> CBS::find_solution() {
    priority_queue<CBSNode*, vector<CBSNode*>, CompareCBSNode> open; // open list

    /* generate the root CBS node */
    auto root = new CBSNode();
    all_nodes.push_back(root);  // whenever generating a new node, we need to
                                 // put it into all_nodes
                                 // so that we can release the memory properly later in ~CBS()

    // find paths for the root node
    root->paths.resize(a_star.ins.num_of_agents);
    for (int i = 0; i < a_star.ins.num_of_agents; i++) {
        // TODO: if you change the input format of function find_path()
        //  you also need to change the following line to something like
        //  root->paths[i] = a_star.find_path(i, list<Constraint>());
        root->paths[i] = a_star.find_path(i, root->constraints.begin(), root->constraints.end());
        if (root->paths[i].empty()) {
            cout << "Fail to find a path for agent " << i << endl;
            return vector<Path>(); // return "No solution"
        }
    }
    // compute the cost of the root node
    for (const auto& path : root->paths)
        root->cost += path.size();

    // put the root node into open list
    open.push(root);

    while (!open.empty()) {
        auto p = open.top();
        open.pop();

        auto collision = find_collision(p->paths);
        if (getFirstAgent(collision) == -1) {
            return p->paths;
        }
        // constraints from collisions
        auto new_constraints = get_constraints(collision);
        for (const auto & constraint : new_constraints) {
            auto q = new CBSNode(*p);
            all_nodes.push_back(q);
            q->constraints.insert(constraint);

            int ai = getAgentId(constraint);
            Path path = a_star.find_path(ai, q->constraints.begin(), q->constraints.end());
            if (!path.empty()) {
                q->paths[ai] = path;

                for (Path inner_path : q->paths) {
                    q->cost += inner_path.size();
                }

                open.push(q);
            }
        }
    }

    return vector<Path>(); // return "No solution"
}

Collision CBS::find_collision(const vector<Path> & paths) const {
    int a1_at_t, a1_bf_t, a2_at_t, a2_bf_t;

    int timesteps = max_element(paths.begin(), paths.end(), 
            [](const Path & p1, const Path & p2) { return p1.size() < p2.size(); }
        )->size();

    for (int t = 0; t < timesteps; ++t) {
        // vertex collisions
        for (int a1 = 0; a1 < paths.size(); ++a1) {
            a1_at_t = atOrBack<int>(paths[a1], t);
            for (int a2 = a1 + 1; a2 < paths.size(); ++a2) {
                a2_at_t = atOrBack<int>(paths[a2], t);
                if (a1_at_t == a2_at_t) {
                    return Collision(true, a1, a2, t, a1_at_t, -1);
                }
            }
        }
    }

    for (int t = 1; t < timesteps; ++t) {
        // edge collisions
        for (int a1 = 0; a1 < paths.size(); ++a1) {
            a1_at_t = atOrBack<int>(paths[a1], t);
            a1_bf_t = atOrBack<int>(paths[a1], t-1);

            for (int a2 = a1 + 1; a2 < paths.size(); ++a2) {
                a2_at_t = atOrBack<int>(paths[a2], t);
                a2_bf_t = atOrBack<int>(paths[a2], t-1);

                if (a1_at_t == a2_bf_t && a1_bf_t == a2_at_t) 
                {
                    return Collision(false, a1, a2, t, a1_bf_t, a1_at_t);
                }
            }
        }
    }

    return Collision(false, -1, -1, -1, -1, -1);
}

vector<Constraint> CBS::get_constraints(const Collision & collision) const {
    vector<Constraint> constraints;
    int firstAgent = getFirstAgent(collision);
    int secondAgent = getSecondAgent(collision);
    int firstPosition = getFirstPosition(collision);
    int secondPosition = getSecondPosition(collision);
    int timestep = getTimestep(collision);

    if (isVertex(collision)) {
        constraints = {
            Constraint(firstAgent, 
                firstPosition, 
                VERTEX_CONSTRAINT, 
                timestep 
            ),
            Constraint(secondAgent, 
                firstPosition,
                VERTEX_CONSTRAINT, 
                timestep 
            )
        };
    } else {
        constraints = {
            Constraint(firstAgent,
                firstPosition,
                secondPosition,
                timestep 
            ),
            Constraint(secondAgent,
                secondPosition,
                firstPosition,
                timestep 
            )
        };
    }
    return constraints;
}


CBS::~CBS() {
    // release the memory
    for (auto n : all_nodes)
        delete n;
}