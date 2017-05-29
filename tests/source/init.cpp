#include <testclass.hpp>
#include <catch.hpp>

SCENARIO ("check","[!mayfail]")
{
	char url1[] = { "https://api.github.com/repos/DavidTsyg/Sem2_Lab7/commits?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" };
	char filename1[] = {"repo.json"};
	char url2[] = { "https://api.github.com/repos/DavidTsyg/Sem2_Lab7/contributors?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" };
	char filename2[] = {"count.json"};
	
	json j1, j2;
	
	getJSON(url1, filename1);
	toJSON(j1, filename1, 2);
	getJSON(url2, filename2);
	toJSON(j2, filename2, 2);
	ns::getcommitAmount(j2);
	ns::pushtoList(j1, ns::commitAmount);
	
	REQUIRE (ns::commitList.size() == 25);
}

SCENARIO ("find_parents","[!mayfail]") 
{ 
	char url1[] = { "https://api.github.com/repos/DavidTsyg/Sem2_Lab7/commits?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" }; 
	char filename1[] = {"repo.json"}; 

	json j1; 
	ns::commit *a; 

	getJSON(url1, filename1); 
	toJSON(j1, filename1, 2); 


	std::string shatmp = {"fe232265ca5d35107133c0656ccff49e966b18f0"};
	a = ns::find_parents(shatmp); 

	REQUIRE (a == nullptr); 
}

SCENARIO ("get_parents","[!mayfail]") 
{ 
	char url1[] = { "https://api.github.com/repos/DavidTsyg/Sem2_Lab7/commits?client_id=fcca22e4d885095a01e1&client_secret=5865c57c862238c4e83ea6f97e86ce24e85368fd" }; 
	char filename1[] = {"repo.json"}; 


	json j1; 
	ns::commit c(j1, 0); 
	ns::commit *a = new ns::commit(j1,1); 

	getJSON(url1, filename1); 
	toJSON(j1, filename1, 2); 
	ns::get_parents(j1, 25, a);
	REQUIRE (c.sha == a->parents.begin()->first); 
}