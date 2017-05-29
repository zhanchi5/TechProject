#include <iostream>
#include "json.hpp"
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <cstddef>
#include <vector>

using nlohmann::json;

int fileSize;

std::string formattedJson(char *json, unsigned int files_or_commits)
{
	std::string pretty;

	if (json == NULL || strlen(json) == 0)
	{
		return pretty;
	}

	std::string str = std::string(json);
	bool        quoted = false;
	bool        escaped = false;
	std::string INDENT = "    ";
	int         indent = 0;
	int         length = (int)str.length();
	int         i;

	for (i = 0; i < length; i++)
	{
		char ch = str[i];

		switch (ch)
		{
		case '{':
		case '[':
			pretty += ch;

			if (!quoted)
			{
				pretty += "\n";

				if (!(str[i + 1] == '}' || str[i + 1] == ']'))
				{
					++indent;

					for (int j = 0; j < indent; j++)
					{
						pretty += INDENT;
					}
				}
			}

			break;

		case '}':
		case ']':
			if (!quoted)
			{
				if ((i > 0) && (!(str[i - 1] == '{' || str[i - 1] == '[')))
				{
					pretty += "\n";

					--indent;

					for (int j = 0; j < indent; j++)
					{
						pretty += INDENT;
					}
				}
				else if ((i > 0) && ((str[i - 1] == '[' && ch == ']') || (str[i - 1] == '{' && ch == '}')))
				{
					for (int j = 0; j < indent; j++)
					{
						pretty += INDENT;
					}
				}
			}

			pretty += ch;

			break;

		case '"':
			pretty += ch;
			escaped = false;

			if (i > 0 && str[i - 1] == '\\')
			{
				escaped = !escaped;
			}

			if (!escaped)
			{
				quoted = !quoted;
			}

			break;

		case ',':
			pretty += ch;

			if (!quoted)
			{
				pretty += "\n";

				for (int j = 0; j < indent; j++)
				{
					pretty += INDENT;
				}
			}

			break;

		case ':':
			pretty += ch;

			if (!quoted)
			{
				pretty += " ";
			}

			break;
		default:
			pretty += ch;

			break;
		}

	}
	std::size_t found;
	if (files_or_commits == 2)
		found = pretty.find_last_of(']');
	else
		found = pretty.find_last_of('}');
	pretty.erase(found + 1, EOF);
	return pretty;
}

static void getJSON(char* url, char* filename) {
	CURL  *curl; // указатель на объект cURL
	curl = curl_easy_init(); // запуск "легкого" хэндлера
	CURLcode res; // объект класса cURLcode

	if (curl) {
		struct curl_slist *chunk = NULL; // список на ноль
		chunk = curl_slist_append(chunk, "User-Agent: David"); // кастомный хэдер USERAGENT в список хэдеров
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk); // список отправляется в хэдер запроса
		curl_easy_setopt(curl, CURLOPT_URL, url); // ссылка, по которой обращаемся

		FILE *pF;
		pF = fopen(filename, "wb");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pF); // команда на копирование данных в файл

		res = curl_easy_perform(curl); // выполнение запросов 

		if (res != CURLE_OK) // проверка
			fprintf(stderr, "Could not download file: %s\n",
				curl_easy_strerror(res));
		else printf("SUCCESS: File has been dowloaded\n");

		rewind(pF);
		fseek(pF, 0, SEEK_END);
		fileSize = ftell(pF); // получение размера файла
		curl_easy_cleanup(curl);
		fclose(pF);
	}
}

void toJSON(json& j, char* filename, unsigned int files_or_commits) {
	std::ifstream fromFile(filename);
	char* line = new char[fileSize];
	while (!fromFile.eof()) {
		for (unsigned int i = 0; i <= fileSize; ++i) {
			fromFile >> line[i];
		}
	}
	fromFile.close();

	std::ofstream toFile(filename);
	toFile << formattedJson(line, files_or_commits);
	toFile.close();

	std::ifstream finFile(filename);
	j << finFile;

	finFile.close();
}


namespace ns {
	unsigned int commitAmount;
	struct tree {
		std::vector<std::string> files;
		std::vector<tree*> sub_trees;
		tree(){}
		tree(const json& jas) {
			char appendix[] = { "?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" };
			if (jas["truncated"].get<bool>() == 0)
			{
				for (auto i = 0; i < jas["tree"].size(); ++i)
				{
					std::cout << jas["tree"][i]["type"].get<std::string>() << std::endl;
					if (jas["tree"][i]["type"].get<std::string>() == "tree")
					{
						json jbuf;
						char* buf_url = new char[(jas["tree"][i]["url"].get<std::string>()).length() + strlen(appendix) + 1];
						for (unsigned int j = 0; j < (jas["tree"][i]["url"].get<std::string>()).length(); ++j)
						{
							buf_url[j] = (jas["tree"][i]["url"].get<std::string>())[j];
						}
						for (unsigned int j = (jas["tree"][i]["url"].get<std::string>()).length(); j < (jas["tree"][i]["url"].get<std::string>()).length() + strlen(appendix); ++j)
						{
							buf_url[j] = appendix[j - (jas["tree"][i]["url"].get<std::string>()).length()];
						}
						buf_url[(jas["tree"][i]["url"].get<std::string>()).length() + strlen(appendix)] = '\0';
						getJSON(buf_url, "tree_fill_buf.json");
						toJSON(jbuf, "tree_fill_buf.json", 1);
						delete[] buf_url;
						tree* subdir = new tree(jbuf);
						sub_trees.push_back(subdir);
					}
					else
						files.push_back(jas["tree"][i]["path"].get<std::string>());
				}
			}
			else
			{
				for (auto i = 0; i < jas["tree"].count("path"); ++i)
				{
					files.push_back(jas["tree"][i]["path"].get<std::string>());
				}
				sub_trees.push_back(nullptr);
			}
		}
	};
	void fill_tree(tree& commit_tree, std::string initial_url)
	{
		char appendix[] = { "?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" };
		char* init_url = new char[initial_url.length() + strlen(appendix) + 1];
		for (unsigned int j = 0; j < initial_url.length(); ++j)
		{
			init_url[j] = initial_url[j];
		}
		for (unsigned int j = initial_url.length(); j < initial_url.length() + strlen(appendix); ++j)
		{
			init_url[j] = appendix[j - initial_url.length()];
		}
		init_url[initial_url.length() + strlen(appendix)] = '\0';
		
		json jas;
		getJSON(init_url, "init_buf.json");
		toJSON(jas, "init_buf.json", 1);
		
		if (jas["truncated"].get<bool>() == 0)
		{
			for (auto i = 0; i < jas["tree"].size(); ++i)
			{
				std::cout << jas["tree"][i]["type"].get<std::string>() << std::endl;
				if (jas["tree"][i]["type"].get<std::string>() == "tree")
				{
					json jbuf;
					char* buf_url = new char[(jas["tree"][i]["url"].get<std::string>()).length() + strlen(appendix) + 1];
					for (unsigned int j = 0; j < (jas["tree"][i]["url"].get<std::string>()).length(); ++j)
					{
						buf_url[j] = (jas["tree"][i]["url"].get<std::string>())[j];
					}
					for (unsigned int j = (jas["tree"][i]["url"].get<std::string>()).length(); j < (jas["tree"][i]["url"].get<std::string>()).length() + strlen(appendix); ++j)
					{
						buf_url[j] = appendix[j - (jas["tree"][i]["url"].get<std::string>()).length()];
					}
					buf_url[(jas["tree"][i]["url"].get<std::string>()).length() + strlen(appendix)] = '\0';
					getJSON(buf_url, "tree_fill_buf.json");
					toJSON(jbuf, "tree_fill_buf.json", 1);
					delete[] buf_url;
					tree* subdir = new tree(jbuf);
					commit_tree.sub_trees.push_back(subdir);
				}
				else
				{
					commit_tree.files.push_back(jas["tree"][i]["path"].get<std::string>());
				}
			}
		}
		else
		{
			for (auto i = 0; i < jas["tree"].count("path"); ++i)
			{
				commit_tree.files.push_back(jas["tree"][i]["path"].get<std::string>());
			}
			commit_tree.sub_trees.push_back(nullptr);
		}
		delete[] init_url;
	}
	
	struct commit {
		std::string sha;
		std::string login;
		std::string date;
		std::string message;
		std::string tree_sha;
		std::string tree_url;
		std::map<std::string, commit*> parents;
		tree commit_tree;
		commit(const json& j, unsigned int n) {
			this->sha = j[n]["sha"].get<std::string>();
			this->login = j[n]["author"]["login"].get<std::string>();
			this->date = j[n]["commit"]["author"]["date"].get<std::string>();
			this->message = j[n]["commit"]["message"].get<std::string>();
			this->tree_sha = j[n]["commit"]["tree"]["sha"].get<std::string>();
			this->tree_url = j[n]["commit"]["tree"]["url"].get<std::string>();
		}

	};
	std::vector<commit*> commitList;
	commit* find_parents(std::string parent_sha) {
		for (unsigned int i = 0; i != (commitList.end() - commitList.begin()); ++i) {
			if ((parent_sha == (commitList[i])->sha)) {
				return commitList[i];
			}
		}
		return nullptr;
	}
	void get_parents(const json& j, unsigned int n, commit* c) {
		for (auto i = j[n]["parents"].begin(); i != j[n]["parents"].end(); ++i) {
			c->parents.insert(std::pair<std::string, commit*>((*i)["sha"].get<std::string>(), find_parents((*i)["sha"].get<std::string>())));
		}
	}

	void pushtoList(json& j, unsigned int n) {
		if (n > 25) {
			for (unsigned int i = 0; i < 25; ++i) {
				commit* buf = new ns::commit(j, i);
				ns::commitList.push_back(buf);
			}
			for (unsigned int i = 0; i < 25; ++i) {
				get_parents(j, i, ns::commitList[i]);
				ns::fill_tree((ns::commitList[i])->commit_tree, (ns::commitList[i])->tree_url);
			}
		}
		else {
			for (unsigned int i = 0; i < n; ++i) {
				commit* buf = new ns::commit(j, i);
				ns::commitList.push_back(buf);
			}
			for (unsigned int i = 0; i < n; ++i) {
				get_parents(j, i, ns::commitList[i]);
				ns::fill_tree((ns::commitList[i])->commit_tree, (ns::commitList[i])->tree_url);
			}
		}
	}
	void getcommitAmount(json& j) {
		commitAmount = j[0]["contributions"].get<unsigned int>(); 
	}

	

}


void tree_cleanup(ns::tree mytree)
{
	for (unsigned int i = 0; i < mytree.sub_trees.size(); ++i)
		if (mytree.sub_trees[0] == nullptr)
			delete mytree.sub_trees[0];
		else
		{
			tree_cleanup(*(mytree.sub_trees[i]));
			delete mytree.sub_trees[i];
		}
}