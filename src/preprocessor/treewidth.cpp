#include "treewidth.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <ostream>
#include <cstdlib>
#include <cassert>
#include <queue>
#include <algorithm>
#include <sstream>
#include <thread>

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include <boost/process.hpp>
#include <boost/asio.hpp>

#include "utils.hpp"

namespace sspp {

namespace decomp {

TreeDecomposition Treedecomp(const Graph& graph, double time) {
	int n = graph.n();
	if (n == 0) {
		TreeDecomposition dec(0, 0);
		return dec;
	}
	if (n == 1) {
		TreeDecomposition dec(1, 1);
		dec.SetBag(1, {0});
		return dec;
	}
	if (time < 0.099) {
		TreeDecomposition dec(1, n);
		vector<int> all;
		for (int i = 0; i < n; i++) {
			all.push_back(i);
		}
		dec.SetBag(1, all);
		return dec;
	}
	assert(n >= 2);
	auto es = graph.Edges();
	int m = es.size();
	boost::asio::io_context ctx;
	boost::asio::readable_pipe in{ctx};
	boost::asio::writable_pipe out{ctx};
	boost::process::process flow_cutter(ctx.get_executor(), "./flow_cutter_pace17", {}, boost::process::process_stdio{out, in, {}});

			// we need to give input to the child
	auto start = std::chrono::system_clock::now();
	boost::asio::write(out, boost::asio::buffer("p tw " + std::to_string(n) + " " + std::to_string(m) + "\n"));
	for (auto e : es) {
		boost::asio::write(out, boost::asio::buffer(std::to_string(e.F+1) + " " + std::to_string(e.S+1) + "\n"));
	}
	out.close();
	std::cout << "c o Primal edges "<< m << std::endl;

	std::string line;
	do {
		boost::asio::read_until(in, boost::asio::dynamic_buffer(line), '\n');
		if(line.find("c status", 0) != 0) {
			break;
		}
	} while(flow_cutter.running());
	auto passed = std::chrono::system_clock::now() - start;
	auto remaining = std::chrono::duration<double>(time) - passed;
	std::this_thread::sleep_for(remaining);
	flow_cutter.interrupt();
	std::string tmp;
	int claim_width = 0;
	TreeDecomposition dec(0, 0);
	boost::system::error_code ec;
	while (ec != boost::asio::error::eof) {
		line.clear();
		boost::asio::read_until(in, boost::asio::dynamic_buffer(line), '\n');
		std::cout << line << std::endl;
		std::stringstream ss(line);
		ss>>tmp;
		if (tmp == "c") continue;
		if (tmp == "s") {
			ss>>tmp;
			assert(tmp == "td");
			int bs,nn;
			ss>>bs>>claim_width>>nn;
			assert(nn == n);
			claim_width--;
			dec = TreeDecomposition(bs, nn);
		} else if (tmp == "b") {
			int bid;
			ss>>bid;
			vector<int> bag;
			int v;
			while (ss>>v) {
				bag.push_back(v-1);
			}
			dec.SetBag(bid, bag);
		} else {
			int a = stoi(tmp);
			int b;
			ss>>b;
			dec.AddEdge(a, b);
		}
	}
	in.close();
	auto status = flow_cutter.wait();
	assert(status >= 0);
	assert(dec.Width() <= claim_width);
	cout << "c o width " << dec.Width() << endl;
	assert(dec.Verify(graph));
	return dec;
}
} // namespace decomp
} // namespace sspp