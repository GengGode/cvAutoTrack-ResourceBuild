#include <iostream>
#include <map>
#include <future>
#include <vector>
#include <opencv2/opencv.hpp>

// 用来存储地图图片区块以及对应的地图范围
// 每个区块大小为 2048 * 2048
// 能够根据范围获取地图图片区块
// 加载图片均为实际 2048 * 2048 大小
// 用于实现地图的缩放
// 遍历./map/目录下的所有图片
// 1_1.png 为(1,1)区块的图片
// 1_2.png 为(1,2)区块的图片
// 2_1.png 为(2,1)区块的图片
// 2_2.png 为(2,2)区块的图片
// 以此类推

class BlockMapResource
{
public:
    BlockMapResource()
    {
        loadMap();
    }
    ~BlockMapResource() = default;

    // 加载地图图片
    void loadMap();

    // 获取地图图片
    cv::Mat getMap(const cv::Rect &rect) { return get_map(rect, find_indexs(rect)); }

    cv::Mat getAllMap() { return getMap(min_rect); }

private:
    std::vector<int> find_indexs(const cv::Rect &rect);
    cv::Mat get_map(const cv::Rect &rect, const std::vector<int> indexs);
    // 图片大小归一化
    void size_normalize(cv::Mat &img)
    {
        if (img.cols == 2048 && img.rows == 2048)
            return;
        // 为了防止边缘出现淡化现象，放大时使用 INTER_NEAREST 插值
        cv ::resize(img, img, cv::Size(2048, 2048), 0, 0, (img.cols > 512 || img.rows > 512) ? cv::INTER_NEAREST : cv::INTER_CUBIC);
    }

    std::pair<int, int> parse_file_name(const std::string &name)
    {
        std::string y_str = name.substr(name.rfind("_") + 1, name.find(".") - (name.rfind("_") + 1));
        std::string x_str = name.substr(name.rfind("_", name.rfind("_") - 1) + 1, name.rfind("_") - (name.rfind("_", name.rfind("_") - 1) + 1));
        int x = -std::stoi(y_str);
        int y = -std::stoi(x_str);
        return std::make_pair(x, y);
    }

    cv::Rect gen_bounding_rect()
    {
        cv::Rect rect;
        for (auto &r : m_rect)
            rect |= r;
        return rect;
    }

private:
    // 存储地图图片
    std::vector<cv::Mat> m_map;
    // 存储地图图片对应的地图范围
    std::vector<cv::Rect> m_rect;
    // 存储地图图片对应的地图区块位置在vector中的索引
    std::map<std::pair<int, int>, int> m_index;

    cv::Rect min_rect;
};

void BlockMapResource::loadMap()
{
    // 遍历./map/目录下的所有图片
    // i_j.png 为(i,j)区块的图片
    // 以此类推

    std::string path = "../../../src/map/";
    std::vector<std::string> files;

    // 遍历目录下的所有图片
    cv::glob(path, files, false);
    for (int i = 0; i < files.size(); i++)
    {
        // 获取图片区块坐标 UI_MAP_-1_1.png
        // 获取图片名
        std::string name = files[i].substr(files[i].find_last_of("\\") + 1);
        auto [x, y] = parse_file_name(name);
        // 加载图片
        cv::Mat img = cv::imread(files[i]);
        // 图片大小归一化 2048 * 2048
        size_normalize(img);
        // 存储图片
        m_map.push_back(img);
        // 存储图片对应的地图范围
        m_rect.push_back(cv::Rect((x - 1) * 2048, (y - 1) * 2048, 2048, 2048));
        // 存储图片对应的地图区块位置在vector中的索引
        m_index[std::make_pair(x, y)] = i;
    }
    min_rect = gen_bounding_rect();
}

std::vector<int> BlockMapResource::find_indexs(const cv::Rect &rect)
{
    if (auto r = rect & min_rect; r.area() == 0)
        return {};
    // 直接遍历所有区块，获取存在交集的区块的索引
    std::vector<int> indexs;
    for (int i = 0; i < m_rect.size(); i++)
    {
        cv::Rect r = rect & m_rect[i];
        if (r.area() > 0)
            indexs.push_back(i);
    }
    return indexs;
}

cv::Mat BlockMapResource::get_map(const cv::Rect &rect, const std::vector<int> indexs)
{
    cv::Mat map = cv::Mat::zeros(rect.size(), CV_8UC3);

    std::list<std::future<void>> futures;
    for (auto &index : indexs)
    {
        cv::Rect r = rect & m_rect[index];
        // 如果交集面积为0，直接返回
        if (r.area() == 0)
            continue;
        // 获取相对于地图图片的范围
        cv::Rect r1 = r - m_rect[index].tl();
        // 获取相对于区块图片的范围
        cv::Rect r2 = r - rect.tl();
        futures.emplace_back(std::async(std::launch::async, [r1, r2, index, this, &map]
                                        { m_map[index](r1).copyTo(map(r2)); }));
    }
    for (auto &f : futures)
        f.get();
    return map;
}

void test_quadTree(BlockMapResource &q, int x, int y, int w, int h)
{
    cv::Rect rect(x, y, w, h);
    cv::Mat map = q.getMap(rect);
    cv::imshow("map", map);
    cv::waitKey(100);
}

void save_quadTree(BlockMapResource &q)
{
    cv::Mat map = q.getAllMap();
    // cv::imwrite("AllMap.png", map);
    cv::Mat mini_map;
    double f = 600.0 / 2048.0;
    cv::resize(map, mini_map, cv::Size(), f, f, cv::INTER_NEAREST);
    cv::imwrite("AllMap_mini.png", mini_map);
}

void test_1st()
{
    BlockMapResource quadTree;

    auto start = std::chrono::steady_clock::now();

    save_quadTree(quadTree);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "getAllMap: " << diff.count() << " s" << std::endl;

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
            auto mat = quadTree.getMap(cv::Rect(5000 + i, 3000 + i, 200, 200));
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "getMap: " << diff.count() << " s" << std::endl;
    }
}

int main(int argc, char **argv)
{
    std::cout << "Hello World!" << std::endl;
    test_1st();
    return 0;
}