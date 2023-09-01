#include <iostream>
#include <filesystem>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <meojson/include/json.hpp>

#include "string_convect.h"

static auto cache_dir = std::filesystem::path{"./cache/"};
static auto save_dir = std::filesystem::path{"./save/"};
cpr::Proxies proxies{{"http", "http://127.0.0.1:1080"}, {"https", "https://127.0.0.1:1080"}};
void init()
{
    if (!std::filesystem::exists(cache_dir))
    {
        std::filesystem::create_directory(cache_dir);
    }
    if (!std::filesystem::exists(save_dir))
    {
        std::filesystem::create_directory(save_dir);
    }
}
std::string format_filename(const std::string &filename)
{
    const std::array<char, 8> invalid_chars = {'?', '*', '\\', '/', ':', '|', '<', '>'};
    std::string new_filename;
    for (auto &c : filename)
    {
        if (std::find(invalid_chars.begin(), invalid_chars.end(), c) != invalid_chars.end())
        {
            new_filename += '_';
        }
        else
        {
            new_filename += c;
        }
    }
    return new_filename;
}
std::string url_to_cache_key(const std::string &url)
{
    std::string cache_key;
    return format_filename(url);
}

std::string url_down_text(const std::string &url, bool is_used_cache = false)
{
    if (is_used_cache)
    {
        auto cache_key = url_to_cache_key(url);
        auto cache_file = cache_dir / cache_key;
        if (std::filesystem::exists(cache_file))
        {
            spdlog::info("cache hit: {}", url);
            std::ifstream ifs(cache_file);
            std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
            return content;
        }
        spdlog::info("cache miss: {}", url);
    }

    cpr::Response r = cpr::Get(cpr::Url{url}, proxies);
    if (r.status_code != 200)
    {
        spdlog::error("url: {}, status_code: {}", url, r.status_code);
        return "";
    }
    if (is_used_cache)
    {
        auto cache_key = url_to_cache_key(url);
        auto cache_file = cache_dir / cache_key;
        std::ofstream ofs(cache_file);
        ofs << r.text;
    }
    return r.text;
}

std::optional<std::string> get_token()
{
    auto token_res = cpr::Post(
        cpr::Url{"http://ddns.minemc.top:13010/oauth/token?scope=all&grant_type=client_credentials"},
        cpr::Authentication{"client", "secret", cpr::AuthMode::BASIC},
        cpr::Header{
            {"User-Agent", "Apifox/1.0.0 (https://apifox.com)"},
            {"Accept", "*/*"},
            {"Host", "ddns.minemc.top:13010"},
            {"Connection", "keep-alive"}});
    if (token_res.status_code != 200)
    {
        spdlog::error("token_res.status_code: {}", token_res.status_code);
        return std::nullopt;
    }
    auto token_json_opt = json::parse(token_res.text);
    if (token_json_opt.has_value() == false)
    {
        spdlog::error("token_json_opt.has_value() == false");
        return std::nullopt;
    }
    auto token_json = token_json_opt.value();
    auto access_token = token_json["access_token"].as_string();
    return access_token;
}

std::vector<std::string> get_md5_list(std::string access_token)
{
    auto bz2_list_res = cpr::Get(
        cpr::Url{"http://ddns.minemc.top:13010/api/marker_doc/list_page_bz2_md5"},
        cpr::Header{
            {"User-Agent", "Apifox/1.0.0 (https://apifox.com)"},
            {"Authorization", "Bearer " + access_token},
            {"Accept", "*/*"},
            {"Host", "ddns.minemc.top:13010"},
            {"Connection", "keep-alive"}});
    if (bz2_list_res.status_code != 200)
    {
        spdlog::error("bz2_list_res.status_code: {}", bz2_list_res.status_code);
        return {};
    }
    auto bz2_list_json_opt = json::parse(bz2_list_res.text);
    if (bz2_list_json_opt.has_value() == false)
    {
        spdlog::error("bz2_list_json_opt.has_value() == false");
        return {};
    }
    auto bz2_list_json = bz2_list_json_opt.value();
    auto bz2_list = bz2_list_json["data"].as_array();
    std::vector<std::string> bz2_md5_list;
    for (auto &bz2_md5 : bz2_list)
    {
        bz2_md5_list.push_back(bz2_md5.as_string());
    }
    return bz2_md5_list;
}
std::optional<std::string> get_bz2_file(std::string access_token, int index)
{
    auto bz2_file_res = cpr::Get(
        cpr::Url{fmt::format("http://ddns.minemc.top:13010/api/marker_doc/list_page_bz2/{}", index)},
        cpr::Header{
            {"User-Agent", "Apifox/1.0.0 (https://apifox.com)"},
            {"Authorization", "Bearer " + access_token},
            {"Accept", "*/*"},
            {"Host", "ddns.minemc.top:13010"},
            {"Connection", "keep-alive"}});
    if (bz2_file_res.status_code != 200)
    {
        spdlog::error("bz2_file_res.status_code: {}", bz2_file_res.status_code);
        return std::nullopt;
    }
    return bz2_file_res.text;
}

std::optional<std::string> unpack_bz2(const std::string &bz2_content)
{
    auto bz2_file = cache_dir / "bz2_file";
    std::ofstream ofs(bz2_file);
    ofs << bz2_content;
    ofs.close();
    std::string cmd = fmt::format("bzip2.exe -d {}", bz2_file.string());
    std::system(cmd.c_str());
    std::ifstream ifs(bz2_file.string() + ".out");
    std::string file_content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    return file_content;
}
int main(int argc, char **argv)
{
    init();
    auto access_token_opt = get_token();
    if (access_token_opt.has_value() == false)
    {
        spdlog::error("access_token_opt.has_value() == false");
        return -1;
    }
    auto access_token = access_token_opt.value();
    if (access_token.empty())
    {
        spdlog::error("access_token.empty()");
        return -1;
    }
    spdlog::info("access_token: success");
    auto bz2_md5_list = get_md5_list(access_token);
    spdlog::info("bz2_md5_list.size(): {}", bz2_md5_list.size());

    auto bz2_file_content_opt = get_bz2_file(access_token, 0);
    if (bz2_file_content_opt.has_value() == false)
    {
        spdlog::error("bz2_file_content_opt.has_value() == false");
        return -1;
    }
    auto bz2_file_content = bz2_file_content_opt.value();
    spdlog::info("bz2_file_content: {}", bz2_file_content.size());
    // bzip2 -d file
    auto file_content_opt = unpack_bz2(bz2_file_content);
    if (file_content_opt.has_value() == false)
    {
        spdlog::error("file_content_opt.has_value() == false");
        return -1;
    }
    auto file_content = file_content_opt.value();
    spdlog::info("file_content: {}", file_content.size());
    // json
    auto json_opt = json::parse(file_content);
    if (json_opt.has_value() == false)
    {
        spdlog::error("json_opt.has_value() == false");
        return -1;
    }
    auto json = json_opt.value();

    return 0;
}