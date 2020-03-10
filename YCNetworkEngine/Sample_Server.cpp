#include "pch.h"

#include "YCServer.h"
#include "YC_LOG.h"
#include "YCMempool.h"
#include "YCServer.h"
#include "YCPacket.h"
#include "YCSync.h"




#pragma pack(push, 1)
struct test_t
{
	char c[100];
};
#pragma pack(pop)

struct sesstion_t
{
	int id;
};

int main()
{
	std::unordered_map<int, sesstion_t> clients;

	ioev::Map<test_t>().To<0>();


	YCServer s(51234,
		[&](int id) {
			clients[id] = sesstion_t { id };
			yc::log("connect client! [{}]", id);
		},
		[&](int id) {
			clients.erase(id);
			yc::log("disconnect client! [{}]", id);
		}
	);

	ioev::Signal<test_t>([&s, &clients](test_t* d, int id) {
		yc::log("[client {}] : {}", id, d->c);

		auto t = *d;
		auto ID = id;
		s.Job->Add([&s, &clients, t, ID] {
			auto str = fmt::format("client {} : {}", ID, t.c);
			
			char* c = (char*)t.c;
			str.copy(c, str.size());
			c[str.size()] = '\0';


			for (auto& i : clients)
			{
				s.Send(i.first, &t);
			}
		});
	});


	s.Srv_Start();



	return 0;
}