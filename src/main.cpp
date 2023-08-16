#include <iostream>
#include "BlockMapResource.h"

void test_quadTree(BlockMapResource &q, int x, int y, int w, int h)
{
    cv::Rect rect(x, y, w, h);
    cv::Mat map = q.view(rect);
    cv::imshow("map", map);
    cv::waitKey(100);
}

void save_quadTree(BlockMapResource &q)
{

    {
        auto start = std::chrono::steady_clock::now();

        cv::Mat map = q.view(cv::Rect(-50, -50, 100, 100));
        cv::imwrite("zMap.png", map);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "view: " << diff.count() << " s" << std::endl;
    }
    auto start = std::chrono::steady_clock::now();

    cv::Mat map = q.view();

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "view: " << diff.count() << " s" << std::endl;
    cv::imwrite("AllMap.png", map);
    cv::Mat mini_map;
    double f = 600.0 / 2048.0 * 2;
    cv::resize(map, mini_map, cv::Size(), f, f, cv::INTER_NEAREST);
    cv::imwrite("AllMap_mini.png", mini_map);
}

void test_1st()
{
    BlockMapResource quadTree("../../../src/map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));
    auto start = std::chrono::steady_clock::now();

    save_quadTree(quadTree);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "view: " << diff.count() << " s" << std::endl;

    test_quadTree(quadTree, 0, 0, 100, 100);
    test_quadTree(quadTree, 0, 0, 100, 100);
    test_quadTree(quadTree, -1000, 0, 100, 100);
    test_quadTree(quadTree, -100, 0, 100, 100);
    test_quadTree(quadTree, -100, 0, 200, 100);
    test_quadTree(quadTree, 100, 0, 200, 200);
    test_quadTree(quadTree, 100, 0, 2000, 2000);
    test_quadTree(quadTree, 1000, 0, 2000, 2000);
    test_quadTree(quadTree, 100000, 0, 2000, 2000);

    cv::waitKey(10);

    //// 计时比较 getMap 和 getMap_1st 的效率
    //// 循环100次

    { // getMap
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 10; i++)
        {
            auto mat = quadTree.view(cv::Rect(5000 + i, 3000 + i, 200, 200));
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "view: " << diff.count() << " s" << std::endl;
    }
}

int main(int argc, char **argv)
{
    std::cout << "Hello World!" << std::endl;
    test_1st();
    return 0;
}