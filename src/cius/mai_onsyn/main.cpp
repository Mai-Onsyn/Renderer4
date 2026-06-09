#include <iostream>

import Application;
import Time;
import Types;
import Tile;
import MathUtil;

int main() {
    // Application app("DisplayWindow", 800, 600);
    // app.run();
    List<Tile> tiles = Tile::divideScreen(1920, 1080, 11);

    for (const auto& tile : tiles) {
        std::cout << tile.toString() << std::endl;
    }

    return 0;
}