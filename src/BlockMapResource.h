
#include <map>
#include <regex>
#include <future>
#include <vector>
#include <numeric>
#include <filesystem>
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

// cv::Point less define
namespace std
{
    template <>
    struct less<cv::Point>
    {
        bool operator()(const cv::Point &a, const cv::Point &b) const
        {
            return a.x < b.x || (a.x == b.x && a.y < b.y);
        }
    };
} // namespace std

class BlockMapResource
{
public:
    BlockMapResource()
    {
        loadMap();
    }
    BlockMapResource(const std::filesystem::path &path, std::string target_name, cv::Point map_origin, cv::Point origin_index);
    ~BlockMapResource() = default;

    // 加载地图图片
    void loadMap();
    void insert(const cv::Mat &image, const int x, const int y)
    {
        m_map.push_back(image);
        m_rects.push_back(cv::Rect((-y - 1) * 2048, (-x - 1) * 2048, 2048, 2048));
        m_index[cv::Point(x, y)] = static_cast<int>(m_map.size()) - 1;
        min_rect = gen_bounding_rect();
        // m_origin_index = cv::Point(std::min(m_origin_index.x, x), std::min(m_origin_index.y, y));
    }

    // 获取地图图片
    cv::Mat view() { return view_abs(gen_bounding_rect()); }
    cv::Mat view(const cv::Rect &rect) { return view(rect, find_indexs(rect)); }
    cv::Mat view_abs(const cv::Rect &rect) { return view_abs(rect, find_indexs_abs(rect)); }
    cv::Mat view(const cv::Rect &rect, const std::vector<int> index_s)
    {
        return view_abs(to_abs(rect), index_s);
    }
    cv::Mat view_abs(const cv::Rect &rect, const std::vector<int> index_s)
    {
        cv::Mat map = cv::Mat::zeros(rect.size(), CV_8UC3);
        for (auto &index : index_s)
            map = view_abs(rect, index, map);
        return map;
    }
    cv::Mat view_async(const cv::Rect &rect, const std::vector<int> index_s);

    cv::Mat view_abs(const cv::Rect &rect, const int index, cv::Mat map = cv::Mat())
    {
        if (map.empty())
            map = cv::Mat::zeros(rect.size(), CV_8UC3);
        cv::Rect r = rect & m_rects[index];
        // 如果交集面积为0，直接返回
        if (r.area() == 0)
            return map;
        // 获取相对于地图图片的范围
        cv::Rect r1 = r - m_rects[index].tl();
        // 获取相对于区块图片的范围
        cv::Rect r2 = r - rect.tl();
        m_map[index](r1).copyTo(map(r2));
        return map;
    }
    // 图片大小归一化
    void
    size_normalize(cv::Mat &img)
    {
        if (img.cols == 2048 && img.rows == 2048)
            return;
        // 为了防止边缘出现淡化现象，放大时使用 INTER_NEAREST 插值
        cv ::resize(img, img, cv::Size(2048, 2048), 0, 0, (img.cols > 512 || img.rows > 512) ? cv::INTER_NEAREST : cv::INTER_CUBIC);
    }

    std::optional<std::pair<std::string, cv::Point>> parse_file_name(const std::string &name, const std::string &target_name)
    {
        std::regex reg("UI_" + target_name + "_(-?\\d+)_(-?\\d+).png");
        std::smatch result;
        if (std::regex_match(name, result, reg) == false)
            return std::nullopt;
        int x = std::stoi(result[1].str());
        int y = std::stoi(result[2].str());
        return std::make_pair(name, cv::Point(x, y));
    }

private:
    cv::Rect gen_bounding_rect()
    {
        return std::reduce(m_rects.begin(), m_rects.end(), cv::Rect(), [](const cv::Rect &a, const cv::Rect &b)
                           { return a | b; });
    }

    std::vector<int> find_indexs(const cv::Rect &rect) { return find_indexs_abs(to_abs(rect)); }
    std::vector<int> find_indexs_abs(const cv::Rect &rect);
    cv::Mat get_map(const cv::Rect &rect, const std::vector<int> indexs);

private:
    // 原点图片在vector中的索引
    cv::Point m_origin_index;
    // 图片原点位置，相对于地图图片左上角
    cv::Point m_origin;
    // 地图原点位置，相对于图片原点
    cv::Point m_map_origin;
    // 地图原点在图片左上角的绝对位置
    cv::Point abs_origin() { return m_origin + m_map_origin; }
    cv::Point to_abs(const cv::Point &p) { return p + abs_origin(); }
    cv::Rect to_abs(const cv::Rect &r) { return r + abs_origin(); }

private:
    // 存储地图图片
    std::vector<cv::Mat> m_map;
    // 存储地图图片对应的地图范围
    std::vector<cv::Rect> m_rects;
    // 存储地图图片对应的地图区块位置在vector中的索引
    std::map<cv::Point, int> m_index;

    cv::Rect min_rect;
};
BlockMapResource::BlockMapResource(const std::filesystem::path &path, std::string target_name, cv::Point map_origin, cv::Point origin_index)
    : m_map_origin(map_origin), m_origin_index(origin_index)
{
    if (std::filesystem::exists(path) == false)
        return; // 文件夹不存在

    std::vector<std::pair<std::string, cv::Point>> target_files;
    for (auto &p : std::filesystem::directory_iterator(path))
    {
        auto result = parse_file_name(p.path().filename().string(), target_name);
        if (result.has_value())
            target_files.push_back(result.value());
    }
    for (auto &[name, xy] : target_files)
    {
        auto [x, y] = xy;
        cv::Mat img = cv::imread((path / name).string());
        size_normalize(img);
        insert(img, x, y);
    }

    m_origin = m_rects[m_index[origin_index]].tl() + cv::Point(1024, 1024);
}

std::vector<int> BlockMapResource::find_indexs_abs(const cv::Rect &rect)
{
    if (auto r = rect & min_rect; r.area() == 0)
        return {};
    // 直接遍历所有区块，获取存在交集的区块的索引
    std::vector<int> indexs;
    for (int i = 0; i < m_rects.size(); i++)
    {
        cv::Rect r = rect & m_rects[i];
        if (r.area() > 0)
            indexs.push_back(i);
    }
    return indexs;
}

cv::Mat BlockMapResource::view_async(const cv::Rect &rect, const std::vector<int> indexs)
{
    cv::Mat map = cv::Mat::zeros(rect.size(), CV_8UC3);
    std::list<std::future<void>> futures;
    for (auto &index : indexs)
    {
        cv::Rect r = rect & m_rects[index];
        if (r.area() == 0)
            continue;
        cv::Rect r1 = r - m_rects[index].tl();
        cv::Rect r2 = r - rect.tl();
        futures.emplace_back(std::async(std::launch::async, [r1, r2, index, this, &map]
                                        { m_map[index](r1).copyTo(map(r2)); }));
    }
    for (auto &f : futures)
        f.get();
    return map;
}
