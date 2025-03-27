#ifndef FORWARDING_PROCESSOR_HPP
#define FORWARDING_PROCESSOR_HPP

#include "processor.hpp"

class ForwardingProcessor : public Processor {
private:
    // Forwarding unit
    ForwardingUnit forwarding_unit;
    Forward_HazardDetectionUnit hazard_unit;
    MUX_ALU mux_alu;
    
public:
    ForwardingProcessor() = default;
    
    // Override specific pipeline stages
    void fetch() override;
    void decode() override;
    void execute() override;
};

#endif // FORWARDING_PROCESSOR_HPP