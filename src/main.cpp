#include <iostream>
#include <map>
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

/// @brief 用来存储地图图片区块以及对应的地图范围
struct _BlockMapNode
{
    std::pair<int, int> id;

    int depth = 0;
    bool is_leaf = false;

    // 存储地图图片
    cv::Mat image;
    // 存储地图图片对应的地图范围
    cv::Rect rect;

    // 存储地图图片对应的区块相邻关系
    std::shared_ptr<_BlockMapNode> up;
    std::shared_ptr<_BlockMapNode> down;
    std::shared_ptr<_BlockMapNode> left;
    std::shared_ptr<_BlockMapNode> right;
};

class _BlockMap
{
public:
    _BlockMap();
    ~_BlockMap();

    /// @brief 插入区块
    /// @param x 区块id
    /// @param y 区块id
    /// @param image 区块图片
    /// @param rect 区块范围
    void insert(const int x, const int y, const cv::Mat &image, const cv::Rect &rect)
    {
        std::shared_ptr<_BlockMapNode> node = std::make_shared<_BlockMapNode>();
        node->id = std::make_pair(x, y);
        node->image = image;
        node->rect = rect;
        node->depth = 0;
        node->is_leaf = true;
        m_map[std::make_pair(x, y)] = node;

        m_rect.x = std::min(m_rect.x, rect.x);
        m_rect.y = std::min(m_rect.y, rect.y);
        m_rect.width = std::max(m_rect.width, rect.x + rect.width) - m_rect.x;
        m_rect.height = std::max(m_rect.height, rect.y + rect.height) - m_rect.y;
    }

    bool find(const int x, const int y)
    {
        return m_map.find(std::make_pair(x, y)) != m_map.end();
    }

    void remove(const int x, const int y)
    {
        m_map.erase(std::make_pair(x, y));
    }

    std::shared_ptr<_BlockMapNode> get(const int x, const int y)
    {
        return get_not_null(x, y);
    }

private:
    std::shared_ptr<_BlockMapNode> get_not_null(const int x, const int y)
    {
        if (m_map.find(std::make_pair(x, y)) == m_map.end())
        {
            // return std::make_shared<_BlockMapNode>();
            auto node = std::make_shared<_BlockMapNode>();
            node->id = std::make_pair(x, y);
            node->depth = 0;
            node->is_leaf = true;
            m_map[std::make_pair(x, y)] = node;
        }
        return m_map[std::make_pair(x, y)];
    }

private:
    std::map<std::pair<int, int>, std::shared_ptr<_BlockMapNode>> m_map;
    cv::Rect m_rect;
};

class BlockMap
{
    _BlockMap map;

public:
    void insert(const int x, const int y, const cv::Mat &image)
    {
        map.insert(x, y, image, cv::Rect((x - 1) * 2048, (y - 1) * 2048, 2048, 2048));
    }
    cv::Mat get(const int x, const int y)
    {
        map.get(x, y)->image;
    }
    void remove(const int x, const int y)
    {
        map.remove(x, y);
    }
};
struct BlockPosition
{
    int x;
    int y;
};
struct BlockMapNode
{
    cv::Mat m_map;
    cv::Rect m_rect;
    BlockPosition pos;
    int x;
    int y;
    int index;
};

class BlockMapResource
{
public:
    BlockMapResource();
    ~BlockMapResource();

    // 加载地图图片
    void loadMap();

    // 获取地图图片
    cv::Mat getMap(const cv::Rect &rect);

    cv::Mat getAllMap();

    struct Adjacent
    {
        int up = -1;
        int down = -1;
        int left = -1;
        int right = -1;
    };

private:
    std::vector<int> find_indexs(const cv::Rect &rect);
    std::array<int, 4> adjacent(const int index);
    void getMap(const cv::Mat &mat, const cv::Rect &rect, const std::vector<int> indexs);
    // 是否递归检索过的区块的标记
    std::vector<bool> m_flag;

private:
    inline int id(const int pos) { return pos / 2048 == 0 ? 1 : pos / 2048 + 1; }

private:
    // 存储地图图片
    std::vector<cv::Mat> m_map;
    // 存储地图图片对应的地图范围
    std::vector<cv::Rect> m_rect;
    // 存储地图图片对应的区块相邻关系
    std::vector<Adjacent> m_adjacent;
    // 存储地图图片对应的地图区块位置在vector中的索引
    std::map<std::pair<int, int>, int> m_index;

    cv::Rect min_rect;
    cv::Point min_tl_point;
    cv::Point min_br_point;
};

BlockMapResource::BlockMapResource()
{
    loadMap();
}

BlockMapResource::~BlockMapResource()
{
}

void BlockMapResource::loadMap()
{
    // 遍历./map/目录下的所有图片
    // i_j.png 为(i,j)区块的图片
    // 以此类推

    std::string path = "C:/Users/XiZhu/source/repos/cvAutoTrack-ResourceBuild/src/map/";
    std::vector<std::string> files;

    // 遍历目录下的所有图片
    cv::glob(path, files, false);
    for (int i = 0; i < files.size(); i++)
    {
        // 获取图片名
        std::string name = files[i].substr(files[i].find_last_of("\\") + 1);
        // 获取图片区块坐标 UI_MAP_-1_1.png
        std::string y_str = name.substr(name.rfind("_") + 1, name.find(".") - (name.rfind("_") + 1));
        std::string x_str = name.substr(name.rfind("_", name.rfind("_") - 1) + 1, name.rfind("_") - (name.rfind("_", name.rfind("_") - 1) + 1));
        int x = -std::stoi(y_str);
        int y = -std::stoi(x_str);
        // 加载图片
        cv::Mat img = cv::imread(files[i]);
        // 图片大小归一化 2048 * 2048
        if (img.cols != 2048 || img.rows != 2048)
        {
            // 为了防止边缘出现淡化现象，放大时使用 INTER_NEAREST 插值
            if (img.cols > 512 || img.rows > 512)
                cv::resize(img, img, cv::Size(2048, 2048), 0, 0, cv::INTER_NEAREST);
            else
                cv::resize(img, img, cv::Size(2048, 2048), 0, 0, cv::INTER_CUBIC);
        }
        // 存储图片
        m_map.push_back(img);
        // 存储图片对应的地图范围
        int x_org = (x - 1) * 2048;
        int y_org = (y - 1) * 2048;
        m_rect.push_back(cv::Rect(x_org, y_org, 2048, 2048));
        // 存储图片对应的地图区块位置在vector中的索引
        m_index[std::make_pair(x, y)] = i;

        //
        if (x_org < min_tl_point.x)
        {
            min_tl_point.x = x_org;
        }
        if (y_org < min_tl_point.y)
        {
            min_tl_point.y = y_org;
        }
        if (x_org > min_br_point.x)
        {
            min_br_point.x = x_org;
        }
        if (y_org > min_br_point.y)
        {
            min_br_point.y = y_org;
        }

        min_rect = cv::Rect(min_tl_point, min_br_point + cv::Point(2048, 2048));
    }
    for (int i = 0; i < m_rect.size(); i++)
    {
        int x = m_rect[i].x / 2048 + 1;
        int y = m_rect[i].y / 2048 + 1;
        // 存储图片对应的区块相邻关系
        /*
              1,4
          2,3 2,4 2,5
              3,4
        */
        Adjacent adjacent;
        if (m_index.find(std::make_pair(x - 1, y)) != m_index.end())
        {
            adjacent.up = m_index[std::make_pair(x - 1, y)];
        }
        if (m_index.find(std::make_pair(x + 1, y)) != m_index.end())
        {
            adjacent.down = m_index[std::make_pair(x + 1, y)];
        }
        if (m_index.find(std::make_pair(x, y - 1)) != m_index.end())
        {
            adjacent.left = m_index[std::make_pair(x, y - 1)];
        }
        if (m_index.find(std::make_pair(x, y + 1)) != m_index.end())
        {
            adjacent.right = m_index[std::make_pair(x, y + 1)];
        }

        m_adjacent.push_back(adjacent);
    }
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
        {
            indexs.push_back(i);
        }
    }
    return indexs;
}

std::array<int, 4> BlockMapResource::adjacent(const int index)
{
    int x = m_rect[index].x / 2048 + 1;
    int y = m_rect[index].y / 2048 + 1;
    std::array<int, 4> result = {
        m_index[std::make_pair(x - 1, y)],
        m_index[std::make_pair(x + 1, y)],
        m_index[std::make_pair(x, y - 1)],
        m_index[std::make_pair(x, y + 1)]};
    return result;
}

cv::Mat BlockMapResource::getMap(const cv::Rect &rect)
{
    // 初始化地图图片
    cv::Mat map = cv::Mat::zeros(rect.size(), CV_8UC3);
    auto indexs = find_indexs(rect);
    getMap(map, rect, indexs);
    return map;
}

cv::Mat BlockMapResource::getAllMap()
{
    return getMap(min_rect);
}

#include <future>
void BlockMapResource::getMap(const cv::Mat &mat, const cv::Rect &rect, const std::vector<int> indexs)
{
    // 并发实现
    std::list<std::future<void>> futures;
    for (auto &index : indexs)
    {
        // 取交集
        cv::Rect r = rect & m_rect[index];
        // 如果交集面积为0，直接返回
        if (r.area() == 0)
            continue;
        // 获取相对于地图图片的范围
        cv::Rect r1 = r - m_rect[index].tl();
        // 获取相对于区块图片的范围
        cv::Rect r2 = r - rect.tl();
        futures.emplace_back(std::async(std::launch::async, [r1, r2, index, this, &mat]
                                        { m_map[index](r1).copyTo(mat(r2)); }));
    }
    for (auto &f : futures)
    {
        f.get();
    }
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

    //// getMap
    // auto start = std::chrono::steady_clock::now();
    // for(int i = 0; i < 10; i++)
    //{
    //     mat = quadTree.getMap(cv::Rect(5000+i, 3000+i, 200, 200));
    // }
    // auto end = std::chrono::steady_clock::now();
    // std::chrono::duration<double> diff = end - start;
    // std::cout << "getMap: " << diff.count() << " s" << std::endl;

    //// getMap_1st
    // start = std::chrono::steady_clock::now();
    // for(int i = 0; i < 10; i++)
    //{
    //     mat = quadTree.getMap_1st(cv::Rect(5000+i, 3000+i, 200, 200));
    // }
    // end = std::chrono::steady_clock::now();
    // diff = end - start;
    // std::cout << "getMap_1st: " << diff.count() << " s" << std::endl;
}

int main(int argc, char **argv)
{
    std::cout << "Hello World!" << std::endl;
    test_1st();
    return 0;
}