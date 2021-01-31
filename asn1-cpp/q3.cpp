#include <cstdlib>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <sstream>
#include <random>
#include <functional>

using namespace std;
using namespace chrono;

tuple<double, microseconds> reference_summation(vector<float> nums) {
    auto tp0 = system_clock::now();
    double sum = 0.0;

    for (int i = 0; i < nums.size(); i++) {
        sum += double(nums[i]);
    }
    auto tp1 = system_clock::now();
    microseconds time = duration_cast<microseconds>(tp1 - tp0);

    return make_tuple(sum, time);
}

tuple<float, microseconds> plain_summation(vector<float> nums) {
    auto tp0 = system_clock::now();
    float sum = 0.0;


    for (int i = 0; i < nums.size(); i++) {
        sum += nums[i];
    }
    auto tp1 = system_clock::now();
    microseconds time = duration_cast<microseconds>(tp1 - tp0);

    return make_tuple(sum, time);
}

tuple<float, microseconds> compensated_summation(vector<float> nums) {
    auto tp0 = system_clock::now();
    float sum = 0.0;
    float c = 0.0; // running compensation

    float y;
    float t;

    for (int i = 0; i < nums.size(); i++) {
        y = nums[i] - c;
        t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    auto tp1 = system_clock::now();
    microseconds time = duration_cast<microseconds>(tp1 - tp0);

    return make_tuple(sum, time);
}

tuple<float, microseconds> sorted_summation(vector<float> nums) {
    auto tp0 = system_clock::now();
    // sort in place
    sort(nums.begin(), nums.end());

    float sum = 0.0;
    for (int i = 0; i < nums.size(); i++) {
        sum += nums[i];
    }
    auto tp1 = system_clock::now();
    microseconds time = duration_cast<microseconds>(tp1 - tp0);

    return make_tuple(sum, time);
}

vector<float> gen_many_floats(
        unsigned n_elems, float range_min, float range_max, int seed) {

    default_random_engine rng(seed);
    uniform_real_distribution<float> ud {range_min, range_max};
    auto dice = bind ( ud, rng );

    vector<float> r(n_elems);
    generate(begin(r), end(r), dice);

    return r;
}

microseconds calc_time_mean(vector<microseconds> data) {
    microseconds sum = duration_cast<microseconds>(duration<double>(0.0));
    for (auto i = 0; i < data.size(); i++) {
        sum += data[i];
    }
    return sum / data.size();
}

microseconds calc_time_std(vector<microseconds> data, microseconds mean) {
    double sum_sq = 0.0;
    double avg = duration<double>(mean).count();
    double delta;
    for (auto i = 0; i < data.size(); i++) {
        delta = duration<double>(data[i]).count() - avg;
        sum_sq += delta * delta;
    }
    double std = sqrt(sum_sq/double(data.size() - 1));
    return duration_cast<microseconds>(duration<double>(std));
}

double calc_err_mean(vector<double> data) {
    double sum_err = 0.0;
    for (auto i = 0; i < data.size(); i++) {
        sum_err += data[i];
    }
    return sum_err / double(data.size());
}

double calc_err_std(vector<double> data) {
    double sum_err = 0.0;
    for (auto i = 0; i < data.size(); i++) {
        sum_err += data[i];
    }
    // to obtain unbiased estimate of variance, should divide by 1/(N - 1)
    return sum_err * sqrt(1.0/double(data.size() - 1));
}

void print_stats(string description, vector<double> errors,
                 vector<microseconds> times) {

    cout << description << " | ERR: ";
    if (errors.size() > 0) {
        double e_avg = calc_err_mean(errors);
        double e_std = calc_err_std(errors);
        cout << e_avg << " +/- " << e_std;
    } else {
        cout << "n/a";
    }

    if (times.size() > 0) {
        microseconds t_mean = calc_time_mean(times);
        microseconds t_std = calc_time_std(times, t_mean);
        cout << " | TIME: " << t_mean.count() << " +/- "
             << t_std.count() << " us\n";
    } else {
        cout << "(missing time data!)\n";
    }
}

struct ExperimentResults {
    vector<microseconds> ref_times;
    vector<microseconds> plain_times;
    vector<microseconds> compensated_times;
    vector<microseconds> sorted_times;
    vector<double> plain_errs;
    vector<double> compensated_errs;
    vector<double> sorted_errs;

    ExperimentResults() {};

    ExperimentResults(unsigned capacity) {
        ref_times.reserve(capacity);
        plain_times.reserve(capacity);
        compensated_times.reserve(capacity);
        sorted_times.reserve(capacity);
        plain_errs.reserve(capacity);
        compensated_errs.reserve(capacity);
        sorted_errs.reserve(capacity);
    }

    void insert(microseconds ref,
                tuple<double, microseconds> plain,
                tuple<double, microseconds> compensated,
                tuple<double, microseconds> sorted) {
        ref_times.push_back(ref);
        insert_plain(plain);
        insert_compensated(compensated);
        insert_sorted(sorted);
    }

    void insert_plain(tuple<double, microseconds> result) {
        auto [err, time] = result;
        plain_errs.push_back(err);
        plain_times.push_back(time);
    }

    void insert_compensated(tuple<double, microseconds> result) {
        auto [err, time] = result;
        compensated_errs.push_back(err);
        compensated_times.push_back(time);
    }

    void insert_sorted(tuple<double, microseconds> result) {
        auto [err, time] = result;
        sorted_errs.push_back(err);
        sorted_times.push_back(time);
    }

    void print_stats() {
        vector<double> ref_errs;
        stringstream description;
        description << "ref (N=" << ref_times.size() << ")";
        ::print_stats(description.str(), ref_errs, ref_times);

        description.str(string()); // clear the string stream
        description << "plain (N=" << plain_errs.size() << ")";
        ::print_stats(description.str(), plain_errs, plain_times);

        description.str(string()); // clear the string stream
        description << "compensated (N="
            << compensated_errs.size() << ")";
        ::print_stats(description.str(), compensated_errs, compensated_times);

        description.str(string()); // clear the string stream
        description << "sorted (N=" << sorted_errs.size() << ")";
        ::print_stats(description.str(), sorted_errs, sorted_times);
    }
};

tuple<double, microseconds> run_sub_exp(string description, tuple<float,
        microseconds> f (vector<float>), vector<float> nums, double sum_ref) {
    auto [sum, time] = f(nums);
    cout << description << " sum: " << sum;
    // expectation of sum is sum of expectations
    double err = abs((double(sum) - sum_ref));
    cout << ", err: " << err << "\n";
    return make_tuple(err, time);
}

void run_experiments(unsigned n_repeats, unsigned n_elems, float range_min,
                     float range_max) {
    cout << "running " << n_repeats << " experiments, with vectors of size "
            << n_elems << ", chosen from the interval [" << range_min << ", "
            << range_max << "]" << "\n";

    ExperimentResults results = ExperimentResults(n_repeats);
    for (auto i = 0; i < n_repeats; i++) {
        cout << "------------------------------\n";
        auto seed = rand();
        cout << "running experiment " << i << "(seed: " << seed << ")\n";
        vector<float> nums =
                gen_many_floats(n_elems, range_min, range_max, seed);

        auto [sum_ref, ref_time] = reference_summation(nums);
        cout << "ref sum: " << sum_ref << "\n";
        auto plain = run_sub_exp("plain", plain_summation, vector(nums),
                                 sum_ref);
        auto compensated = run_sub_exp("compensated",
                                       compensated_summation, vector(nums),
                                       sum_ref);
        auto sorted = run_sub_exp("sorted",
                                  sorted_summation, vector(nums), sum_ref);
        results.insert(ref_time, plain, compensated, sorted);
    }
    cout << "------------------------------\n";
    cout << "ran " << n_repeats << " experiments, with vectors of size "
         << n_elems << ", chosen from the interval [" << range_min << ", "
         << range_max << "]" << "\n";
    results.print_stats();
}

int main() {
    unsigned n_repeats = 100;
    unsigned n_elems = 1000000;

    run_experiments(n_repeats, n_elems, 0.0, 1.0);
}
