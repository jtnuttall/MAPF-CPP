#include "AStarPlanner.h"
#include <algorithm> // reverse
#include <iostream>

ostream& operator<<(ostream& os, const Constraint& constraint){
    os << "Constraint ( " 
       << getAgentId(constraint) << ", "
       << getFirstLocation(constraint) << ", "
       << getSecondLocation(constraint) << ", "
       << getTimestep(constraint)
       << " )";
    return os;
}

ostream& operator<<(ostream& os, const Path& path)
{
    for (auto loc : path) {
        os << loc << " ";
    }
    return os;
}

Path AStarPlanner::make_path(const AStarNode* goal_node) const {
    Path path;
    const AStarNode* curr = goal_node;
    while (curr != nullptr) {
        path.push_back(curr->location);
        curr = curr->parent;
    }
    std::reverse(path.begin(),path.end());
    return path;
}