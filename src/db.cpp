#include "db.h"

using namespace std;


int main_test()
{
	cout << "Testing" << endl;
	btree<int, int64_t> mytree;
	mytree.insert(1, 1);
	mytree.insert(1, 3);

	int testmax = 100;

	for (int i = 2; i < testmax; i++)
	{
		mytree.insert(i, (int64_t)i*i);
		//cout << mytree.to_string() << endl;
	}
	for (int i = 1; i < 20; i++)
		cout << *mytree.find(i) << " ";
	cout << endl;
	for (int i = testmax - 20; i < testmax; i++)
		cout << *mytree.find(i) << " ";
	cout << endl;
	for (auto& val : mytree.find_range(3, 9))
		cout << val << " ";
	cout << endl;


	system("pause");
	return 0;
}

int main(int argc, char* argv[])
{
	cout << "Topkek" << endl;
	if (argc > 1)
	{
		if (strcmp(argv[1], "test") == 0)
			return main_test();
	}
	
	main_test();

	return 0;
}