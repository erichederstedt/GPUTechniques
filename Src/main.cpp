#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>

std::string load_file(std::string path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void save_to_file(const std::string& folder, const std::string& filename, const std::string& content) {
    // Step 1: Create the folder (if it doesn't exist)
    std::filesystem::create_directories(folder);  // Safe to call even if it already exists

    // Step 2: Build the full file path
    std::string full_path = folder + "/" + filename;

    // Step 3: Write the string to the file
    std::ofstream file(full_path);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << full_path << std::endl;
        return;
    }

    file << content;
    file.close();

    std::cout << "File saved to: " << full_path << std::endl;
}

size_t init_index(std::string& document, const std::string tag)
{
    size_t position = document.find(tag);
    if (position == std::string::npos) 
    {
        return SIZE_MAX;
    }

    document.erase(position, tag.size());
    return static_cast<int>(position);
}

struct Article
{
    std::string parent_path;
    std::string path;
    std::string name;
    std::string article;
};
struct Folder
{
    std::string name;
    std::vector<Article> articles;
};
std::vector<Folder> folders;

#include "yaml.hpp"
#pragma warning(disable: 4189)
std::string parse_article(std::string path, std::string title, std::string template_path = "template_article.html", bool in_folder = true)
{
    std::string template_article = load_file(template_path);
    size_t index_folders = init_index(template_article, "***folders***");
    size_t index_title = init_index(template_article, "***title***");
    size_t index_description = init_index(template_article, "***description***");
    size_t index_requirements = init_index(template_article, "***requirements***");
    size_t index_content = init_index(template_article, "***content***");

    std::string article_contents = load_file(path+".html");
    TINY_YAML::Yaml* coolYamlObject = nullptr;
    if (index_description != SIZE_MAX && index_requirements != SIZE_MAX)
        coolYamlObject = new TINY_YAML::Yaml(path+".yaml");

    if (index_content != SIZE_MAX)
        template_article.insert(index_content, article_contents);
    if (index_requirements != SIZE_MAX && coolYamlObject)
        template_article.insert(index_requirements, (*coolYamlObject)["requirements"].getData<std::string>());
    if (index_description != SIZE_MAX && coolYamlObject)
        template_article.insert(index_description, (*coolYamlObject)["description"].getData<std::string>());
    if (index_title != SIZE_MAX)
        template_article.insert(index_title, title);
    
    std::string shit_string;

    std::string in_folder_escape;
        if (in_folder)
            in_folder_escape = "../";

    shit_string.append("<div class=\"nav-link\">");
    shit_string.append("<button onclick=\"window.location.href='" + in_folder_escape + "index.html" + "';\">Introduction</button>");
    shit_string.append("</div>");

    size_t folder_count = 0;
    for (Folder& folder : folders)
    {
        shit_string.append("<div class=\"nav-folder\">\n");
        shit_string.append("<div class=\"folder-header\" onclick=\"toggleFolder('folder-content-" + std::to_string(folder_count) + "', this)\">\n");
        shit_string.append("<span id=\"arrow-" + std::to_string(folder_count) + "\">▼</span> " + folder.name + "\n");
        shit_string.append("</div>\n");
        shit_string.append("<div id=\"folder-content-" + std::to_string(folder_count) + "\" class=\"folder-content\">\n");
        for (Article& article : folder.articles)
        {
            shit_string.append(std::string("<button onclick=\"window.location.href='") + in_folder_escape + std::string(article.path + ".html") + std::string("';\">") + article.name + std::string("</button>"));
        }
        shit_string.append("</div>\n");
        shit_string.append("</div>\n");
        folder_count++;
    }
    template_article.insert(index_folders, shit_string);

    if(coolYamlObject)
        delete coolYamlObject;

    return template_article;
}

std::vector<std::string> get_folders()
{
    std::vector<std::string> folder_names;

    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
    {
        if (entry.is_directory() && entry.path().filename() != "generated") 
        {
            folder_names.push_back(entry.path().filename().string());
        }
    }

    return folder_names;
}

std::vector<std::string> get_files(const std::string& path) {
    std::vector<std::string> html_files;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".html") {
            std::string shit = entry.path().string();
            html_files.push_back(shit);
        }
    }

    return html_files;
}

int main(void) 
{
    for (std::string& folder_path : get_folders())
    {
        folders.push_back({});
        Folder& folder = folders.back();
        folder.name = folder_path;
        
        for (std::string& file : get_files(folder.name))
        {
            std::string parent_path;
            std::string path = file;
            std::string filename;
            {
                std::filesystem::path path_name = path;
                parent_path = path_name.parent_path().string();
                filename = path_name.filename().string();
                path = parent_path + "/" + filename;
                size_t index = filename.find_last_of('.');
                filename.erase(index, filename.size() - index);
            }
            path.erase(path.find_last_of('.'), sizeof(".html"));

            Article article;
            article.name = filename;
            article.path = path;
            article.parent_path = parent_path;
            folder.articles.push_back(article);
        }
    }

    for (Folder& folder : folders)
    {
        for (Article& article : folder.articles)
        {
            article.article = parse_article(article.path, article.name);
        }
        
        for (Article& article : folder.articles)
        {
            save_to_file("generated/" + article.parent_path, article.name + ".html", article.article);
        }
    }

    std::string index_article = parse_article("index", "Introduction", "template_index.html", false);
    save_to_file("generated/", "index.html", index_article);

    std::cout << "Hello World!\n";
    
    return 0;
}