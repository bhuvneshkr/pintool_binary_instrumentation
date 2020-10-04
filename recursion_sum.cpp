#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>


static unsigned long sum(unsigned long num)
{
	if (num != 0)
		return num + sum(num - 1);
	else
		return num;
}

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        std::cerr << "Usage: recursion_sum [num]" << std::endl;
        return 1;
    }

    unsigned long num = 100;
    if (argc == 2)
    {
        std::istringstream is;
        is.str(argv[1]);
        is >> num;
        if (!is)
        {
            std::cerr << "Illegal number '" << argv[1] << "'" << std::endl;
            return 1;
        }
    }

    unsigned long ans = sum(num);
    std::cout << "Sum upto number " << num << " is " << ans << std::endl;
    return 0;
}
