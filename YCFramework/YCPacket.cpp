#include "pch.h"
#include "YCPacket.h"


std::unordered_map<size_t, int> PacketEvent::packet_events;
std::unordered_map<int, std::list<std::function<void(void*,int)>>> PacketEvent::event;