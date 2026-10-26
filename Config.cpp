#include "Config.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>

namespace {

const std::uint64_t MAX_VALUE = 4294967296ULL;  // 2^32, the upper limit for the numeric settings
const int MAX_CPUS = 128;

std::string toLower(std::string text) {
    for (std::size_t i = 0; i < text.size(); ++i) {
        text[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(text[i])));
    }
    return text;
}

std::string trim(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Removes one pair of surrounding quotes, so scheduler "rr" and scheduler rr both work.
std::string unquote(const std::string& text) {
    if (text.size() >= 2 && (text[0] == '"' || text[0] == '\'') && text[text.size() - 1] == text[0]) {
        return text.substr(1, text.size() - 2);
    }
    return text;
}

bool parseNumber(const std::string& text, std::uint64_t& value) {
    if (text.empty() || text.size() > 19) {
        return false;
    }
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }
    std::istringstream in(text);
    in >> value;
    return !in.fail();
}

}  // namespace

std::string schedulerName(SchedulerType type) {
    return type == SchedulerType::RR ? "rr" : "fcfs";
}

bool loadConfig(const std::string& path, Config& config, std::string& error) {
    std::ifstream in(path.c_str());
    if (!in) {
        error = "Could not open " + path + ". Put it in the same folder the program runs from.";
        return false;
    }

    // Each line is "<key> <value>". Blank lines and anything after '#' are ignored.
    std::map<std::string, std::string> values;
    std::string line;
    while (std::getline(in, line)) {
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos) {
            line.erase(comment);
        }
        std::istringstream words(line);
        std::string key;
        if (!(words >> key)) {
            continue;
        }
        std::string value;
        std::getline(words, value);
        values[toLower(key)] = unquote(trim(value));
    }

    const char* const required[] = {"num-cpu",  "scheduler", "quantum-cycles", "batch-process-freq",
                                    "min-ins",  "max-ins",   "delays-per-exec"};
    for (std::size_t i = 0; i < sizeof required / sizeof required[0]; ++i) {
        if (values.find(required[i]) == values.end()) {
            error = path + " is missing \"" + required[i] + "\".";
            return false;
        }
    }

    Config result;

    // Reads one numeric setting and checks it's within [minimum, maximum].
    struct NumberField {
        const char* key;
        std::uint64_t minimum;
        std::uint64_t maximum;
        std::uint64_t* target;
    };
    std::uint64_t numCpu = 0;
    const NumberField fields[] = {
        {"num-cpu", 1, MAX_CPUS, &numCpu},
        {"quantum-cycles", 1, MAX_VALUE, &result.quantumCycles},
        {"batch-process-freq", 1, MAX_VALUE, &result.batchProcessFreq},
        {"min-ins", 1, MAX_VALUE, &result.minIns},
        {"max-ins", 1, MAX_VALUE, &result.maxIns},
        {"delays-per-exec", 0, MAX_VALUE, &result.delaysPerExec},
    };
    for (std::size_t i = 0; i < sizeof fields / sizeof fields[0]; ++i) {
        const NumberField& field = fields[i];
        const std::string& text = values[field.key];
        std::uint64_t value = 0;
        if (!parseNumber(text, value) || value < field.minimum || value > field.maximum) {
            std::ostringstream message;
            message << "\"" << field.key << "\" must be a whole number from " << field.minimum << " to "
                    << field.maximum << " (got \"" << text << "\").";
            error = message.str();
            return false;
        }
        *field.target = value;
    }
    result.numCpu = static_cast<int>(numCpu);

    const std::string scheduler = toLower(values["scheduler"]);
    if (scheduler == "fcfs") {
        result.scheduler = SchedulerType::FCFS;
    } else if (scheduler == "rr") {
        result.scheduler = SchedulerType::RR;
    } else {
        error = "\"scheduler\" must be fcfs or rr (got \"" + values["scheduler"] + "\").";
        return false;
    }

    if (result.minIns > result.maxIns) {
        error = "\"min-ins\" can't be larger than \"max-ins\".";
        return false;
    }

    config = result;
    return true;
}
