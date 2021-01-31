#include <iostream>
#include <complex>
#include <sstream>

using namespace std;

auto ask_complex() {
    complex<double> c;
    string in;

    cout << "Prepare to enter a complex number." << "\n";
    cout << "Please use format: (real, imag)." << "\n";
    cout << "For example: 0 + 1i would be input as (0, 1)." << "\n";
    cout << "Input: ";
    // prefer `getline` over `cin`:
    // http://www.cplusplus.com/forum/articles/6046/
    getline(cin, in);
    istringstream(in) >> c;

    return c;
}

auto print_complex(complex<double> c) {
    cout << "You entered " << c.real() << " + " << c.imag() << "i" << endl;
}

int main() {
    print_complex(ask_complex());
}
