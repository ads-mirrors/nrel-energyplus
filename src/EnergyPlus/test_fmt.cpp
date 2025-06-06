#include <EnergyPlus/IOFiles.hh>

#include <fmt/format.h>

int main()
{
    std::vector<double> numbers{8.413899999999999E-007, 8.4138E-002, 178.23273867, 1004.0, 3509.0384893743, 8.41561564E015};
    size_t N = numbers.size();
    for (size_t i = 0; i < N; ++i) {
        numbers.push_back(-numbers[i]);
    }
    std::vector<std::string> formats{// Fortran
                                     "Z",
                                     "N",
                                     "S",
                                     "R",
                                     "T",
                                     // built ins
                                     "E",
                                     "f",
                                     "G",
                                     ""};
    std::vector<int> precisions{0, 1, 3, 5, 7, 10, 15};

    // Header for CSV
    fmt::print("Number,Precision");
    for (auto &fmt_type : formats) {
        fmt::print(",{0}_spec,{0}_value", fmt_type.empty() ? "None (empty)" : fmt_type);
    }
    fmt::print("\n");

    for (double num : numbers) {
        for (int prec : precisions) {
            fmt::print("{},{}", num, prec);
            for (auto &fmt_type : formats) {
                std::string format_spec = "{}";
                if (!fmt_type.empty()) {
                    format_spec = fmt::format("{{:.{0}{1}}}", prec, fmt_type);
                }
                std::string result = "None";
                try {
                    result = EnergyPlus::format(format_spec, num);
                } catch (...) {
                }
                fmt::print(",{},{}", format_spec, result);
            }
            fmt::print("\n");
        }
    }

    return 0;
}
