#include <random>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <tuple>

// #pragma GCC optimize("Ofast","inline","fast-math","unroll-loops","no-stack-protector")
#pragma GCC optimize("inline", "unroll-loops", "no-stack-protector")
#pragma GCC target("sse,sse2,ssse3,sse4,popcnt,abm,mmx,avx,avx2,tune=native", "f16c")

static auto _ = []()
{std::ios_base::sync_with_stdio(false);std::cin.tie(nullptr);std::cout.tie(nullptr);return 0; }();

std::random_device rd;
std::mt19937 gen(rd());

// Define a struct for coordinates
struct Coordinate
{
    int x;
    int y;
};

// Define constants
constexpr int L = 1024; // side length of the square lattice
constexpr double SIGMA = 0.5;
constexpr double THETA = 0.01;
constexpr int STEPS_PER_LATTICEPOINT = 4000;
constexpr int RECORDING_STEP = 3 * STEPS_PER_LATTICEPOINT / 4;
constexpr int RECORDING_INTERVAL = 20;

constexpr int EMPTY = 0;
constexpr int GREEN_NUTRIENT = 1;
constexpr int BLUE_NUTRIENT = 2;
constexpr int SOIL = 3;
constexpr int GREEN_WORM = 4;
constexpr int BLUE_WORM = 5;

// Define distributions
std::uniform_int_distribution<> dis(0, 1);
std::uniform_int_distribution<> dis_site(0, 5);
std::uniform_int_distribution<> dis_l(0, L - 1);
std::uniform_real_distribution<> dis_real(0.0, 1.0);

Coordinate get_random_neighbour(Coordinate c, int L)
{
    // choose a random coordinate to change
    int coord_changing = dis(gen);
    // choose a random direction to change the coordinate
    int change = 2 * dis(gen) - 1;
    // change the coordinate
    c.x = (coord_changing == 0) ? (c.x + change + L) % L : c.x;
    c.y = (coord_changing == 1) ? (c.y + change + L) % L : c.y;

    return c;
}

std::vector<std::vector<int>> init_lattice(int L)
{
    std::vector<std::vector<int>> soil_lattice(L, std::vector<int>(L));
    for (int i = 0; i < L; ++i)
    {
        for (int j = 0; j < L; ++j)
        {
            soil_lattice[i][j] = dis_site(gen);
        }
    }
    return soil_lattice;
}
void update(std::vector<std::vector<int>> &soil_lattice, int L, double sigma, double theta,
            int *soil_production, int *empty_production, int *green_nutrient_production, int *blue_nutrient_production, int *green_production, int *blue_production)
{
    // select a random site
    Coordinate site = {dis_l(gen), dis_l(gen)};

    int site_value = soil_lattice[site.x][site.y];

    if (site_value == EMPTY || site_value == GREEN_NUTRIENT || site_value == BLUE_NUTRIENT)
    { // empty or nutrient
        // choose a random neighbour
        Coordinate nbr = get_random_neighbour(site, L);
        if (soil_lattice[nbr.x][nbr.y] == SOIL)
        { // if neighbour is soil
            // fill with soil-filling rate
            if (dis_real(gen) < sigma)
            {
                soil_lattice[site.x][site.y] = SOIL;
                (*soil_production)++;
            }
        }
    }
    else if (site_value == GREEN_WORM || site_value == BLUE_WORM)
    { // worm or parasite
        // check for death
        if (dis_real(gen) < theta)
        {
            soil_lattice[site.x][site.y] = EMPTY;
            (*empty_production)++;
        }
        else
        {
            // move into a neighbour
            Coordinate new_site = get_random_neighbour(site, L);
            // check the value of the new site
            int new_site_value = soil_lattice[new_site.x][new_site.y];

            // move the worm or parasite
            soil_lattice[new_site.x][new_site.y] = soil_lattice[site.x][site.y];
            soil_lattice[site.x][site.y] = EMPTY;

            // check if a blue worm moves into green nutrient
            if (new_site_value == GREEN_NUTRIENT && site_value == BLUE_WORM)
            {
                // reproduce behind you
                soil_lattice[site.x][site.y] = soil_lattice[new_site.x][new_site.y];
                (*blue_production)++;
            }
            // check if a green worm moves into blue nutrient
            if (new_site_value == BLUE_NUTRIENT && site_value == GREEN_WORM)
            {
                // reproduce behind you
                soil_lattice[site.x][site.y] = soil_lattice[new_site.x][new_site.y];
                (*green_production)++;
            }

            // check if the new site is soil
            else if (new_site_value == SOIL)
            {
                // leave nutrient behind
                if (site_value == GREEN_WORM)
                {
                    soil_lattice[site.x][site.y] = GREEN_NUTRIENT;
                    (*green_nutrient_production)++;
                }
                else if (site_value == BLUE_WORM)
                {
                    soil_lattice[site.x][site.y] = BLUE_NUTRIENT;
                    (*blue_nutrient_production)++;
                }
            }
            // check if the new site is a worm or parasite
            else if (new_site_value == GREEN_WORM || new_site_value == BLUE_WORM)
            {
                // keep both with worms/parasites (undo the vacant space in original site)
                soil_lattice[site.x][site.y] = new_site_value;
            }
        }
    }
}

std::tuple<int, int, int, int, int> count_soil_boundaries(const std::vector<std::vector<int>> &soil_lattice)
{
    int soil_nonsoil_boundaries = 0;
    int soil_greenworm_boundaries = 0;
    int soil_blueworm_boundaries = 0;
    int soil_greennutrient_boundaries = 0;
    int soil_bluenutrient_boundaries = 0;

    for (int i = 0; i < L; ++i)
    {
        for (int j = 0; j < L; ++j)
        {
            if (soil_lattice[i][j] == SOIL)
            {
                // Check right neighbor
                if (soil_lattice[i][(j + 1) % L] != SOIL)
                {
                    soil_nonsoil_boundaries++;
                    if (soil_lattice[i][(j + 1) % L] == GREEN_WORM)
                    {
                        soil_greenworm_boundaries++;
                    }
                    else if (soil_lattice[i][(j + 1) % L] == GREEN_NUTRIENT)
                    {
                        soil_greennutrient_boundaries++;
                    }
                    else if (soil_lattice[i][(j + 1) % L] == BLUE_WORM)
                    {
                        soil_blueworm_boundaries++;
                    }
                    else if (soil_lattice[i][(j + 1) % L] == BLUE_NUTRIENT)
                    {
                        soil_bluenutrient_boundaries++;
                    }
                }

                // Check bottom neighbor
                if (soil_lattice[(i + 1) % L][j] != SOIL)
                {
                    soil_nonsoil_boundaries++;
                    if (soil_lattice[(i + 1) % L][j] == GREEN_WORM)
                    {
                        soil_greenworm_boundaries++;
                    }
                    else if (soil_lattice[(i + 1) % L][j] == GREEN_NUTRIENT)
                    {
                        soil_greennutrient_boundaries++;
                    }
                    else if (soil_lattice[(i + 1) % L][j] == BLUE_WORM)
                    {
                        soil_blueworm_boundaries++;
                    }
                    else if (soil_lattice[(i + 1) % L][j] == BLUE_NUTRIENT)
                    {
                        soil_bluenutrient_boundaries++;
                    }
                }
            }
        }
    }

    return std::make_tuple(soil_nonsoil_boundaries, soil_greennutrient_boundaries, soil_bluenutrient_boundaries, soil_greenworm_boundaries, soil_blueworm_boundaries);
}

void run_csd(std::ofstream &file, double sigma, double theta)
{
    std::vector<std::vector<int>> soil_lattice = init_lattice(L);

    int soil_production = 0;
    int empty_production = 0;
    int green_nutrient_production = 0;
    int blue_nutrient_production = 0;
    int green_worm_production = 0;
    int blue_worm_production = 0;

    for (int step = 0; step <= STEPS_PER_LATTICEPOINT; ++step)
    {
        for (int i = 0; i < L * L; ++i)
        {
            update(soil_lattice, L, sigma, theta, &soil_production, &empty_production, &green_nutrient_production, &blue_nutrient_production, &green_worm_production, &blue_worm_production);
        }

        if ((step >= RECORDING_STEP) && (step % RECORDING_INTERVAL == 0))
        {
            int counts[6] = {0};
            for (const auto &row : soil_lattice)
            {
                for (int cell : row)
                {
                    ++counts[cell];
                }
            }
            double emptys = static_cast<double>(counts[EMPTY]) / (L * L);
            double green_nutrients = static_cast<double>(counts[GREEN_NUTRIENT]) / (L * L);
            double blue_nutrients = static_cast<double>(counts[BLUE_NUTRIENT]) / (L * L);
            double soil = static_cast<double>(counts[SOIL]) / (L * L);
            double green_worms = static_cast<double>(counts[GREEN_WORM]) / (L * L);
            double blue_worms = static_cast<double>(counts[BLUE_WORM]) / (L * L);

            double e_production = static_cast<double>(empty_production) / (L * L);
            double gn_production = static_cast<double>(green_nutrient_production) / (L * L);
            double bn_production = static_cast<double>(blue_nutrient_production) / (L * L);
            double s_production = static_cast<double>(soil_production) / (L * L);
            double gw_production = static_cast<double>(green_worm_production) / (L * L);
            double bw_production = static_cast<double>(blue_worm_production) / (L * L);

            int soil_nonsoil_boundaries, soil_greennutrient_boundaries, soil_bluenutrient_boundaries, soil_greenworm_boundaries, soil_blueworm_boundaries;
            std::tie(soil_nonsoil_boundaries, soil_greennutrient_boundaries, soil_bluenutrient_boundaries, soil_greenworm_boundaries, soil_blueworm_boundaries) = count_soil_boundaries(soil_lattice);

            file << step << "\t" << soil_nonsoil_boundaries << "\t" << soil_greennutrient_boundaries << "\t" << soil_bluenutrient_boundaries << "\t"
                 << soil_greenworm_boundaries << "\t" << soil_blueworm_boundaries << "\t"
                 << emptys << "\t" << green_nutrients << "\t" << blue_nutrients << "\t" << soil << "\t" << green_worms << "\t" << blue_worms << "\t"
                 << e_production << "\t" << gn_production << "\t" << bn_production << "\t" << s_production << "\t" << gw_production << "\t" << bw_production << "\n";
        }

        // Reset the production counters
        soil_production = 0;
        empty_production = 0;
        green_nutrient_production = 0;
        blue_nutrient_production = 0;
        green_worm_production = 0;
        blue_worm_production = 0;

        std::cout << "Progress: " << std::fixed << std::setprecision(2) << static_cast<double>(step) / STEPS_PER_LATTICEPOINT * 100 << "%\r" << std::flush;
    }
}

int main(int argc, char *argv[])
{
    double sigma = SIGMA;
    double theta = THETA;
    if (argc > 1)
        sigma = std::stod(argv[1]);
    if (argc > 2)
        theta = std::stod(argv[2]);

    std::string exePath = argv[0];
    std::string exeDir = std::filesystem::path(exePath).parent_path().string();
    std::ostringstream filePathStream;
    filePathStream << exeDir << "\\outputs\\sametheta\\soil_boundaries\\sigma_" << sigma << "_theta_" << theta << ".tsv";
    std::string filePath = filePathStream.str();

    std::ofstream file;
    file.open(filePath);
    file << "step\tsoil_nonsoil_boundaries\tsoil_greennutrient_boundaries\tsoil_bluenutrient_boundaries\tsoil_greenworm_boundaries\tsoil_blueworm_boundaries\t"
         << "emptys\tgreen_nutrients\tblue_nutrients\tsoil\tgreen_worms\tblue_worms\t"
         << "empty_production\tgreen_nutrient_production\tblue_nutrient_production\tsoil_production\tgreen_worm_production\tblue_worm_production\n";

    run_csd(file, sigma, theta);

    file.close();

    return 0;
}