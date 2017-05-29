#include "testclass.hpp"
int main() {

	char url1[] = { "https://api.github.com/repos/DavidTsyg/Sem2_Lab7/commits?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" };
	char url2[] = { "https://api.github.com/repos/DavidTsyg/Sem2_Lab7/contributors?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" };

	//	Получаем все коммиты
	json j1, j2;

	getJSON(url1, "repo.json");
	toJSON(j1, "repo.json", 2);
	getJSON(url2, "count.json");
	toJSON(j2, "count.json", 2);
	ns::getcommitAmount(j2);
	ns::pushtoList(j1, ns::commitAmount);

	for (unsigned int i = 0; i < ns::commitList.size(); ++i) {
		std::cout << "Commit number: " << ns::commitList.size() - i << std::endl;
		std::cout << "Date: " << (ns::commitList[i])->date << std::endl;
		std::cout << "Author: " << (ns::commitList[i])->login << std::endl;
		std::cout << "Message: " << (ns::commitList[i])->message << std::endl;
		std::cout << "SHA: " << (ns::commitList[i])->sha << std::endl;
		std::cout << "Parents SHA" << std::endl;
		for (auto it = (ns::commitList[i])->parents.begin(); it != (ns::commitList[i])->parents.end(); ++it)
		{
		std::cout << it->first << std::endl ;
		if (it->second != nullptr)
		std::cout << "IT WORKS" << std::endl;
		}
		std::cout << "First file of tree:  "<<((ns::commitList[i])->commit_tree).files[0] << std::endl;
		std::cout << "--------------------------\n";
	} 
	///////////////////// END OF TESTING AREA //////////////////////////////
	for (unsigned int i = 0; i < ns::commitList.size(); ++i) {
		tree_cleanup(ns::commitList[i]->commit_tree);
		delete ns::commitList[i];
	}
	std::cout << std::endl;
	system("pause");
	return 0;
}