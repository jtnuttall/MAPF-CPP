#pragma once
#include "MAPFInstance.h"
#include <ostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

constexpr int VERTEX_CONSTRAINT = -1;
constexpr int ALL_FUTURE_TIMESTEPS = -1;


template <typename T>
T atOrBack(vector<T> vec, size_t index) {
    return index < vec.size() ? vec[index] : vec.back();
}

// Vertex constraint <a, x, -1, t>
// that prohibits agent a from being at location x at timestep t
// Edge constraint <a, x, y, t>
// that prohibits agent a from moving from locations x to y from timesteps t-1 to t
typedef tuple<int, int, int, int > Constraint;
inline int getAgentId(const Constraint & constraint) { return get<0>(constraint); }
inline int getFirstLocation(const Constraint & constraint) { return get<1>(constraint); }
inline int getSecondLocation(const Constraint & constraint) { return get<2>(constraint); }
inline int isVertexConstraint(const Constraint & constraint) 
    { return getSecondLocation(constraint) == VERTEX_CONSTRAINT; }
inline int getTimestep(const Constraint & constraint) { return get<3>(constraint); }
inline bool allRemainingTimesteps(const Constraint & constraint)
    { return getTimestep(constraint) < 0; }

ostream& operator<<(ostream& os, const Constraint& constraint);

inline bool operator<(const Constraint& lhs, const Constraint& rhs) {
    return 
        getAgentId(lhs) < getAgentId(rhs) ||
        getFirstLocation(lhs) < getFirstLocation(rhs) ||
        getSecondLocation(lhs) < getSecondLocation(rhs) ||
        allRemainingTimesteps(lhs) == allRemainingTimesteps(rhs) ||
        getTimestep(lhs) < getTimestep(rhs);
}

// Path is a sequence of locations,
// where path[i] represents the location at timestep i
typedef vector<int> Path;
ostream& operator<<(ostream& os, const Path& path); // used for printing paths

// A hash function used to hash a pair of any kind
// This will be used in Task 1 when you try to
// use pair as the key of an unordered_map
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

struct AStarNode {
    int location;
    int g;
    int h;
    int timestep;
    AStarNode* parent;

    AStarNode(): location(-1), g(-1), h(-1), timestep(-1), parent(nullptr) {}
    AStarNode(int location, int g, int h, int timestep, AStarNode* parent):
            location(location), g(g), h(h), timestep(timestep), parent(parent) {}
};

// This function is used by priority_queue to prioritize nodes
struct CompareAStarNode {
    bool operator()(const AStarNode* n1, const AStarNode* n2) {
        if (n1->g + n1->h == n2->g + n2->h) // if both nodes have the same f value,
            return n1->h > n2->h; // break ties by preferring smaller h value
        else
            return n1->g + n1->h > n2->g + n2->h; // otherwise, prefer smaller f value
    }
};

class AStarPlanner {
public:
    const MAPFInstance& ins;

    AStarPlanner(const MAPFInstance& ins): ins(ins) {}

    /* Avoid rewriting code using list */
    inline Path find_path(int agent_id, list<Constraint> & constraints) {
        return find_path(agent_id,constraints.begin(), constraints.end());
    }

    /* Otherwise, use iterator. Allows free choice of constraints container type */
    template <class Iterator>
    Path find_path(int agent_id, Iterator constraints_begin, Iterator constraints_end) {
        this->agent_id = agent_id;
        int start_location = ins.start_locations[agent_id];
        int goal_location = ins.goal_locations[agent_id];

        // Open list
        priority_queue<AStarNode*, vector<AStarNode*>, CompareAStarNode> open;

        // Unordered map is an associative container that contains key-value pairs with unique keys.
        // The following unordered map is used for duplicate detection, where the key is the location of the node.
        // unordered_map<int, AStarNode*> all_nodes;
        // TODO: For Task 1, you need to replace the above line with
        unordered_map<pair<int, int>, AStarNode*, hash_pair> all_nodes;

        timestep = 0;
        int h = ins.get_Manhattan_distance(start_location, goal_location); // h value for the root node
        auto root = new AStarNode(start_location, 0, h, timestep, nullptr);
        open.push(root);


        Path path;
        while (!open.empty()) {
            curr = open.top();
            open.pop();

            timestep = curr->timestep + 1;

            // goal test
            bool has_goal_constraint = any_of(constraints_begin, constraints_end,
                [=](const Constraint & constraint) {
                    return getTimestep(constraint) > curr->timestep
                        && (  getFirstLocation(constraint) == goal_location
                           || getSecondLocation(constraint) == goal_location
                           );
                }
            );

            if (curr->location == goal_location && !has_goal_constraint) {
                path = make_path(curr);
                break;
            }

            if (curr->timestep > ins.num_of_agents * ins.map_size()) {
                path = Path();
                break;
            }

            /* apply constraints
             * - curr->location at timestep-1
             * - next_location at timestep
             */
            list<int> adj_locs = ins.get_adjacent_locations(curr->location);

            // cout << agent_id << endl;
            prune_nodes(adj_locs, constraints_begin, constraints_end);

            AStarNode * next;
            // generate child nodes
            for (auto next_location : adj_locs) {
                auto it = all_nodes.find(make_pair(next_location, timestep));

                // the location has not been visited before and is valid at constraint
                if (it == all_nodes.end()) {
                    int next_g = curr->g + 1;
                    int next_h = ins.get_Manhattan_distance(next_location, goal_location);

                    next = new AStarNode(next_location, next_g, next_h, timestep, curr);
                    open.push(next);

                    all_nodes[make_pair(next_location, timestep)] = next;
                }
                // Note that if the location has been visited before,
                // next_g + next_h must be greater than or equal to the f value of the existing node,
                // because we are searching on a graph with uniform-cost edges.
                // So we don't need to update the existing node.
            }
        }

        // release memory
        for (auto n : all_nodes)
            delete n.second;

        return path;
    }

private:
    AStarNode * curr;
    int agent_id;
    int timestep;
    // used to retrieve the path from the goal node
    Path make_path(const AStarNode* goal_node) const;

    void clear();

    /* Prune nodes on any container that implements forward iterator */
    template <class Iterator>
    void prune_nodes(list<int> & adj_locs, Iterator constraints_begin, Iterator constraints_end) {
        /* Capture by value lambda plus remove_if simplifies filter op. */
        adj_locs.erase(remove_if(adj_locs.begin(), adj_locs.end(),
            [=](int next_location) {
                return
                any_of(constraints_begin, constraints_end,
                    [=](const Constraint & constraint) {
                        bool appliesToTimestep
                            = allRemainingTimesteps(constraint)
                            ? timestep >= -getTimestep(constraint)
                            : timestep == getTimestep(constraint);
                        /* If constraint applies to agent and timestep */
                        if ( getAgentId(constraint) == agent_id && appliesToTimestep)
                        {
                            return isVertexConstraint(constraint)
                                ? next_location == getFirstLocation(constraint)
                                : curr->location == getFirstLocation(constraint)
                                    && next_location == getSecondLocation(constraint);
                        }

                        /* No constraints apply, next. */
                        return false;
                    }
                );
            }
        ), adj_locs.end());
    }
};

