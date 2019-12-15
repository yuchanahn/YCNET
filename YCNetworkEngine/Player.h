#pragma once
#include "GameObject.h"

class Player : public GameObject
{
public:
	char name[50];
	int Hp;
	int Mp;
	int Str;
	int Dex;
	int Int;
	int Cri;


	void NetworkSend();
public:
	void Hit(int Dmg);

	
};

