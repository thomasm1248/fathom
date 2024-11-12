#pragma once

#include "ViewFile.h"
#include "../sajson.h"

class ViewFileJSON : public ViewFile
{
public:
    ViewFileJSON(std::string filepath, SDL_Renderer* renderer);
    bool read(std::vector<std::shared_ptr<Node>> &nodes);

private:
    std::string filepath;
    SDL_Renderer* renderer;

    bool traverse(std::vector<std::shared_ptr<Node>> &nodes, const sajson::value& view);
    bool readNodeList(std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& nodeList);
    bool readNode(std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& node);
};
