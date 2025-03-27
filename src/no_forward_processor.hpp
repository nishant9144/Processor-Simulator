#ifndef NO_FORWARDING_PROCESSOR_HPP
#define NO_FORWARDING_PROCESSOR_HPP

#include "processor.hpp"

class NoForwardingProcessor : public Processor {
private:
    // Pipeline diagram specific to no forwarding

    // Hazard detection
    HazardDetectionUnit hazard_unit;
    
public:
    NoForwardingProcessor() = default;
    
    // Override specific pipeline stages if needed
    void fetch() override;
    void decode() override;
    void execute() override;
};

#endif // NO_FORWARDING_PROCESSOR_HPP