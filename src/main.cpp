#include <iostream>
#include "BlockMapResource.h"
#include "MapItemSet.h"

#include <Windows.h>
std::string utf8_to_gbk(const std::string &src)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, NULL, 0);
    if (len <= 0)
        return "";
    std::shared_ptr<wchar_t> wstr(new wchar_t[len + 1]);
    memset(wstr.get(), 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, wstr.get(), len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr.get(), -1, NULL, 0, NULL, NULL);
    if (len <= 0)
        return "";
    std::shared_ptr<char> str(new char[len + 1]);
    memset(str.get(), 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr.get(), -1, str.get(), len, NULL, NULL);
    return std::string(str.get());
}

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

void test()
{
    std::filesystem::path maps_data_path("C:/Users/XiZhu/Desktop/data");

    std::map<std::string, cv::Mat> maps;
    for (auto &p : std::filesystem::directory_iterator(maps_data_path))
    {
        if (p.path().filename().string().find("UI_Map") == std::string::npos)
            continue;
        // std::cout << p.path() << std::endl;
        auto textures_path = p.path() / "Texture2D";
        if (std::filesystem::exists(textures_path) == false)
            continue;
        auto map_name = p.path().filename().string().substr(3, p.path().filename().string().size() - 3 - 1);
        std::cout << map_name << std::endl;
        BlockMapResource mr(textures_path, map_name, cv::Point(0, 0), cv::Point(0, 0));
        maps[map_name] = mr.view();
        cv::imwrite(map_name + ".png", maps[map_name]);
    }
}

void test_2()
{

    std::vector<std::shared_ptr<ItemInface>> items;
    for (int i = 0; i < 1000; i++)
        items.push_back(std::make_shared<ItemObject>(cv::Point2d(i, i), "name" + std::to_string(i)));

    ItemSetTree tree(cv::Rect2d(-8500, -8500, 17000, 17000), items);
    {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 1000; i++)
        {
            auto result = tree.find(cv::Rect2d(i, -100, 200, 200));
            // std::cout << result.size() << std::endl;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "view: " << diff.count() << " ms" << std::endl;
    }
    {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 1000; i++)
        {
            auto result = tree.find(cv::Rect2d(-1000, i, 2000, 2000));
            // std::cout << result.size() << std::endl;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "view: " << diff.count() << " ms" << std::endl;
    }
    tree.print();
}

#include <algorithm>
#include <math.h>
void test__()
{

    std::vector<std::shared_ptr<ItemInface>> items;
    for (int i = 0; i < 1000; i++)
        items.push_back(std::make_shared<ItemObject>(cv::Point2d(i, i), "name" + std::to_string(i)));

    BlockMapResource quadTree("../../src/map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));

    auto map_center = quadTree.get_abs_origin();
    auto map_rect = quadTree.get_min_rect();
    // 根据地图的中心点和地图的最小矩形，分割为四个矩形
    auto map_rect_top_left = cv::Rect2d(map_rect.x, map_rect.y, map_center.x - map_rect.x, map_center.y - map_rect.y);
    auto map_rect_top_right = cv::Rect2d(map_center.x, map_rect.y, map_rect.x + map_rect.width - map_center.x, map_center.y - map_rect.y);
    auto map_rect_bottom_left = cv::Rect2d(map_rect.x, map_center.y, map_center.x - map_rect.x, map_rect.y + map_rect.height - map_center.y);
    auto map_rect_bottom_right = cv::Rect2d(map_center.x, map_center.y, map_rect.x + map_rect.width - map_center.x, map_rect.y + map_rect.height - map_center.y);
    // 获取最大半径
    std::vector<double> rect_radius = {map_rect_top_left.width, map_rect_top_left.height, map_rect_top_right.width, map_rect_top_right.height, map_rect_bottom_left.width, map_rect_bottom_left.height, map_rect_bottom_right.width, map_rect_bottom_right.height};
    auto max_radius = *std::max_element(rect_radius.begin(), rect_radius.end());
    int max_radius_int = static_cast<int>(std::round(max_radius));

    ItemSetTree tree(cv::Rect2d(-max_radius_int, -max_radius_int, max_radius_int * 2, max_radius_int * 2), items);
    auto result = tree.find_childs(cv::Rect2d(-max_radius_int, -max_radius_int, max_radius_int * 2, max_radius_int * 2));
    auto map = quadTree.view();
    for (auto &node : result)
    {
        for (auto &item : node->items)
        {
            auto item_pos = item->pos - cv::Point2d(-max_radius_int, -max_radius_int);
            cv::circle(map, item_pos, 5, cv::Scalar(0, 0, 255), -1);
        }
        auto node_rect = node->rect;
        cv::rectangle(map, node_rect - cv::Point2d(-max_radius_int, -max_radius_int), cv::Scalar(0, 255, 0), 1);
    }

    // tree.print();
}

#include <meojson/include/json.hpp>
std::vector<cv::Point2d> from_json(std::string json_file);
std::vector<cv::Point2d> from_txt()
{
    std::vector<cv::Point2d> points;
    std::vector<std::string> paths = {
        "../../src/item/0.json",
        "../../src/item/1.json",
        "../../src/item/2.json",
        "../../src/item/3.json",
        "../../src/item/4.json",
        "../../src/item/5.json",
        "../../src/item/6.json",
        "../../src/item/7.json",
        "../../src/item/8.json",
        "../../src/item/9.json",
        "../../src/item/10.json",
        "../../src/item/11.json",
        "../../src/item/12.json",
        "../../src/item/13.json",
        "../../src/item/14.json",
        "../../src/item/15.json"};
    for (auto &path : paths)
    {
        auto point = from_json(path);
        // merge
        points.insert(points.end(), point.begin(), point.end());
    }

    return points;
}
#include <fstream>
std::vector<cv::Point2d> from_json(std::string json_file)
{
    std::vector<cv::Point2d> points;
    std::ifstream in(json_file);
    std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto json_res = json::parse(str);
    if (json_res.has_value() == false)
    {
        return points;
    }
    auto json = json_res.value();
    auto type = json.type();
    auto array = json.as_array();
    for (auto &item : array)
    {
        auto position_str = item["position"].as_string();
        // std::cout << position_str << std::endl;
        //  350.20,-456.00
        double x = std::stod(position_str.substr(0, position_str.find(",")));
        double y = std::stod(position_str.substr(position_str.find(",") + 1));
        points.push_back(cv::Point2d(x, y));
    }

    return points;
}

cv::Rect2d get_max_rect(BlockMapResource &quadTree)
{
    auto map_center = quadTree.get_abs_origin();
    auto map_rect = quadTree.get_min_rect();
    // 根据地图的中心点和地图的最小矩形，分割为四个矩形
    auto map_rect_top_left = cv::Rect2d(map_rect.x, map_rect.y, map_center.x - map_rect.x, map_center.y - map_rect.y);
    auto map_rect_top_right = cv::Rect2d(map_center.x, map_rect.y, map_rect.x + map_rect.width - map_center.x, map_center.y - map_rect.y);
    auto map_rect_bottom_left = cv::Rect2d(map_rect.x, map_center.y, map_center.x - map_rect.x, map_rect.y + map_rect.height - map_center.y);
    auto map_rect_bottom_right = cv::Rect2d(map_center.x, map_center.y, map_rect.x + map_rect.width - map_center.x, map_rect.y + map_rect.height - map_center.y);
    // 获取最大半径
    std::vector<double> rect_radius = {map_rect_top_left.width, map_rect_top_left.height, map_rect_top_right.width, map_rect_top_right.height, map_rect_bottom_left.width, map_rect_bottom_left.height, map_rect_bottom_right.width, map_rect_bottom_right.height};
    auto max_radius = *std::max_element(rect_radius.begin(), rect_radius.end());
    int max_radius_int = static_cast<int>(std::round(max_radius));
    return cv::Rect2d(-max_radius_int, -max_radius_int, max_radius_int * 2, max_radius_int * 2);
}
int main(int argc, char *argv[])
{
    auto points = from_txt();
    std::vector<std::shared_ptr<ItemInface>> items;
    for (int i = 0; i < points.size(); i++)
        items.push_back(std::make_shared<ItemObject>(points[i], "name" + std::to_string(i)));

    BlockMapResource quadTree("../../src/map/", "MapBack", cv::Point(232, 216), cv::Point(-1, 0));
    auto map_center = quadTree.get_abs_origin();
    auto max_rect = get_max_rect(quadTree); // cv::Rect2d(quadTree.get_min_rect());
    auto origin = cv::Rect2d(quadTree.get_min_rect()).tl() - cv::Point2d(map_center);

    ItemSetTree tree(max_rect, items);
    auto result = tree.find_childs(max_rect);
    auto map_r = quadTree.view(cv::Rect2d(-100, -100, 200, 200));
    auto map = quadTree.view();
    for (auto &node : result)
    {
        auto scale = 1.5;
        for (auto &item : node->items)
        {
            auto item_pos = item->pos * scale - origin;
            cv::circle(map, item_pos, 10, cv::Scalar(0, 0, 255), 4);
        }
        auto node_rect = cv::Rect2d(node->rect.tl() * scale - origin, cv::Size2d(node->rect.width * scale, node->rect.height * scale));
        cv::rectangle(map, node_rect, cv::Scalar(0, 255, 0), 1);
    }
    return 0;
}