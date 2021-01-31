#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

const double PI = atan(1) * 4;

// Box-Muller method
vector<double> sample_normal_bm(unsigned num_samples) {
    unsigned N = (num_samples + 1)/2;

    vector<double> result;
    result.reserve(2*N);

    double u1; double u2;
    double rand_max = double(RAND_MAX);
    for (unsigned i = 0; i < N; i++) {
        u1 = double(rand())/rand_max;
        u2 = double(rand())/rand_max;
        result.push_back(sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2));
        result.push_back(sqrt(-2.0 * log(u2)) * cos(2.0 * PI * u1));
    }

    result.resize(num_samples);
    return result;
}

// Polar Marsiglia method
vector<double> sample_normal_pm(unsigned num_samples) {
    vector<double> result;
    result.reserve(num_samples);

    double u1; double u2;
    double v1; double v2;
    double w;
    double s;
    double rand_max = double(RAND_MAX);
    while (result.size() < num_samples) {
        u1 = double(rand())/rand_max;
        u2 = double(rand())/rand_max;

        v1 = 2*u1 - 1;
        v2 = 2*u2 - 1;
        w = v1*v1 + v2*v2;

        if (w < 1.0) {
            s = sqrt(-2.0 * log(w) / w);
            result.push_back(s * v1);
            result.push_back(s * v2);
        }
    }

    result.resize(num_samples);
    return result;
}

enum Method {PolarMarsiglia, BoxMuller};

vector<double> gen_normal_samples(
        unsigned num_samples, double mean, double variance, Method method) {
    vector<double> rs;
    switch (method) {
        case PolarMarsiglia:
            rs = sample_normal_pm(num_samples);
            break;
        case BoxMuller:
            rs = sample_normal_bm(num_samples);
            break;
    }
    double scale = sqrt(variance);
    for (auto i = 0; i < rs.size(); i++) {
        rs[i] = scale * rs[i] + mean;
    }
    return rs;
}

// Implementation of Kolmogorov-Smirnov test (simplest goodness of fit test)
// https://www.itl.nist.gov/div898/handbook/eda/section3/eda35g.htm
bool kolmogorov_smirnov_test(vector<double> f(unsigned)) {
    unsigned N = 10000;
    double sqrt2 = sqrt(2);
    auto rs = f(N);
    sort(rs.begin(), rs.end());

    // values from which test statistic will be determined
    vector<double> xs;
    xs.reserve(N);
    double fyi;
    for (double i = 0.0; i < double(N); i += 1.0) {
        fyi = 0.5 * (1.0 + erf(rs[i]/sqrt2));
        xs.push_back(abs(fyi - (i / double(N))));
    }
    auto max_res = max_element(xs.begin(), xs.end());
    double D = xs[distance(xs.begin(), max_res)];

    // http://people.cs.pitt.edu/~lipschultz/cs1538/prob-table_KS.pdf
    double crit = 1.63 / sqrt(N);

    if (D < crit) {
        return true;
    } else {
        return false;
    }
}

double test_method(vector<double> f(unsigned), unsigned num_repeats) {
    double good = 0.0;
    for (auto i = 0; i < num_repeats; i++) {
        good += double(kolmogorov_smirnov_test(f));
    }
    return good/double(num_repeats);
}

template<typename T>
void print_vector(vector<T> input) {
    for (auto i = 0; i < input.size(); i++) {
        cout << input[i] << " ";
    }
}

double measure_gen_rate(vector<double> f(unsigned)) {
    auto tp0 = system_clock::now();
    f(100000);
    auto tp1 = system_clock::now();
    microseconds time = duration_cast<microseconds>(tp1 - tp0);
    return 100000.0/double(time.count());
}

int main() {
    cout << "fraction good fits with 99% probability (BoxMuller): "
         << test_method(sample_normal_bm,1000) << "\n";
    cout << "fraction good fits 99% probability (PolarMarsiglia): "
         << test_method(sample_normal_pm, 1000) << "\n";


    cout << "samples/us (BoxMuller): "
         << measure_gen_rate(sample_normal_bm) << "\n";
    cout << "samples/us (PolarMarsiglia): "
         << measure_gen_rate(sample_normal_pm) << "\n";

    auto ns = gen_normal_samples(
            5, 0.0, 1.0, PolarMarsiglia);
    cout << "5-sample (Polar-Marsiglia): ";
    print_vector(ns);
    cout << "\n";

    ns = gen_normal_samples(5, 0.0, 1.0, BoxMuller);
    cout << "5-sample (using Box-Muller): ";
    print_vector(ns);
    cout << "\n";
}

