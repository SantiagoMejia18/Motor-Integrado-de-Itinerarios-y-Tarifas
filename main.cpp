/* =========================================================================
   Write here the full name of the members of the team formed for the
   development of the practice
   =========================================================================
   
   PRACTICE - STARTING CODE (DO NOT CHANGE THE GIVEN SECTIONS)
   Integrated Itinerary and Fare Engine - Medellin Metro System
   Programming Languages and Paradigms (ST0244) - Universidad EAFIT

   Build: g++ -std=c++17 -Wall -Wextra -o practice main.cpp
   Target environment: OnlineGDB, language selector set to C++17

   WHAT YOU RECEIVE
     - the currency formatter, the Leg and ChargedLeg records and the whole
       main() test harness, so that the expected output is reproducible
     - the declarations you must complete, each marked with TODO

   WHAT YOU MUST WRITE
     - PART 1: the TransportMode hierarchy
     - PART 2: the Passenger hierarchy
     - PART 3: chargeItinerary and the Network class
     - PART 4: the four report functions, with STL algorithms only

   The file must compile with ZERO errors and ZERO warnings under
   g++ -std=c++17 -Wall -Wextra. A warning is a defect, not a detail.
   ========================================================================= */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <map>
#include <functional>

/* -------------------------------------------------------------------------
   Utility: Colombian peso formatting (3820 -> "$ 3.820")
   ------------------------------------------------------------------------- */
std::string formatCOP(long long value) {
    std::string digits = std::to_string(value);
    std::string out;
    int count = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        out.push_back(*it);
        if (++count % 3 == 0 && it + 1 != digits.rend()) {
            out.push_back('.');
        }
    }
    std::reverse(out.begin(), out.end());
    return "$ " + out;
}

/* -------------------------------------------------------------------------
   PART 1. Abstract base class of the transport-mode hierarchy
   ------------------------------------------------------------------------- */
class TransportMode {
public:
    TransportMode(std::string name, int stations)
        : name_(std::move(name)), stations_(stations) {}

    virtual ~TransportMode() = default;          // mandatory: deleted through base pointer

    const std::string& name() const { return name_; }
    int stations() const { return stations_; }

    // 1.1  Pure virtual: this class cannot know either value.
    virtual long long baseFare() const = 0;
    virtual std::string serviceType() const = 0;

    // 1.2  Policy hooks with a sensible default of true.
    //      Subclasses override them only where the behaviour really differs.
    virtual bool allowsStudentFlatFare() const { return true; }
    virtual bool allowsIntegration() const { return true; }
    virtual bool opensIntegrationWindow() const { return true; }

    // 1.3  NON-virtual template method: the algorithm is written once here
    //      and calls the pure virtual members through this object.
    std::string describe() const {
        return serviceType() + " " + name() + " (" + std::to_string(stations()) +
               " stations) - full fare " + formatCOP(baseFare());
    }

private:
    std::string name_;
    int stations_;
};

// 1.4  The five concrete modes.
class MetroLine : public TransportMode {
public:
    MetroLine(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Metro line"; }
};

class TramLine : public TransportMode {
public:
    TramLine(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Tram line"; }
};

class CableLine : public TransportMode {
public:
    CableLine(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Cable line"; }
};

class TouristCable : public CableLine {
public:
    TouristCable(std::string name, int stations, long long surcharge)
        : CableLine(std::move(name), stations), surcharge_(surcharge) {}
    // The parent value is reused: the full fare is never repeated here.
    long long baseFare() const override { return CableLine::baseFare() + surcharge_; }
    std::string serviceType() const override { return "Tourist cable"; }
    bool allowsStudentFlatFare() const override { return false; }
    bool allowsIntegration() const override { return false; }
    bool opensIntegrationWindow() const override { return false; }
private:
    long long surcharge_;
};

class FeederRoute : public TransportMode {
public:
    FeederRoute(std::string name, int stations)
        : TransportMode(std::move(name), stations) {}
    long long baseFare() const override { return 0; }
    std::string serviceType() const override { return "Feeder route"; }
    // Free because the trunk fare covers it: it must not open a window.
    bool opensIntegrationWindow() const override { return false; }
};

/* -------------------------------------------------------------------------
   PART 2. Passenger hierarchy, Non-Virtual Interface idiom
   fareOn() is NOT virtual: it is the template method that clamps the result.
   computeFare() is the customisation point each profile overrides.
   ------------------------------------------------------------------------- */
// 2.1  Passenger base class, Non-Virtual Interface idiom.
class Passenger {
public:
    explicit Passenger(std::string name) : name_(std::move(name)) {}
    virtual ~Passenger() = default;

    const std::string& name() const { return name_; }
    virtual std::string profile() const = 0;

    // NOT virtual: template method that clamps the result (never negative).
    long long fareOn(const TransportMode& mode) const {
        return std::max(0LL, computeFare(mode));
    }

protected:
    // Customisation point. Default behaviour: the full fare of the mode.
    virtual long long computeFare(const TransportMode& mode) const {
        return mode.baseFare();
    }

private:
    std::string name_;
};

// 2.2  The five profiles (Civica fares, 2026).
class FrequentPassenger : public Passenger {
public:
    explicit FrequentPassenger(std::string name) : Passenger(std::move(name)) {}
    std::string profile() const override { return "Civica frequent"; }
    // Does NOT override computeFare: it inherits the default behaviour.
};

class StudentPassenger : public Passenger {
public:
    explicit StudentPassenger(std::string name) : Passenger(std::move(name)) {}
    std::string profile() const override { return "Municipal student"; }
protected:
    long long computeFare(const TransportMode& mode) const override {
        // Asks the mode for its policy instead of asking what class it is.
        if (mode.allowsStudentFlatFare()) {
            return 1600;
        }
        return Passenger::computeFare(mode);
    }
};

class SeniorPassenger : public Passenger {
public:
    explicit SeniorPassenger(std::string name) : Passenger(std::move(name)) {}
    std::string profile() const override { return "Senior citizen"; }
protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 490;
    }
};

class DisabilityPassenger : public Passenger {
public:
    explicit DisabilityPassenger(std::string name) : Passenger(std::move(name)) {}
    std::string profile() const override { return "Person with disability"; }
protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 1100;
    }
};

class BearerPassenger : public Passenger {
public:
    explicit BearerPassenger(std::string name) : Passenger(std::move(name)) {}
    std::string profile() const override { return "Bearer (occasional)"; }
protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) + 580;
    }
};

/* -------------------------------------------------------------------------
   PART 3. The itinerary and the integration rule
   ------------------------------------------------------------------------- */
struct Leg {
    int minute;                   // minutes elapsed since the journey started
    const TransportMode* mode;    // non-owning observer pointer
};

struct ChargedLeg {
    int minute;
    std::string modeName;
    std::string serviceType;
    long long charge;
    bool integrated;              // true: free transfer inside the window
};

// 3.1  The Network OWNS its modes; main() only observes them.
class Network {
public:
    TransportMode& add(std::unique_ptr<TransportMode> mode) {
        modes_.push_back(std::move(mode));
        return *modes_.back();
    }

    const std::vector<std::unique_ptr<TransportMode>>& modes() const { return modes_; }

    void printInventory() const {
        std::for_each(modes_.begin(), modes_.end(),
            [](const std::unique_ptr<TransportMode>& mode) {
                std::cout << "  - " << mode->describe() << '\n';
            });
    }

private:
    std::vector<std::unique_ptr<TransportMode>> modes_;
};

// 3.2  Applies the integration rule.
std::vector<ChargedLeg> chargeItinerary(std::vector<Leg> legs,
                                        const Passenger& passenger) {
    constexpr int WINDOW_MINUTES = 90;
    constexpr int MAX_FREE_TRANSFERS = 2;

    // a) sort by minute with a lambda comparator
    std::sort(legs.begin(), legs.end(),
              [](const Leg& a, const Leg& b) { return a.minute < b.minute; });

    std::vector<ChargedLeg> result;
    result.reserve(legs.size());

    bool windowOpen = false;
    int windowStart = 0;
    int freeUsed = 0;

    // The only explicit loop allowed in the whole program.
    for (const Leg& leg : legs) {
        const TransportMode& mode = *leg.mode;
        long long charge = passenger.fareOn(mode);
        bool integrated = false;

        if (!mode.allowsIntegration()) {
            // d) always pays, never touches the window
        } else if (windowOpen &&
                   leg.minute - windowStart <= WINDOW_MINUTES &&
                   freeUsed < MAX_FREE_TRANSFERS) {
            // c) free transfer inside the window
            charge = 0;
            integrated = true;
            ++freeUsed;
        } else if (mode.opensIntegrationWindow()) {
            // b) / f) pays and opens a new window
            windowOpen = true;
            windowStart = leg.minute;
            freeUsed = 0;
        }
        // e) otherwise: pays its own fare and leaves the window untouched

        result.push_back({leg.minute, mode.name(), mode.serviceType(), charge, integrated});
    }
    return result;
}

/* -------------------------------------------------------------------------
   PART 4. Reports built exclusively with STL algorithms and lambdas
   ------------------------------------------------------------------------- */
// 4.1  Total paid.
long long totalPaid(const std::vector<ChargedLeg>& legs) {
    return std::accumulate(legs.begin(), legs.end(), 0LL,
        [](long long total, const ChargedLeg& leg) { return total + leg.charge; });
}

// 4.2  Number of free transfers.
long long freeTransferCount(const std::vector<ChargedLeg>& legs) {
    return static_cast<long long>(std::count_if(legs.begin(), legs.end(),
        [](const ChargedLeg& leg) { return leg.integrated; }));
}

// 4.3  Amount paid per service type.
std::map<std::string, long long> paidByServiceType(const std::vector<ChargedLeg>& legs) {
    std::map<std::string, long long> totals;
    std::for_each(legs.begin(), legs.end(),
        [&totals](const ChargedLeg& leg) { totals[leg.serviceType] += leg.charge; });
    return totals;
}

// 4.4  Names of the modes that satisfy a criterion injected by the CALLER.
std::vector<std::string> namesMatching(
        const Network& network,
        const std::function<bool(const TransportMode&)>& policy) {
    std::vector<std::string> names;
    const auto& modes = network.modes();
    std::for_each(modes.begin(), modes.end(),
        [&names, &policy](const std::unique_ptr<TransportMode>& mode) {
            if (policy(*mode)) {
                names.push_back(mode->name());
            }
        });
    return names;
}

// 4.5  Join with ", ".
std::string joined(const std::vector<std::string>& items) {
    return items.empty()
        ? std::string()
        : std::accumulate(std::next(items.begin()), items.end(), items.front(),
              [](std::string acc, const std::string& item) {
                  return std::move(acc) + ", " + item;
              });
}

/* -------------------------------------------------------------------------
   MAIN
   ------------------------------------------------------------------------- */
int main() {
    std::cout << "============================================================\n";
    std::cout << "   INTEGRATED ITINERARY AND FARE ENGINE - Valle de Aburra\n";
    std::cout << "============================================================\n\n";

    Network network;
    TransportMode& lineA  = network.add(std::make_unique<MetroLine>("A", 21));
    TransportMode& lineB  = network.add(std::make_unique<MetroLine>("B", 7));
    TransportMode& tramTA = network.add(std::make_unique<TramLine>("T-A Ayacucho", 9));
    TransportMode& cableK = network.add(std::make_unique<CableLine>("K Santo Domingo", 4));
    TransportMode& arvi   = network.add(std::make_unique<TouristCable>("L Arvi", 2, 9180));
    TransportMode& feeder = network.add(std::make_unique<FeederRoute>("130 Robledo", 18));

    std::cout << ">> 1. NETWORK INVENTORY (polymorphic describe)\n";
    network.printInventory();
    std::cout << '\n';

    /* Itinerary deliberately given out of chronological order, so that the
       sort step is observable. */
    const std::vector<Leg> itinerary = {
        {45,  &cableK},
        {0,   &feeder},
        {12,  &lineA},
        {150, &lineB},
        {70,  &tramTA},
        {200, &arvi}
    };

    const FrequentPassenger frequent("Diego");
    const StudentPassenger  student("Valentina");
    const SeniorPassenger   senior("Dona Amparo");

    std::cout << ">> 2. FULL FARE PER PROFILE ON METRO LINE A\n";
    std::cout << "  " << frequent.profile() << " pays " << formatCOP(frequent.fareOn(lineA)) << '\n';
    std::cout << "  " << student.profile()  << " pays " << formatCOP(student.fareOn(lineA))  << '\n';
    std::cout << "  " << senior.profile()   << " pays " << formatCOP(senior.fareOn(lineA))   << '\n';
    std::cout << "  " << student.profile()  << " on the tourist cable pays "
              << formatCOP(student.fareOn(arvi)) << " (no flat fare there)\n\n";

    const std::vector<ChargedLeg> charged = chargeItinerary(itinerary, frequent);

    std::cout << ">> 3. CHARGED ITINERARY (frequent profile)\n";
    std::for_each(charged.begin(), charged.end(), [](const ChargedLeg& c) {
        std::cout << "  min " << c.minute << " | " << c.serviceType << " " << c.modeName
                  << " | " << formatCOP(c.charge)
                  << (c.integrated ? "  [free transfer]" : "") << '\n';
    });
    std::cout << '\n';

    std::cout << ">> 4. REPORTS BUILT WITH STL ALGORITHMS\n";
    std::cout << "  accumulate: total paid -> " << formatCOP(totalPaid(charged)) << '\n';
    std::cout << "  count_if: free transfers -> " << freeTransferCount(charged) << '\n';

    const auto byType = paidByServiceType(charged);
    std::for_each(byType.begin(), byType.end(),
        [](const std::pair<const std::string, long long>& entry) {
            std::cout << "  map: paid on " << entry.first << " -> "
                      << formatCOP(entry.second) << '\n';
        });

    const auto firstPaid = std::find_if(charged.begin(), charged.end(),
        [](const ChargedLeg& c) { return c.charge > 0; });
    std::cout << "  find_if: first leg that actually paid -> "
              << (firstPaid == charged.end() ? std::string("none") : firstPaid->modeName) << '\n';

    const std::function<bool(const TransportMode&)> bigModes =
        [](const TransportMode& m) { return m.stations() > 8; };
    std::cout << "  injected policy: modes with more than 8 stations -> "
              << joined(namesMatching(network, bigModes)) << '\n';

    std::cout << "  total for the student on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, student))) << '\n';
    std::cout << "  total for the senior on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, senior))) << '\n';

    std::cout << "\n============================================================\n";
    std::cout << "  End of demonstration.\n";
    std::cout << "============================================================\n";
    return 0;
}
