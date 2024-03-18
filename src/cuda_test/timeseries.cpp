#include <random>
#include <vector>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#pragma GCC optimize("inline", "unroll-loops", "no-stack-protector")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,avx2,tune=native", "f16c")

static auto _ = []()
{std::ios_base::sync_with_stdio(false);std::cin.tie(nullptr);std::cout.tie(nullptr);return 0; }();

// Define constants
constexpr double SIGMA = 1;
constexpr double THETA = 0.5;
constexpr int L = 2048; // 2^10 = 1024
constexpr long long STEPS_PER_LATTICEPOINT = 5000;
constexpr long long N_STEPS = STEPS_PER_LATTICEPOINT * L * L;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, 1);
std::uniform_int_distribution<> dis_site(0, L *L - 1);
std::uniform_int_distribution<> dis_dir(0, 3);
std::uniform_real_distribution<> dis_prob(0, 1);

std::vector<bool> initLattice(int L)
{

    std::vector<bool> soil_lattice(L * L);
    for (int i = 0; i < L * L; ++i)
    {
        soil_lattice[i] = dis(gen);
    }
    return soil_lattice;
}

struct Coordinate {
    int x;
    int y;
};

Coordinate get_random_neighbour(Coordinate site, int L) {
    int dir = dis_dir(gen);
    switch (dir) {
        case 0: // left
            return {(site.x - 1 + L) % L, site.y};
        case 1: // right
            return {(site.x + 1) % L, site.y};
        case 2: // above
            return {site.x, (site.y - 1 + L) % L};
        case 3: // below
            return {site.x, (site.y + 1) % L};
    }
    return site; // should never reach here
}

void updateLattice(std::vector<bool> &lattice) {
    // Choose a random site
    int site_index = dis_site(gen);
    Coordinate site = {site_index % L, site_index / L};

    if (lattice[site_index]) // if the site is occupied
    {
        // make it empty with rate THETA
        if (dis_prob(gen) < THETA)
        {
            lattice[site_index] = false;
        }
    }
    else // if the site is empty
    {
        // choose a random neighbour
        Coordinate nbr = get_random_neighbour(site, L);
        int nbr_index = nbr.y * L + nbr.x;

        // if the neighbour is occupied, make it occupied with rate SIGMA
        if (lattice[nbr_index] && dis_prob(gen) < SIGMA)
        {
            lattice[site_index] = true;
        }
    }
}

int countLattice(const std::vector<bool> &lattice)
{
    int count = 0;
    for (bool site : lattice)
    {
        if (site)
        {
            ++count;
        }
    }
    return count;
}

int main(int argc, char *argv[])
{
    std::vector<bool> lattice = initLattice(L);

    std::string exePath = argv[0];
    std::string exeDir = std::filesystem::path(exePath).parent_path().string();

    std::ostringstream filename;
    filename << exeDir << "/outputs/sigma_" << SIGMA << "_theta_" << THETA << "/L_" << L << "_bl_" << L << ".csv";

    std::filesystem::create_directories(exeDir + "/outputs/sigma_" + std::to_string(SIGMA) + "_theta_" + std::to_string(THETA));

    std::ofstream file(filename.str());

    for (long long step = 0; step < N_STEPS; ++step)
    {
        updateLattice(lattice);
        if (step % (L * L) == 0)
        {
            int count = countLattice(lattice);
            double fraction = static_cast<double>(count) / (L * L);
            file << fraction << "\n";
            std::cout << "Progress: " << std::fixed << std::setprecision(2) << static_cast<double>(step) / (N_STEPS - 1) * 100 << "%\r" << std::flush;
        }
    }

    file.close();

    return 0;
}