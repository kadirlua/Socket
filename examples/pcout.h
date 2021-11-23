#pragma once
#include <iostream>
#include <sstream>
#include <mutex>

struct pcout : std::stringstream {
	~pcout() {
		std::lock_guard<std::mutex> lock_(mtx_);
		std::cout << rdbuf();
		std::cout.flush();
	}
	static std::mutex mtx_;
};