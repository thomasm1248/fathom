#pragma once

#include <string>
#include "Node.h"
#include <vector>
#include <memory>

class ViewFile
{
public:
    virtual bool read(std::vector<std::shared_ptr<Node>> &nodes) = 0;
    virtual bool write(std::vector<std::shared_ptr<Node>>& nodes) = 0;
};
