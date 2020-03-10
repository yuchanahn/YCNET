#include "pch.h"

#include "YCSend.h"
#include "YCClient.h"
#include "YC_LOG.h"

#define BUFSIZE 1024

#pragma pack(push, 1)
struct test_t
{
	char c[100];
};
#pragma pack(pop)

int main()
{
	ioev::Map<test_t>().To<0>();

	YCClient master;
	master.connect("127.0.0.1", 51234);

	ioev::Signal<test_t>([](auto d, int) {
		printf("%s\n", d->c);
	});

	std::thread th([&] {
		while (1)
		{
			int rt = master.read_packet();
			if (rt == -1)
			{
				yc::log("error");
			}
			else if (rt == 0)
			{
				//yc::log("disconnect!");
			}
		}
	});
	
	YCSend send_manager(master.get_socket());

	while (1)
	{
		test_t t;
		std::cin >> t.c;
		send_manager.send(t);
	}

	th.join();
	return 0;
}