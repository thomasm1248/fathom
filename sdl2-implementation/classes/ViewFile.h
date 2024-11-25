#pragma once

#include <string>
#include "Node.h"
#include <vector>
#include <memory>

class ViewFile
{
public:
    virtual bool read(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows) = 0;
    virtual bool write(std::vector<std::shared_ptr<Node>>& node, std::vector<std::shared_ptr<Arrow>>& arrowss) = 0;
};
