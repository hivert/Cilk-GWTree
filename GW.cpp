#include <random>
#include <time.h>
#include <x86intrin.h>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <array>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>

#include "creducer_opadd_array.h"

using namespace std;

struct Random { // pad thread local random generators to avoid false sharing
    mt19937 gen;
    uniform_int_distribution<int> distr;
    char _pad[65536 - sizeof(gen) - sizeof(distr)];

    Random(mt19937 g) : gen(g), distr(0, 1) {};
    int flip() { return distr(gen); }
};

std::vector<Random> thread_rand;
const long int MAX_TREE = 100000;
std::array<uint8_t, MAX_TREE> tree;

bool reached = false;

void make_tree(long int i) {
    if (i >= MAX_TREE) {
        reached = true;
        return;
    }
    uint8_t coin = thread_rand[__cilkrts_get_worker_number()].flip();
    if (coin) {
        tree[i] = 1;
        cilk_spawn make_tree(2*i);
        make_tree(2*i + 1);
    }
}

int main(int argc, char *argv[]) {

    // Setup a vector of random generators
    for (int i=0; i < __cilkrts_get_nworkers(); i++) {
        thread_rand.push_back(Random(mt19937(clock() + i)));
    }
    for (int i=0; i < __cilkrts_get_nworkers(); i++) {
        cout << "Thread " << i << " : "
             << thread_rand[i].flip() << endl;
    }

    cilk_for (long int i = 0; i != MAX_TREE; i++) tree[i] = 0;

    make_tree(1);

    cilk::reducer< cilk::op_add<int> > r;
    cilk_for (long int i = 0; i != MAX_TREE; i++) *r += tree[i];
    cout << "Bound reached " << reached << endl;
    cout << "size : " << r.get_value() << endl;
    return 0;

/*
  std::chrono::high_resolution_clock::time_point tstart, tfin;

  tstart = std::chrono::high_resolution_clock::now();
  tfin = std::chrono::high_resolution_clock::now();
  auto tm = std::chrono::duration_cast<
    std::chrono::duration<double>>(tfin - tstart);

  std::cout << "n = " << std::setw(2) << n << "  ";
  std::cout << "time = " << std::fixed <<
    std::setw(9) << std::setprecision(6) << tm.count() << "s\n";
  std::cout << "Result: ";
  for(int i = 0; i < 16; i++)
    std::cout << res[i] << " ";
  std::cout << std::endl;
  return 0;
*/
}
