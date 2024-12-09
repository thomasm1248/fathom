#pragma once

#include "ViewFile.h"
#include "../sajson.h"
#include "Font.h"

class ViewFileJSON : public ViewFile
{
public:
    ViewFileJSON(std::string filepath, SDL_Renderer* renderer, std::shared_ptr<Font> labelNodeFont);
    bool read(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows);
    bool write(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows);

private:
    std::string filepath;
    SDL_Renderer* renderer;
    std::shared_ptr<Font> labelNodeFont;

    bool traverse(std::vector<std::shared_ptr<Node>> &nodes, std::vector<std::shared_ptr<Arrow>>& arrows, const sajson::value& view);
    bool readNodeList(std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& nodeList);
    bool readNode(std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& node);
    bool readArrowList(std::vector<std::shared_ptr<Arrow>>& arrows, std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& arrowList);
    bool readArrow(std::vector<std::shared_ptr<Arrow>>& arrows, std::vector<std::shared_ptr<Node>>& nodes, const sajson::value& arrow);
};
